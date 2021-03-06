#include "mMpcExLoadCalibrations.h"
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
#include "PdbMpcExHotDead.hh"
#include "PdbMpcExHighLow.hh"
#include "PdbMpcExSensorMIP.hh"
#include "PdbMpcExMinipadMIP.hh"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include <ctime>
#include <iostream>

mMpcExLoadCalibrations::mMpcExLoadCalibrations() : SubsysReco("MMPCEXLOADCALIBRATINS") {
}

mMpcExLoadCalibrations::~mMpcExLoadCalibrations(){
}

int mMpcExLoadCalibrations::InitRun(PHCompositeNode *topNode){

  //flag if == 0, pedestals (and hot/dead) only
  //all calibrations otherwise
  recoConsts *rc = recoConsts::instance();
  int calibMode = rc->get_IntFlag("MPCEXCALIBMODE",0x0); 

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
  // I am going to make one calibration for each minipad
  // the TMpcExCalib object should have "default" values
  // 
  for(unsigned short im=0; im<MpcExConstants::NMINIPADS; im++){
    TMpcExCalib *calib = new TMpcExCalib(im);
    calibs->addCalib(calib);
  }

  //
  // I also need the mapper later on
  //
  MpcExMapper *mapper = MpcExMapper::instance();

  //
  //first grab the runnumber so that we can get the correct data from the database
  //we will check the RecoConsts, then the RunHeader, the use the current date
  //

  int runNumber = rc->get_IntFlag("RUNNUMBER");
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
    PdbCalBank *pedbank = bankManager->fetchBank("PdbMpcExOnlinePedestalsBank",bankID,"mpcexonlinepedestals",*start_time);
    if(pedbank){
      pedbank->printHeader();

      PdbMpcExOnlinePedestals pedestals = (PdbMpcExOnlinePedestals&)(pedbank->getEntry(0));
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

	TMpcExCalib *calib = calibs->get(key);
	//this calib should definitely not be NULL... but its always good to check
	if(calib != NULL){
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
	  std::cout<<PHWHERE<<" A calibration does not exist already for (arm,packet,chipmap): ("<<arm<<","<<packet<<","<<chipmap<<")"<<std::endl;
	  std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	  delete start_time;
	  return ABORTRUN;
	}
      }//loop on key

      delete pedbank;
    } else {
      std::cout<<PHWHERE<<"I could not read PdbMpcExOnlinePedestalsBank from mpcexonlinepedestals -- bailing"<<std::endl;
      delete start_time;
      return ABORTEVENT;
    }

    //if we are only doing pedestal subtraction, we can stop...
    if(calibMode != 0) {

      //next fetch the hot/dead status from mpcexhotdeadtest
      PdbCalBank *hotbank = bankManager->fetchBank("PdbMpcExHotDeadBank",bankID,"mpcexhotdead",*start_time);
      if(hotbank){
	hotbank->printHeader();

	//only the keys marked hot and dead are kept in the table
	//therefore, the TMpcExCalib object should have zeros by default
	//and then the non-zero values indicating hot/dead etc. should be
	//set here
	int nentries = hotbank->getLength();
	std::cout<<nentries<<std::endl;
	for(int entry=0; entry<nentries; entry++){
	  PdbMpcExHotDead hotdead = (PdbMpcExHotDead&)(hotbank->getEntry(entry));
	  unsigned short key = hotdead.get_key();
	  TMpcExCalib *calib = calibs->get(key);
	  //this calib should definitely not be NULL... but its always good to check
	  if(calib != NULL){
	    calib->set_low_dead_hot_status(hotdead.get_low_gain_status());
	    calib->set_high_dead_hot_status(hotdead.get_high_gain_status());
	  } else {
	    std::cout<<PHWHERE<<" Something is seriously wrong."<<std::endl;
	    std::cout<<PHWHERE<<" A calibration does not exists already for key: "<<key<<std::endl;
	    std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	    delete start_time;
	    return ABORTRUN;
	  }
	}

	delete hotbank;
      } else {
	std::cout<<PHWHERE<<"I could not read PdbMpcExHotDeadBank from mpcexhotdead -- bailing"<<std::endl;
	delete start_time;
	return ABORTEVENT;
      }

      //next fetch the high/low ratios from mpcexhotdeadtest
      PdbCalBank *highlowbank = bankManager->fetchBank("PdbMpcExHighLowBank",bankID,"mpcexhighlow",*start_time);
      if(highlowbank){
	highlowbank->printHeader();

	int nentries = highlowbank->getLength();
	for(int entry=0; entry<nentries; entry++){
	  PdbMpcExHighLow highlow = (PdbMpcExHighLow&)(highlowbank->getEntry(entry));
	  unsigned short key = highlow.get_key();
	  TMpcExCalib *calib = calibs->get(key);
	  //this calib should definitely not be NULL... but its always good to check
	  if(calib != NULL){
	    calib->set_high_low_ratio(highlow.get_high_low());
	    calib->set_high_low_ratio_error(highlow.get_high_low_error());
	  } else {
	    std::cout<<PHWHERE<<" Something is seriously wrong."<<std::endl;
	    std::cout<<PHWHERE<<" A calibration does not exists already for key: "<<key<<std::endl;
	    std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	    delete start_time;
	    return ABORTRUN;
	  }
	}

	delete highlowbank;
      } else {
	std::cout<<PHWHERE<<"I could not read PdbMpcExHighLowBank from mpcexhighlow -- bailing"<<std::endl;
	delete start_time;
	return ABORTEVENT;
      }

      //next fetch the mip positions within a sensor from mpcexhotsensormip
      PdbCalBank *sensormipbank = bankManager->fetchBank("PdbMpcExSensorMIPBank",bankID,"mpcexsensormip",*start_time);
      if(sensormipbank){
	sensormipbank->printHeader();

	//all sensor values are kept - even if the sensor is not active
	int nentries = sensormipbank->getLength();
	for(int entry=0; entry<nentries; entry++){
	  PdbMpcExSensorMIP smip = (PdbMpcExSensorMIP&)(sensormipbank->getEntry(entry));
	  unsigned short arm = smip.get_arm();
	  unsigned short packet = smip.get_packet();
	  unsigned short sensor = smip.get_sensor();
	  float mip = smip.get_mip();
	  float emip = smip.get_mip_error();

	  //first minipad has this chipmap, then increment for the next 128 minipads within the module
	  //in this way all minipads in a given module will get the same mip_in_sensor value
	  unsigned short chipmap = MpcExConstants::NMINIPADS_PER_MODULE*sensor;
	  unsigned short maxchipmap = MpcExConstants::NMINIPADS_PER_MODULE*(sensor+1);
	  while(chipmap<maxchipmap){
	    unsigned short key = mapper->generate_key(arm,packet,chipmap);
	    TMpcExCalib *calib = calibs->get(key);
	  //this calib should definitely not be NULL... but its always good to check
	    if(calib != NULL){
	      calib->set_mip_in_sensor(mip);
	      calib->set_mip_in_sensor_error(emip);
	    } else {
	      std::cout<<PHWHERE<<" Something is seriously wrong."<<std::endl;
	      std::cout<<PHWHERE<<" A calibration does not exists already for key: "<<key<<std::endl;
	      std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	      delete start_time;
	      return ABORTRUN;
	    }
	    chipmap++;
	  }
	}
	
	delete sensormipbank;
      } else {
	std::cout<<PHWHERE<<"I could not read PdbMpcExHighLowBank from mpcexhighlow -- bailing"<<std::endl;
	delete start_time;
	return ABORTEVENT;
      }

      //next fetch the mip corrections for each minipad from mpcexhotminipadmip
      PdbCalBank *minipadmipbank = bankManager->fetchBank("PdbMpcExMinipadMIPBank",bankID,"mpcexminipadmip",*start_time);
      if(minipadmipbank){
	minipadmipbank->printHeader();

	//all minipad values ARE NOT KEPT
	int nentries = minipadmipbank->getLength();
	for(int entry=0; entry<nentries; entry++){
	  PdbMpcExMinipadMIP mmip = (PdbMpcExMinipadMIP&)(minipadmipbank->getEntry(entry));
	  unsigned int key = mmip.get_key();
	  float corr = mmip.get_mip_correction();
	  float ecorr = mmip.get_mip_correction_error();
	  TMpcExCalib *calib = calibs->get(key);
	  //this calib should definitely not be NULL... but its always good to check
	  if(calib != NULL){
	    calib->set_minipad_mip_correction(corr);
	    calib->set_minipad_mip_correction_error(ecorr);
	  } else {
	    std::cout<<PHWHERE<<" Something is seriously wrong."<<std::endl;
	    std::cout<<PHWHERE<<" A calibration does not exists already for key: "<<key<<std::endl;
	    std::cout<<PHWHERE<<" BAILING NOW"<<std::endl;
	    delete start_time;
	    return ABORTRUN;
	  }
	}

	delete minipadmipbank;
      } else {
	std::cout<<PHWHERE<<"I could not read PdbMpcExMinipadMIPBank from mpcexminipadmip -- bailing"<<std::endl;
	delete start_time;
	return ABORTEVENT;
      }
    }//if only pedestal subtraction
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

int mMpcExLoadCalibrations::EndRun(PHCompositeNode *topNode){

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

