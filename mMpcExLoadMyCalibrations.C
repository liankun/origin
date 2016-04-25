#include "mMpcExLoadMyCalibrations.h"
#include "PHCompositeNode.h"
#include "Fun4AllReturnCodes.h"
#include "getClass.h"
#include "TMpcExCalibContainer.h"
#include "recoConsts.h"
#include "PHTimeStamp.h"
#include "RunToTime.hh"
#include "RunHeader.h"
#include "PdbBankManager.hh"
#include "PdbApplication.hh"
#include "PdbBankID.hh"
#include "PdbCalBank.hh"
#include "PdbMpcExOnlinePedestals.hh"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include <ctime>
#include <iostream>

mMpcExLoadMyCalibrations::mMpcExLoadMyCalibrations() {
}

mMpcExLoadMyCalibrations::~mMpcExLoadMyCalibrations(){
}

int mMpcExLoadMyCalibrations::InitRun(PHCompositeNode *topNode){

  TMpcExCalibContainer *calibs = findNode::getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(calibs == NULL){
    std::cout<<PHWHERE<<" I could not find TMpcExCalibContainer"<<std::endl;
    std::cout<<PHWHERE<<" Make sure you load mMpcExCreateNodeTree first."<<std::endl;
    return ABORTRUN;
  }

  //
  //we will clear the calibrations between each run in case 
  //we ever run over multiple runs in the same session
  //
  calibs->clear();

  //
  // I also need the mapper later on
  //
  MpcExMapper *mapper = MpcExMapper::instance();

  //
  //first grab the runnumber so that we can get the correct data from the database
  //we will check the RecoConsts, then the RunHeader, the use the current date
  //

  recoConsts *rc = recoConsts::instance();
  int runNumber = rc->get_IntFlag("RUNNUMBER");
  if(runNumber > 445000) runNumber = 430911;
  PHTimeStamp *start_time;
  if(runNumber != 0){
    RunToTime *runtotime = RunToTime::instance();
    start_time = runtotime->getBeginTime(runNumber);
    std::cout<<PHWHERE<<" Reading runnumber "<<runNumber<<" from recoConsts"<<std::endl;
  } else {
    RunHeader *runHeader = findNode::getClass<RunHeader>(topNode,"RunHeader");
    if(runHeader){
      int runNumber = runHeader->get_RunNumber();
      RunToTime *runtotime = RunToTime::instance();
      start_time = runtotime->getBeginTime(runNumber);
      std::cout<<PHWHERE<<" Reading runnumber "<<runNumber<<" from RunHeadr"<<std::endl;
    } else {
      start_time = new PHTimeStamp();
      start_time->setTics(time(NULL));
      std::cout<<PHWHERE<<" Using today as the access time"<<std::endl;
    }
  }

  //
  //now it is time to grab the information from the database
  //

  //make the connection to the database
  PdbBankManager *bankManager = PdbBankManager::instance();
  if(bankManager == NULL){
    std::cout<<PHWHERE<<" I could not get a valid pointer to PdbBankManger"<<std::endl;
    delete start_time;
    return ABORTRUN;
  }

  //and grab an application
  PdbApplication *app = PdbApplication::instance();
  if(app == NULL){
    std::cout<<PHWHERE<<" I could not get a valid pointer to PdbApplication"<<std::endl;
    delete start_time;
    return ABORTRUN;
  }

  //start reading the data
  if(app->startRead()){
    PdbBankID bankID;
    bankID.setInternalValue(0);

    //first fetch the pedestals from mpcexonlinepedestals
    PdbCalBank *bank = bankManager->fetchBank("PdbMpcExOnlinePedestalsBank",bankID,"mpcexonlinepedestals",*start_time);
    if(bank){
      bank->printHeader();

      PdbMpcExOnlinePedestals pedestals = (PdbMpcExOnlinePedestals&)(bank->getEntry(0));
      for(unsigned int key=0; key<MpcExConstants::NMINIPADS; key++){

	unsigned short arm = mapper->get_arm(key);
	unsigned short packet = mapper->get_packet(key);
	unsigned short chipmap = mapper->get_chipmap(key);
	//the online pedestals are saved in terms of
	//ich where
	//ich = 128*12*chain + 128*chip + channel
	//instead of roc bond
	//but we can construct that given the key
	int chain = mapper->get_chain(key);
	int chip = mapper->get_chip(key);
	int rocbond = mapper->get_rocbond(key);

	//here is the construction for the channels that are saved in the database
	int hich = MpcExConstants::HIGHGAINMINIPAD_IN_CHIP[rocbond] + chip*MpcExConstants::NMINIPADS_PER_MODULE + chain*MpcExConstants::NMINIPADS_PER_MODULE*MpcExConstants::NCHIPS_PER_CHAIN;
	int loch = MpcExConstants::LOWGAINMINIPAD_IN_CHIP[rocbond] + chip*MpcExConstants::NMINIPADS_PER_MODULE + chain*MpcExConstants::NMINIPADS_PER_MODULE*MpcExConstants::NCHIPS_PER_CHAIN;
	//	    std::cout<<iarm<<" "<<ipacket<<" "<<ichannel<<" "<<chain<<" "<<chip<<" "<<rocbond<<" "<<hich<<" "<<loch<<std::endl;

	//grab it from the calibrations list
	//it should exist, but we should check
	TMpcExCalib *calib = calibs->get(key);
	if(calib == NULL){
	  calib = new TMpcExCalib(key);
	  calibs->addCalib(calib);
	  calib->set_low_pedestal(pedestals.get_pedestal(arm,packet,loch));
	  calib->set_low_pedestal_width(pedestals.get_pedestal_width(arm,packet,loch));
	  calib->set_low_pedestal_chi2(pedestals.get_pedestal_chi2(arm,packet,loch));
	  calib->set_low_dcm_threshold(pedestals.get_threshold(arm,packet,loch));
	  calib->set_high_pedestal(pedestals.get_pedestal(arm,packet,hich));
	  calib->set_high_pedestal_width(pedestals.get_pedestal_width(arm,packet,hich));
	  calib->set_high_pedestal_chi2(pedestals.get_pedestal_chi2(arm,packet,hich));
	  calib->set_high_dcm_threshold(pedestals.get_threshold(arm,packet,hich));
	} else {
	  std::cout<<PHWHERE<<" Something is seriously wrong."<<std::endl;
	  std::cout<<PHWHERE<<" A calibration exists already for (arm,packet,chipmap): ("<<arm<<","<<packet<<","<<chipmap<<")"<<std::endl;
	  std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	  delete start_time;
	  return ABORTRUN;
	}
      }//loop on key

      delete bank;
    } else {
      std::cout<<PHWHERE<<"I could not read PdbMpcExOnlinePedestalsBank from mpcexonlinepedestals -- bailing"<<std::endl;
      delete start_time;
      return ABORTEVENT;
    }

  } else {
    app->abort();
    std::cout<<PHWHERE<<" Problem reading from the database"<<std::endl;
    delete start_time;
    return ABORTRUN;
  }

  //clean up
  app->DisconnectDB();

  delete start_time;
  return EVENT_OK;
}

int mMpcExLoadMyCalibrations::End(PHCompositeNode *topNode){

  TMpcExCalibContainer *calibs = findNode::getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(calibs == NULL){
    std::cout<<PHWHERE<<" I could not find TMpcExCalibContainer"<<std::endl;
    std::cout<<PHWHERE<<" That is VERY WORRISOME given the message didn't exist when InitRun was called..."<<std::endl;
    return ABORTRUN;
  }

  //clean up memory...
  calibs->clear();

  return EVENT_OK;
}
