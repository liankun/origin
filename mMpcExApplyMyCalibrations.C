#include "mMpcExApplyMyCalibrations.h"
#include "PHIODataNode.h"
#include "PHNodeIterator.h"
#include "PHCompositeNode.h"
#include "Fun4AllReturnCodes.h"
#include "recoConsts.h"
#include "getClass.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExCalib.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "TMpcExHitSet.h"
#include "MpcExConstants.h"
#include "MpcExMapper.h"
#include "MpcExRawHit.h"
#include "MpcExEventHeader.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TH2D.h"
#include "TH1D.h"
#include <functional>
#include "MpcExEventQuality.h"

mMpcExApplyMyCalibrations::mMpcExApplyMyCalibrations(float threshold_sigma){
  _pedestal_threshold_sigma = threshold_sigma;

 //initialize the flag to default fail
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  _FailCellIDCheck[a][p][c][s] = 1; //true		 
	}
      }
    }
  }

  recoConsts *myrc = recoConsts::instance();
  fixedCalibs = myrc->get_IntFlag("MPCEXFIXEDCALIBS",0x0); 
  if(fixedCalibs)
    std::cout<<PHWHERE<<" Using FIXED calibrations!"<<std::endl;

  calibMode = (mMpcExApplyMyCalibrations::Mode) myrc->get_IntFlag("MPCEXCALIBMODE",0x0); 
  if(calibMode)
    std::cout<<PHWHERE<<" Calibration Mode calibMode = "<< calibMode <<std::endl;

  makeHisto = myrc->get_IntFlag("MpcExApplyCalibHisto",0x0); 
  outputfile = NULL;
  _histo_low = NULL;
  _histo_high = NULL;
  _histo_combined = NULL;
  _histo_lowADC = NULL;
  _histo_highADC = NULL;
  _histo_numHits_S = NULL;
  _histo_numHits_N = NULL;

  eliminateBad = myrc->get_IntFlag("MPCEXKILLBADHITS",0x0); 

  applyStackCut = myrc->get_IntFlag("MPCEXCALIBAPPLYSTACK",0x0); 

  //set up the random number generator
  // 0 starting seed will generate unique seed
  r3 = new TRandom3(0); 

}

mMpcExApplyMyCalibrations::~mMpcExApplyMyCalibrations(){
  if(makeHisto)
    delete outputfile;

  delete r3; 
}

int mMpcExApplyMyCalibrations::Init(PHCompositeNode *topNode)
{

  if(makeHisto){
    outputfile = new TFile("MpcExApplyCalibHisto.root","RECREATE");

    _histo_low = new TH2D("calib_elow_bykey","low gain energy deposit (keV) by key",49152,-0.5,49151.5,256,0.,10000.0); 
    _histo_high = new TH2D("calib_ehigh_bykey","high gain energy deposit (keV) by key",49152,-0.5,49151.5,256,0.,2000.0); 
    _histo_combined = new TH2D("calib_ecombined_bykey","combined energy deposit (keV) by key",49152,-0.5,49151.5,256,0.,10000.0); 

    _histo_lowADC = new TH2D("low_adc_bykey","Low gain pedestal subtracted ADC by key",49152,-0.5,49151.5,256,0.,256.0); 
    _histo_highADC = new TH2D("high_adc_bykey","High gain pedestal subtracted ADC by key",49152,-0.5,49151.5,256,0.,256.0); 
    _histo_numHits_S = new TH1D("num_hits_S","Number of calibrated minipad hits (south)",49152,-0.5,49152.5); 
    _histo_numHits_N = new TH1D("num_hits_N","Number of calibrated minipad hits (north)",49152,-0.5,49152.5); 

  }

  return 0; 

}

int mMpcExApplyMyCalibrations::process_event(PHCompositeNode *topNode){
//  std::cout <<PHWHERE <<"No MpcExEventQuality !!!"<<std::endl;
//code for event selection

  MpcExEventQuality* evt_quality = findNode::getClass<MpcExEventQuality>(topNode,"MpcExEventQuality");
  if(!evt_quality){
    std::cout << PHWHERE <<"No MpcExEventQuality !!!"<<std::endl;
    return ABORTEVENT;
  }
//  if(!evt_quality->IsEventWanted()) return EVENT_OK;

  MpcExRawHit *raw_hits = findNode::getClass<MpcExRawHit>(topNode,"MpcExRawHit");
  if(raw_hits == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find MpcExRawHit"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you are reading the MPC-EX DSTs."<<std::endl;
    return ABORTRUN;
  }

  MpcExEventHeader *evt_head = findNode::getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");
  if(evt_head == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find MpcExEventHeader"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you are reading the MPC-EX DSTs."<<std::endl; //is this right?
    return ABORTRUN;
  }

  TMpcExCalibContainer *calibs = findNode::getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(calibs == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find TMpcExCalibContainer"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you load mMpcExCreateNodeTree."<<std::endl;
    return ABORTRUN;
  }

  TMpcExHitContainer *hits = findNode::getClass<TMpcExHitContainer>(topNode,"TMpcExHitContainer");
  if(hits == NULL){
    std::cout<<PHWHERE<<" Something is terribly wrong -- I cannot find TMpcExHitContainer"<<std::endl;
    std::cout<<PHWHERE<<" Make sure that you load mMpcExCreateNodeTree."<<std::endl;
    return ABORTRUN;
  }

  // Make the stack cut for single event buffering (if requested)
  if( applyStackCut ){
    MpcExEventHeader *_feminfo = findNode::getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");
    if(_feminfo!=NULL){
      if(_feminfo->getStack() != 1){
	return ABORTEVENT;
      }
    }
  }

  //find which chips give inconsistent cellID's compared to rest in the chain
  CellIDCheck(evt_head);

  //construct the raw hit iterator
  TMpcExHitSet<> rawHits(raw_hits);

  TMpcExHitSet<>::const_iterator itr = rawHits.get_iterator();
  TMpcExHitSet<>::const_iterator last = rawHits.end();
  for(; itr!=last; ++itr){
    //we clone the hit from TMpcExHitSet
    //because *itr is the pointer owned by TMpcExHitSet
    //and this pointer will eventually be owned by 
    //the TMpcExHitContainer on the node tree
    TMpcExHit *hit = (*itr)->clone();

    //grab the calibration by the key
    unsigned int key = hit->key();

    //get calibration object
    TMpcExCalib *calib = calibs->get(key);
    if(calib == NULL){
      std::cout<<PHWHERE<<" Something is terribly wrong -- I could not get the calibrations for minipad with key: "<<hit->key()<<std::endl;
      std::cout<<PHWHERE<<" Doing nothing..."<<std::endl;
      delete hit; //avoid the memory leak...
      continue;
    }

    if(!fixedCalibs){

      //check if chip's cellID is okay
      unsigned short arm = hit->arm();
      unsigned short packet = hit->packet();
      unsigned short chain = hit->chain(); 
      unsigned short chip = hit->chip(); 

      hit->set_state_low(TMpcExHit::ADC);
      hit->set_state_high(TMpcExHit::ADC);

      if (_FailCellIDCheck[arm][packet][chain][chip])
	{
	  delete hit; //avoid the memory leak... yep delete them everywhere please
	  continue;
	}

      // Mark dead/hot channels first
      if(calib->low_dead_hot_status()>0){
	hit->set_status_low(TMpcExHit::DEAD_HOT);
      }

      if(calib->high_dead_hot_status()>0){
        hit->set_status_high(TMpcExHit::DEAD_HOT);
      }

      //subtract off the pedestal...
      //but only for non-masked channels
      bool bad_low_pedestal = calib->low_pedestal_chi2()<0;
      bool bad_high_pedestal = calib->high_pedestal_chi2()<0;

      if(!bad_low_pedestal){
	hit->set_low(hit->low()-calib->low_pedestal());
	hit->set_state_low(TMpcExHit::PEDESTAL_SUBTRACTED); 	     
      }
      else{
	hit->set_status_low(TMpcExHit::BAD_PEDESTAL_CALIBRATION); 	    
      }

      if(!bad_high_pedestal){
	hit->set_high(hit->high()-calib->high_pedestal());
	hit->set_state_high(TMpcExHit::PEDESTAL_SUBTRACTED); 	     
      }
      else{
	hit->set_status_high(TMpcExHit::BAD_PEDESTAL_CALIBRATION); 	    
      }

      // mark hits that are not significantly above pedestal
      // this marks "shadow hits" from random benefit due to the 
      // zero suppression

      if(hit->state_low()|TMpcExHit::PEDESTAL_SUBTRACTED){
	float low_pedestal_width = calib->low_pedestal_width();
	if( hit->low() <= (3.0*low_pedestal_width ) ){ //3-sigma
	    hit->set_status_low(TMpcExHit::LOW_ADC_RANDOM_BENEFIT); 	    
	}
      }

      if(hit->state_high()|TMpcExHit::PEDESTAL_SUBTRACTED){
	float high_pedestal_width = calib->high_pedestal_width();
	if( hit->high() <= (3.0*high_pedestal_width ) ){ //3-sigma
	    hit->set_status_high(TMpcExHit::LOW_ADC_RANDOM_BENEFIT); 	    
	}
      }

      // Pedestal subtracted ADC histograms
      if(makeHisto){
	if(hit->isGoodLowHit()&&(hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED))_histo_lowADC->Fill(key,hit->low()); 
	if(hit->isGoodHighHit()&&(hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED))_histo_highADC->Fill(key,hit->high()); 
      }

      // Stop here if only pedestal subtraction was requested 
      if(calibMode==mMpcExApplyMyCalibrations::PEDESTAL_SUBTRACTED_ONLY){
	hits->addHit(hit);
	continue; 
      }

      const float MIP_IN_keV = 147.6; 
      double EMAX_HIGH = ((250.0-20.0)/19.0)*MIP_IN_keV; // starting value, to be updated

      // gain calibration
      if(hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED){

	float MIP_sensor = calib->get_mip_in_sensor(); 
	float HL_ratio = calib->get_high_low_ratio(); 
        
	std::cout <<"MIP_sensor "<<MIP_sensor<<std::endl;
	std::cout <<"HL_ratio "<<HL_ratio<<std::endl;
        hit->set_low((hit->low()+r3->Rndm())*(MIP_IN_keV/MIP_sensor)*HL_ratio);
        hit->set_state_low(TMpcExHit::GAIN_CALIBRATED);

      }

      if(hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED){

	float MIP_sensor = calib->get_mip_in_sensor(); 

        hit->set_high((hit->high()+r3->Rndm())*(MIP_IN_keV/MIP_sensor));
        hit->set_state_high(TMpcExHit::GAIN_CALIBRATED);

	EMAX_HIGH = ((250.0-calib->high_pedestal())/MIP_sensor)*MIP_IN_keV;

      }

      // low and high gain energy histograms
      if(makeHisto){
	if(hit->isGoodGCLowHit())_histo_low->Fill(key,hit->low()); 
	if(hit->isGoodGCHighHit())_histo_high->Fill(key,hit->high()); 
      }

      // Generate the combined energy value
      // NOTES: 
      // (1) The ADC combining is done simply by choosing the 
      //     low or high value above a cutoff. This cutoff is 
      //     arbitrarily chosen at 250 in the un-subtracted 
      //     high-gain ADC. No weighting is chosen in combining the 
      //     ADC's across the boundary region. 
      // (2) There is no slope correction yet applied to the 
      //     combined value. This remains to be done. 

      if( hit->isGoodGCLowHit() && 
	  hit->isGoodGCHighHit() ){
	  
	if(hit->high() < EMAX_HIGH)
	  hit->set_combined(hit->high());
	else
	  hit->set_combined(hit->low());

	hit->set_state_combined(TMpcExHit::VALID); 
	    
      }
      else if( hit->isGoodGCLowHit() )
      {
	hit->set_combined(hit->low());	    
	hit->set_state_combined(TMpcExHit::VALID_LOW_ONLY); 
      }
      else if( hit->isGoodGCHighHit() )
      {
        hit->set_combined(hit->high());	    
	hit->set_state_combined(TMpcExHit::VALID_HIGH_ONLY); 
      }
      else{
        hit->set_combined(0.0);	    
	hit->set_state_combined(TMpcExHit::INVALID); 
      }

      // combined hit histogram 
      if(makeHisto){
	if(hit->isGoodCombinedHit())_histo_combined->Fill(key,hit->combined()); 
      }

    }
    else{

      // fixed calibration for simulations - pedestal at 20 counts,
      // MIP @ 19. H/L ratio = 4.5
      // Must match mMpcExDigitizeHits!
      float low_e = hit->low() - 20.0;
      float high_e = hit->high() - 20.0;
      
      // 3-sigma cut on pedestal width (0.1)
      if(low_e<=0.3) hit->set_status_low(TMpcExHit::LOW_ADC_RANDOM_BENEFIT);
      if(high_e<=0.8) hit->set_status_high(TMpcExHit::LOW_ADC_RANDOM_BENEFIT);

      hit->set_low(low_e);
      hit->set_high(high_e);
      hit->set_state_low(TMpcExHit::PEDESTAL_SUBTRACTED);
      hit->set_state_high(TMpcExHit::PEDESTAL_SUBTRACTED);

      // Mark dead/hot channels
       if(calib->low_dead_hot_status()>0){
       	 hit->set_status_low(TMpcExHit::DEAD_HOT);
       }

       if(calib->high_dead_hot_status()>0){
       	 hit->set_status_high(TMpcExHit::DEAD_HOT);
       }

      // Stop here if only pedestal subtraction was requested 
      if(calibMode==mMpcExApplyMyCalibrations::PEDESTAL_SUBTRACTED_ONLY){
	hits->addHit(hit);
	continue; 
      }

      // Pedestal subtracted ADC histograms
      if(makeHisto){
	if(hit->isGoodLowHit())_histo_lowADC->Fill(key,hit->low()); 
	if(hit->isGoodHighHit())_histo_highADC->Fill(key,hit->high()); 
      }
      
      // energy calibration
      hit->set_low( (hit->low()+r3->Rndm())*(147.6/19.0)*4.5 );
      hit->set_high( (hit->high()+r3->Rndm())*(147.6/19.0) );
      hit->set_state_low(TMpcExHit::GAIN_CALIBRATED);
      hit->set_state_high(TMpcExHit::GAIN_CALIBRATED);

      // low and high gain energy histograms
      if(makeHisto){
	if(hit->isGoodGCLowHit())_histo_low->Fill(key,hit->low()); 
	if(hit->isGoodGCHighHit())_histo_high->Fill(key,hit->high()); 
      }

      // Generate the combined energy 

      static const double EMAX_HIGH = ((255.0-20.0)/19.0)*147.6; 

      if( hit->isGoodGCLowHit() && 
	  hit->isGoodGCHighHit() ){
	  
	if(hit->high() < EMAX_HIGH)
	  hit->set_combined(hit->high());
	else
	  hit->set_combined(hit->low());

	hit->set_state_combined(TMpcExHit::VALID); 
	    
      }
      else if( hit->isGoodGCLowHit() )
      {
	hit->set_combined(hit->low());	    
	hit->set_state_combined(TMpcExHit::VALID_LOW_ONLY); 
      }
      else if( hit->isGoodGCHighHit() )
      {
        hit->set_combined(hit->high());	    
	hit->set_state_combined(TMpcExHit::VALID_HIGH_ONLY); 
      }
      else{
        hit->set_combined(0.0);	    
	hit->set_state_combined(TMpcExHit::INVALID); 
      }

      // combined hit histogram 
      if(makeHisto){
	if(hit->isGoodCombinedHit())_histo_combined->Fill(key,hit->combined()); 
      }

    }

    // Add the hit to the list 
    // (if the eliminateBad flag is hit, eliminate bad combined hits.)
    // delete the hit otherwise

    if( !(eliminateBad && !hit->isGoodCombinedHit()) ) 
      hits->addHit(hit);
    else
      delete hit; 
  }

  if(makeHisto){
    _histo_numHits_S->Fill(hits->sHits()); 
    _histo_numHits_N->Fill(hits->nHits()); 
  }

  return EVENT_OK;
}

int mMpcExApplyMyCalibrations::CellIDCheck(MpcExEventHeader *evt_head){
  //this method stores a list of svx4s that fail the cellid check in this event

  //fill map and array of cellID values for each svx4
  unsigned int Svx4CellIDs[2][8][48] = {{{0}}};
  for (unsigned int i = 0; i < evt_head->getCellIDsSize(); i++){
    //read event header to get cell ID for each chip
    unsigned int a = evt_head->getCellIDsArm(i);
    unsigned int p = evt_head->getCellIDsPkt(i);
    unsigned int s = evt_head->getCellIDsSVXID(i); //this is the chip + 12*chain
    unsigned int cellIDvalue =  evt_head->getCellIDsValue(i);
    Svx4CellIDs[a][p][s] = cellIDvalue;
  }

  //loop over quadrants
  for (unsigned int a=0; a < MpcExConstants::NARMS; a++) { //arm loop
    for (unsigned int p=0; p < MpcExConstants::NPACKETS_PER_ARM; p++) { //packet
      for (unsigned int c=0; c < MpcExConstants::NCHAINS_PER_PACKET; c++) { //chain 

	//make and clear cellIDMap
	std::map<unsigned int, int> CellIDMap;
	CellIDMap.clear();

	//count frequency of each cellID value
	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  //initialize the flag to default fail
	  _FailCellIDCheck[a][p][c][s] = 1; //true
	  
	  //find cellID value for a given arm, packet, chain, chip
	  int svnum = c*12+s;
	  unsigned int cellID = Svx4CellIDs[a][p][svnum];
	  
	  //if it is new to the map, fill it in
	  if (CellIDMap.find(cellID)==CellIDMap.end()) 
	    { 
	      CellIDMap[cellID] = 1;   
	    }
	  else 	    //otherwise increment cellID count
	    CellIDMap[cellID]++;
	}
	
	//find most common non0 cellID
	int maxFreq = -1;
	unsigned int MostCommonCellID = 9999;
	std::map<unsigned int, int>::iterator it;
	for (it = CellIDMap.begin(); it != CellIDMap.end(); it++){
	  unsigned int cellID = it->first;
	  int Freq = it->second;
	  if (Freq > maxFreq && cellID!=0)
	    {
	      maxFreq = Freq;
	      MostCommonCellID = cellID;
	    }
	}	      
	//if all cellIDs=0, then MostCommonCellID=9999 and all in this quadrant fail the check

	for (unsigned int s=0; s < MpcExConstants::NCHIPS_PER_CHAIN; s++) { //chip
	  int svnum = c*12+s;
	  unsigned int cellID = Svx4CellIDs[a][p][svnum];
	  if (cellID == MostCommonCellID) //if most common it doesn't fail
	    {
	      _FailCellIDCheck[a][p][c][s] = 0; //false
	    }
	}
      }
    }
  }

  return 0;
}

int mMpcExApplyMyCalibrations::End(PHCompositeNode *topNode)
{

  if(makeHisto){
    outputfile->Write();
    outputfile->Close();
  }
  return 0;
}

