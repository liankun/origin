#include <mMpcExDataAna.h>
#include <MpcExConstants.h>
#include <TMpcExHitContainer.h>
#include <TMpcExHit.h>
#include <TMpcExHitSet.h>
#include <TMpcExShowerContainer.h>
#include <TMpcExShower.h>
#include <MpcExRawHit.h>
#include "MpcExEventHeader.h"
#include "MpcExSpin.h"
#include "Exogram.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExCalib.h"
#include "TMpcExLShower.h"
#include "TMpcExLShowerContainer.h"

#include "PHIODataNode.h"
#include "getClass.h"
#include "BbcOut.h"
#include "PHGlobal.h"
#include "Bbc.hh"
#include "TriggerHelper.h"

//MPC Containers
#include "MpcMap.h"
#include "MpcCalib.h"
#include <mpcSampleContainer.h>
#include <mpcSampleV1.h>
#include "mpcClusterContent.h"
#include "mpcClusterContainer.h"
#include "mpcClusterContentV1.h"
#include "mpcTowerContainer.h"
#include "mpcTowerContent.h"
#include "mpcTowerContentV1.h"
#include "mpcRawContainer.h"
#include "mpcRawContent.h"

#include "primary.h"
#include "primaryWrapper.h"
#include "fkinWrapper.h"

#include "Fun4AllReturnCodes.h"
#include "Fun4AllServer.h"
#include "Fun4AllHistoManager.h"
#include "recoConsts.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <TH1F.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH1.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TCanvas.h>
#include <TBox.h>
#include <primaryWrapper.h>
#include <TRandom3.h>

#include "MpcExQualityCut.h"
#include "MpcExEventQuality.h"
#include "MpcExHitMap.h"

#include "TrigLvl1.h"
#include "MpcExMapper.h"


using namespace std;
using namespace findNode;

//sort function

mMpcExDataAna::mMpcExDataAna(const char* name) :
  SubsysReco(name)
{
  _vertex = -9999.0;

  eventNum = 0;  
  for(int arm = 0;arm <2;arm++){
    for(int layer = 0;layer <8;layer++){
      hmpcex_nxy[arm][layer] = NULL;
    }
    for(int i = 0;i < 288;i++){
      _mpc_tower_ped[arm][i] = -1;
    }
  }

  for(unsigned int i = 0;i < 49152;i++){
    _dead_map[i][0] = 0;
    _dead_map[i][1] = 0;
  }

}


int mMpcExDataAna::End(PHCompositeNode* topNode){
  return EVENT_OK;
}

mMpcExDataAna::~mMpcExDataAna(){
}

int mMpcExDataAna::Init(PHCompositeNode* topNode){
//  mpcexhit_init();
//  mpcex_eventheader_init();
//  mpcex_shower_init();
//  mpcex_spin_init();
//  mpc_tower_init();
//  mpc_tower_shape_init();
//  mpcex_new_shower_module_init();
//  lshower_init();
//  lshower_mip_init();
//  mpcex_hit2_init();
//  lshower_track_init();
//  bbc_init();
//  lshower_mip_mpc_init();
//  pair_channel_init();
//  zero_surpress_init();
//  sim_mpc_init();
//  HW_init();
//  run16_mpcex_hit_init();
  //vertex distribution
  hvertex_z = new TH1F("hvertex_z","vertex distribtution along z",200,-100,100);
  hvertex_z->GetXaxis()->SetTitle("Vertex along Z/cm");
  
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  hm->registerHisto(hvertex_z);
  cout <<"mMpcExDataAna : Init "<<endl;

  ifstream dead_map_file(_dead_map_path);
  if(dead_map_file.is_open()){
    int runNumber;
    int key;
    int high_status;
    int low_status;
    while(dead_map_file>>runNumber>>key>>high_status>>low_status){
      cout <<runNumber<<" "
           <<key<<" "
	   <<high_status<<" "
	   <<low_status<<" "
	   <<endl;
      _dead_map[key][0] = high_status;
      _dead_map[key][1] = low_status;
    }
  }
  else{
    cout<<PHWHERE<<"can not open the dead map txt file !!!"<<endl;
  }
 
  ifstream tower_ped_file(_mpc_tower_ped_path);
  if(tower_ped_file.is_open()){
    int arm;
    int channel;
    double ped;
    while(tower_ped_file>>arm>>channel>>ped){
      cout <<arm<<" "
           <<channel<<" "
	   <<ped<<" "
	   <<endl;
      _mpc_tower_ped[arm][channel] = ped;
    }
  }
  else{
    cout<<PHWHERE<<"can not open the tower pedestal file !!!"<<endl;
  }
  return EVENT_OK;
}

int mMpcExDataAna::InitRun(PHCompositeNode* topNode){
  set_interface_ptrs(topNode);
  return EVENT_OK;
}

void mMpcExDataAna::set_interface_ptrs(PHCompositeNode* topNode){
  _mpcex_hit_container = getClass<TMpcExHitContainer>(topNode,"TMpcExHitContainer");
  if(!_mpcex_hit_container){
    cout << PHWHERE <<":: No TMpcExHitContainer!!!"<<endl;
    exit(1);
  }

  _mpcex_raw_hits = getClass<MpcExRawHit>(topNode,"MpcExRawHit");
  if(!_mpcex_raw_hits){
    cout << PHWHERE <<":: NO MpcExRawHit !!!"<<endl;
    exit(1);
  }
 
  _mpcex_calibs = getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(!_mpcex_calibs){
    cout << PHWHERE <<":: No TMpcExCalibContainer !!!"<<endl;
    exit(1);
  }


  _mpcex_shower_container = getClass<TMpcExShowerContainer>(topNode,"TMpcExShowerContainer");
  
  if(!_mpcex_shower_container){
    cout << PHWHERE <<":: No TMpcExShowerContainer !!!"<<endl;
//    exit(1);
  }

  _mpcex_lshower_container = getClass<TMpcExLShowerContainer>(topNode,"TMpcExLShowerContainer");
  if(!_mpcex_lshower_container){
    cout <<PHWHERE <<":: No TMpcExLShowerContainer !!!"<<endl;    
  }

  _evt_quality = getClass<MpcExEventQuality>(topNode,"MpcExEventQuality");
  if(!_evt_quality){
    cout << PHWHERE <<"No MpcExEventQuality !!!"<<endl;
    exit(1);
  }


  _mpcex_eventheader = getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");
  if(!_mpcex_eventheader){
    cout << PHWHERE <<":: No MpcExEventHeader !!!"<<endl;
    exit(1);
  }
  
  _mpc_cluster_container = getClass<mpcClusterContainer>(topNode,"mpcClusterContainer");
  if(!_mpc_cluster_container){
    cout <<PHWHERE <<":: No mpcClusterContainer!!!"<<endl;
//    exit(1);
  }
  _mpc_tower_container = getClass<mpcTowerContainer>(topNode,"mpcTowerContainer");
  if(!_mpc_tower_container){
    cout << PHWHERE <<":: No mpcTowerContainer!!!"<<endl;
//    exit(1);
  }
  _mpc_map = MpcMap::instance();
  if(!_mpc_map){
    cout <<"No MpcMap!!!"<<endl;
//    exit(1);
  }

//  _mpc_map->Print();
  _triglvl1 = getClass<TrigLvl1>(topNode,"TrigLvl1");
  if(!_triglvl1){
    cout << PHWHERE <<":: No TrigLvl1 !!!"<<endl;
//    exit(1);
  }
  _mpcex_spin = MpcExSpin::instance();
  if(!_mpcex_spin){
    cout <<"Get MpcExSpin Failled !!!"<<endl;
//    exit(1);
  }
//  cout <<"print the MpcExSpin: "<<endl;
//  _mpcex_spin->Print();
  _mpc_sample_container = getClass<mpcSampleContainer>(topNode,"mpcSampleContainer");
  if(!_mpc_sample_container){
    cout <<PHWHERE<<":: NO mpcSampleContainer !!!"<<endl;
  }
  _mpc_calib = getClass<MpcCalib>(topNode, "MpcCalib");
  if(!_mpc_calib){
    cout <<PHWHERE<<":: No MpcCalib !!!"<<endl;
  }
  _mpc_raw_container = getClass<mpcRawContainer>(topNode,"MpcRaw2");
  if(!_mpc_raw_container){
    cout <<PHWHERE<<":: No mpcRawContainer !!!"<<endl;
  }
}

int mMpcExDataAna::process_event(PHCompositeNode* topNode){
//  cout <<"process mMpcExDataAna "<<endl;
//  TriggerHelper* myTH = new TriggerHelper(topNode);
//  int fire_minbias = myTH->trigScaled("BBCLL1(>1 tubes)");
//  int fire_ultra = myTH->trigScaled("UltraPeriphMPC");
//  if(!fire_minbias) return EVENT_OK;
//  if(!fire_ultra) return EVENT_OK;

  run16_MpcExHit_calib();
//MpcExCutFunction
//  if(!_evt_quality->IsEventWanted()) return EVENT_OK;
 //get information of single photon  
  primaryWrapper* primary = getClass<primaryWrapper> (topNode, "primary");
   _px_prim = 0;
   _py_prim = 0;
   _pz_prim = 0;
   _p_prim = 0;
   _hsx_prim = 0;
   _hsy_prim = 0;
   _eta = 0;
  size_t nprim = 2;  
  if(primary!=NULL){
    for (size_t iprim=0; iprim<nprim; iprim++){
      int idparticle = primary->get_idpart(iprim);
      if(idparticle==1){  //photon
        _px_prim = primary->get_px_momentum(iprim);
	_py_prim = primary->get_py_momentum(iprim);
	_pz_prim = primary->get_pz_momentum(iprim);
      }
    }
    _p_prim = sqrt(_px_prim*_px_prim + _py_prim*_py_prim + _pz_prim*_pz_prim);
    _eta = 1/2.*log((_p_prim + _pz_prim)/(_p_prim - _pz_prim));
    _hsx_prim = _px_prim/_pz_prim*(-1);
    _hsy_prim = _py_prim/_pz_prim;
  }

//vertex
  PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout && !phglobal){
    cout <<"No BbcOut or PHGlobal !!!"<<endl;
    exit(1);
  }
  _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();

  _bbc_charge[1] = bbcout->get_ChargeSum(Bbc::North);
  _bbc_charge[0] = bbcout->get_ChargeSum(Bbc::South);

//  mpcex_eventheader_study();   
//  mpcexhit_study();
  hvertex_z->Fill(_vertex);

  eventNum++;
  if(eventNum%2000 == 0) cout <<eventNum<<" events processed"<<endl;
//  mpcex_spin_study();
//  mpcex_shower_study();
//  event_display();
//  mpcex_hitmap_test();
//  mpc_tower_study();
//  mpc_tower_shape_study();
//  mpcex_new_shower_module_study();
//  new_shower_module_display();
//  lshower_study();
//  lshower_mip_study();
//  mpcex_hit2_study();
//  lshower_track_study();
//  bbc_study(topNode);
//  lshower_mip_mpc_study();
//  pair_channel_study();
//  zero_surpress_study();
//  sim_mpc_study(topNode);
//  lshower_evt_display();
//  primary_particle(topNode);
//  mpc_cluster_study();
//  HW_study();
//  run16_mpcex_hit_study();
  return EVENT_OK;
}

int mMpcExDataAna::mpcexhit_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hmpcex_high_hits[0] = new TH2F("hmpcex_high_hits_arm0","MpcEx Hits in South high q",50000,-0.5,49999.5,255*3,-0.5,254.5);
  hmpcex_high_hits[0]->GetXaxis()->SetTitle("Key");
  hmpcex_high_hits[0]->GetYaxis()->SetTitle("high q");
  hm->registerHisto(hmpcex_high_hits[0]);

  hmpcex_high_hits[1] = new TH2F("hmpcex_high_hits_arm1","MpcEx Hits in North high q",50000,-0.5,49999.5,255*3,-0.5,254.5);
  hmpcex_high_hits[1]->GetXaxis()->SetTitle("Key");
  hmpcex_high_hits[1]->GetYaxis()->SetTitle("high q");
  hm->registerHisto(hmpcex_high_hits[1]);


  hmpcex_low_hits[0] = new TH2F("hmpcex_low_hits_arm0","MpcEx Hits in South low q",50000,-0.5,49999.5,255*3,-0.5,254.5);
  hmpcex_low_hits[0]->GetXaxis()->SetTitle("Key");
  hmpcex_low_hits[0]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hmpcex_low_hits[0]);

  hmpcex_low_hits[1] = new TH2F("hmpcex_low_hits_arm1","MpcEx Hits in North low q",50000,-0.5,49999.5,255*3,-0.5,254.5);
  hmpcex_low_hits[1]->GetXaxis()->SetTitle("Key");
  hmpcex_low_hits[1]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hmpcex_low_hits[1]);

  hmpcex_good_q[0] = new TH2D("hmpcex_good_q_arm0","MpcEx Hits in South good q",50000,-0.5,49999.5,1000,0,0.1);
  hmpcex_good_q[0]->GetXaxis()->SetTitle("Key");
  hmpcex_good_q[0]->GetYaxis()->SetTitle("good q");
  hm->registerHisto(hmpcex_good_q[0]);

  hmpcex_good_q[1] = new TH2D("hmpcex_good_q_arm1","MpcEx Hits in North good q",50000,-0.5,49999.5,1000,0,0.1);
  hmpcex_good_q[1]->GetXaxis()->SetTitle("Key");
  hmpcex_good_q[1]->GetYaxis()->SetTitle("good q");
  hm->registerHisto(hmpcex_good_q[1]);


  hmpcex_high_vs_low[0] = new TH2F("hmpcex_high_vs_low_arm0","MpcEx high vs low in South arm",900,-110.5,789.5,900,-110.5,789.5);
  hmpcex_high_vs_low[0]->GetXaxis()->SetTitle("high q");
  hmpcex_high_vs_low[0]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hmpcex_high_vs_low[0]);

  hmpcex_high_vs_low[1] = new TH2F("hmpcex_high_vs_low_arm1","MpcEx high vs low in North arm",900,-110.5,789.5,900,-110.5,789.5);
  hmpcex_high_vs_low[1]->GetXaxis()->SetTitle("high q");
  hmpcex_high_vs_low[1]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hmpcex_high_vs_low[1]);

  hraw_cal = new TH2F("hraw_cal","Number of raw vs cal in MpcEx",100,0,5000,100,0,5000);
  hraw_cal->GetXaxis()->SetTitle("Number of Raw Hits");
  hraw_cal->GetYaxis()->SetTitle("Number of calibration Hits");
  hm->registerHisto(hraw_cal);

  hoddq = new TH2F("hoddq","Odd ADC distribution",256,-0.5,255.5,256,-0.5,255.5);
  hoddq->GetXaxis()->SetTitle("high ADC");
  hoddq->GetYaxis()->SetTitle("low ADC");
  hm->registerHisto(hoddq);

  hoddq_sub = new TH2F("hoddq_sub","Odd ADC pedestal Subtracted",256,-0.5,255.5,256,-0.5,255.5);
  hoddq->GetXaxis()->SetTitle("high ADC");
  hoddq->GetYaxis()->SetTitle("low ADC");
  hm->registerHisto(hoddq_sub);

  hmpcex_hitfreq[0] = new TH1D("hmpcex_hitfreq_arm0","MpcEx hit frequency for arm 0",26000,-0.5,25999.5);
  hmpcex_hitfreq[0]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hmpcex_hitfreq[0]);

  hmpcex_hitfreq[1] = new TH1D("hmpcex_hitfreq_arm1","MpcEx hit frequency for arm 1",26000,24000-0.5,49999.5);
  hmpcex_hitfreq[1]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hmpcex_hitfreq[1]);

  hmpcex_layer_adc[0] = new TH3D("hmpcex_layer_e_arm0","MpcEx Layer e arm 0",400,-100-0.5,300-0.5,400,-100-0.5,300-0.5,8,-0.5,7.5);
  hmpcex_layer_adc[0]->GetXaxis()->SetTitle("high ADC");
  hmpcex_layer_adc[0]->GetYaxis()->SetTitle("low ADC");
  hmpcex_layer_adc[0]->GetZaxis()->SetTitle("Layer");
  hm->registerHisto(hmpcex_layer_adc[0]);

  hmpcex_layer_adc[1] = new TH3D("hmpcex_layer_e_arm1","MpcEx Layer e arm 0",400,-100-0.5,300-0.5,400,-100-0.5,300-0.5,8,-0.5,7.5);
  hmpcex_layer_adc[1]->GetXaxis()->SetTitle("high ADC");
  hmpcex_layer_adc[1]->GetYaxis()->SetTitle("low ADC");
  hmpcex_layer_adc[1]->GetZaxis()->SetTitle("Layer");
  hm->registerHisto(hmpcex_layer_adc[1]);

  return EVENT_OK;
}

int mMpcExDataAna::mpcexhit_study(){
  TMpcExHitSet<>cal_hits(_mpcex_hit_container);
  TMpcExHitSet<>raw_hits(_mpcex_raw_hits);
  TMpcExHitSet<>::const_iterator begin = cal_hits.get_iterator();
  TMpcExHitSet<>::const_iterator end = cal_hits.end();
  int Nrawhits = 0;
  int Ncalhits = 0;
  while(begin != end){
    TMpcExHit* mpcex_hit = *begin;
    unsigned int key = mpcex_hit->key();
    int arm = mpcex_hit->arm();
    float high_q = mpcex_hit->high();
    float low_q = mpcex_hit->low();
    int layer = mpcex_hit->layer();
    bool state_high = (mpcex_hit->state_high() == TMpcExHit::PEDESTAL_SUBTRACTED);
    bool state_low = (mpcex_hit->state_low() == TMpcExHit::PEDESTAL_SUBTRACTED);
    if(state_high)hmpcex_high_hits[arm]->Fill(key,high_q);
    if(state_low)hmpcex_low_hits[arm]->Fill(key,low_q);
    if(state_high && state_low)hmpcex_layer_adc[arm]->Fill(high_q,low_q,layer);
    
    double good_q = get_good_q(mpcex_hit);
    if(good_q > 0){
      hmpcex_good_q[arm]->Fill(key,good_q);
    }
    if(low_q < 0) hmpcex_high_vs_low[arm]->Fill(high_q,low_q); 
    else hmpcex_high_vs_low[arm]->Fill(high_q,low_q);
    if(!state_high || !state_low || (high_q < 0 && low_q < 0)) {
      begin++;
      continue;
    }
    begin++;
    Ncalhits++;
  }
  begin = raw_hits.get_iterator();
  end = raw_hits.end();
  while(begin != end){
    TMpcExHit* mpcex_raw_hit = *begin;
    float high_adc = mpcex_raw_hit->high();
    float low_adc = mpcex_raw_hit->low();
    Nrawhits++;
    unsigned key = mpcex_raw_hit->key();
    int arm = mpcex_raw_hit->arm();
    hmpcex_hitfreq[arm]->Fill(key);
    if(high_adc >254 && low_adc >254) {
      begin++;
      continue;
    } 
    if(high_adc > 254 || low_adc > 254){
      hoddq->Fill(high_adc,low_adc);
      TMpcExCalib *calib = _mpcex_calibs->get(key);
      high_adc = high_adc - calib->high_pedestal();
      low_adc = low_adc - calib->low_pedestal();
      hoddq_sub->Fill(high_adc,low_adc);
    }
    begin++;
  }
//  cout <<"N raw hits "<<Nrawhits<<"N calhits "<<Ncalhits<<endl;
  hraw_cal->Fill(Nrawhits,Ncalhits);
  return EVENT_OK;
}

int mMpcExDataAna::mpcex_eventheader_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  hStack = new TH1F("hStack","Stack (multievent buffer) distribution",8,-0.5,7.5);
  hStack->GetXaxis()->SetTitle("Multi-Event Buffer");
  hm->registerHisto(hStack);

  hStatePhase[0] = new TH2F("hStatePhase_arm0","StatePhase arm 0",8,-0.5,7.5,500,-0.5,499.5);
  hStatePhase[0]->GetXaxis()->SetTitle("Packet Number");
  hStatePhase[0]->GetYaxis()->SetTitle("State Phase");
  hm->registerHisto(hStatePhase[0]);

  hStatePhase[1] = new TH2F("hStatePhase_arm1","StatePhase arm 1",8,-0.5,7.5,500,-0.5,499.5);
  hStatePhase[1]->GetXaxis()->SetTitle("Packet Number");
  hStatePhase[1]->GetYaxis()->SetTitle("State Phase");
  hm->registerHisto(hStatePhase[1]);

  hCellID[0] = new TH2F("hCellID_arm0","Cell ID arm 0",24*2*8,-0.5,24*2*8-0.5,50,-0.5,49.5); 
  hCellID[0]->GetXaxis()->SetTitle("SVX4 Index");
  hCellID[0]->GetYaxis()->SetTitle("Cell ID");
  hm->registerHisto(hCellID[0]);
  
  hCellID[1] = new TH2F("hCellID_arm1","Cell ID arm 1",24*2*8,-0.5,24*2*8-0.5,50,-0.5,49.5); 
  hCellID[1]->GetXaxis()->SetTitle("SVX4 Index");
  hCellID[1]->GetYaxis()->SetTitle("Cell ID");
  hm->registerHisto(hCellID[1]);


  hCellIDdiff[0] = new TH2F("hCellIDdiff_arm0","Cell ID Difference arm 0",24*2*8,-0.5,24*2*8-0.5,100,-50.5,49.5);
  hCellIDdiff[0]->GetXaxis()->SetTitle("SVX4 Index");
  hCellIDdiff[0]->GetYaxis()->SetTitle("Cell ID difference");
  hm->registerHisto(hCellIDdiff[0]);

  hCellIDdiff[1] = new TH2F("hCellIDdiff_arm1","Cell ID Difference arm 1",24*2*8,-0.5,24*2*8-0.5,100,-50.5,49.5);
  hCellIDdiff[1]->GetXaxis()->SetTitle("SVX4 Index");
  hCellIDdiff[1]->GetYaxis()->SetTitle("Cell ID difference");
  hm->registerHisto(hCellIDdiff[1]);

  
  return EVENT_OK;
}

int mMpcExDataAna::mpcex_eventheader_study(){
  int multibuffer = _mpcex_eventheader->getStack();
//  cout <<mutibuffer<<endl;
  hStack->Fill(multibuffer);
  int state_phase_size = _mpcex_eventheader->getStatephaseSize();
  for(int i = 0;i < state_phase_size;i++){
    int arm = _mpcex_eventheader->getStatephaseArm(i);
//    cout <<"arm "<<arm<<endl;
    unsigned int packet = _mpcex_eventheader->getStatephasePkt(i);
//    cout <<"packet "<<packet<<endl;
    unsigned int value = _mpcex_eventheader->getStatephaseValue(i);
//    cout <<"state phase value "<<value<<endl;
//    int state_phase = _mpcex_eventheader->getStatephase(i);
//    cout <<"state phase "<<state_phase<<endl;
    hStatePhase[arm]->Fill(packet,value);
  }

  int cellID_size = _mpcex_eventheader->getCellIDsSize();
//  cout <<"cellID_size: "<<cellID_size<<endl;
  vector<int> main_cellid;
  for(int i = 0;i < cellID_size;i++){
    int arm = _mpcex_eventheader->getCellIDsArm(i);
//    cout <<"arm "<<arm<<endl;
    int packet = _mpcex_eventheader->getCellIDsPkt(i);
//    cout <<"packet "<<packet<<endl;
    int svx4_id = _mpcex_eventheader->getCellIDsSVXID(i);
//    cout <<"svx4_id "<<svx4_id <<endl;
//    unsigned int cell_id = _mpcex_eventheader->getCellIDs(i);
//    cout <<"Cell ID "<<cell_id<<endl;
    unsigned int cell_id_value = _mpcex_eventheader->getCellIDsValue(i);
//    cout <<"Cell ID value: "<<cell_id_value<<endl;
    int index = packet*48+svx4_id;
    if(_evt_quality->IsCellIDGood(arm,packet,svx4_id) && _evt_quality->IsCellIDDominated(arm,packet,svx4_id) && _evt_quality->IsCellIDUnLocked(arm,packet,svx4_id)) hCellID[arm]->Fill(index,cell_id_value);
    main_cellid.push_back(cell_id_value);
    if(main_cellid.size() == 12){
      int stat[12] = {0};
      for(unsigned int j = 0;j < main_cellid.size();j++){
        int value = main_cellid[j];
	for(unsigned int k = 0; k < main_cellid.size();k++){
          if(main_cellid[k] == value) stat[j]++;
	}
      }
      int max_index = 0;
      int max = 0;
      for(int n = 0;n < 12 ;n++){
        if(stat[n] > max){ 
	  max_index = n;
	  max = stat[n];
	} 
      }
      int max_value = main_cellid[max_index];
      for(int ii = 0;ii < 12 ;ii++){
        if(main_cellid[ii] > 0)hCellIDdiff[arm]->Fill(index - 11 + ii,main_cellid[ii]-max_value);
      }
      main_cellid.clear();
    }
  }
  return EVENT_OK;
}

int mMpcExDataAna::mpcex_spin_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  han = new TH1F("han","A_N distribution",200,-0.6,0.6);
  han->GetXaxis()->SetTitle("Asymmetry");
  hm->registerHisto(han);

  hphi[0] = new TH1F("hphi_arm0","Phi distribution of showers arm 0",15,-3.14,3.14);
  hphi[0]->GetXaxis()->SetTitle("Phi");
  hm->registerHisto(hphi[0]);

  hphi[1] = new TH1F("hphi_arm1","Phi distribution of showers arm 1",15,-3.14,3.14);
  hphi[1]->GetXaxis()->SetTitle("Phi");
  hm->registerHisto(hphi[1]);

  return EVENT_OK;
}


int mMpcExDataAna::mpcex_spin_study(){
  //blue beam goes from south to north
  //yellow beam goes from north to south
  
  float bpol = _mpcex_spin->GetPolBlue(_triglvl1_clock_cross);
  float ypol = _mpcex_spin->GetPolYellow(_triglvl1_clock_cross);
  int bpat = _mpcex_spin->GetSpinPatternBlue(_triglvl1_clock_cross);
  int ypat = _mpcex_spin->GetSpinPatternYellow(_triglvl1_clock_cross);
  int pattern = _mpcex_spin->GetSpinPattern(_triglvl1_clock_cross);
  int spin_up_left = 0;
  int spin_up_right = 0;
  int spin_down_left = 0;
  int spin_down_right = 0;

/*
  cout <<"----------spin information---------"<<endl;
  cout <<"Event Count: "<<eventNum<<endl;
  cout <<"TrigLvl1 Clock Cross: "<<_triglvl1_clock_cross<<endl;
  cout <<"Blue  : pol "<<bpol<<" pattern "<<bpat<<endl;
  cout <<"Yellow: pol "<<ypol<<" pattern "<<ypat<<endl;
  cout <<"Pattern: "<<pattern<<endl;
*/

  unsigned int Nshowers = _mpcex_shower_container->size();
  for(unsigned int i = 0;i < Nshowers;i++){
    TMpcExShower* shower = _mpcex_shower_container->getShower(i);
    int arm = shower->get_arm();
    double total_e = shower->get_roughTotE();
    if(total_e > 0.5){
      if(arm == 0){
        float hx = shower->get_hsx();
	float hy = shower->get_hsy();
	float phi = atan2(hy,hx); 
        if(ypat == 1){
	  phi = phi - 3.14/2.;
	  if(phi < -3.14) phi = 2*3.14+phi;
	  hphi[0]->Fill(phi,ypol);
	}
	if(ypat == -1){
          phi = phi + 3.14/2.;
	  if(phi > 3.14) phi = phi -2*3.14;
	  hphi[0]->Fill(phi,ypol);
	}
      }
      if(arm == 1){
        float hx = shower->get_hsx();
	float hy = shower->get_hsy();
	float phi = atan2(hy,hx); 
        if(bpat == 1){
	  phi = phi - 3.14/2.;
	  if(phi < -3.14) phi = 2*3.14+phi;
	  hphi[1]->Fill(phi,bpol);
	}
	if(bpat == -1){
          phi = phi + 3.14/2.;
	  if(phi > 3.14) phi = phi -2*3.14;
	  hphi[1]->Fill(phi,bpol);
	}
      }
      //AN part
      if(pattern == 1 || pattern == 2){
	if(pattern == 1){
          if(arm == 0){
            float hx = shower->get_hsx();
	    if(hx < 0) spin_up_left++;
	    else spin_up_right++;
	  }
	  else {
            float hx = shower->get_hsx();
	    if(hx < 0) spin_down_left++;
	    else spin_down_right++;
	  }
	}
	else{
          if(arm == 0){
            float hx = shower->get_hsx();
	    if(hx < 0) spin_down_left++;
	    else spin_down_right++;
	  }
	  else {
            float hx = shower->get_hsx();
	    if(hx < 0) spin_up_left++;
	    else spin_up_right++;
	  }
	}
      }
    }
  }
  double asy0 = sqrt(spin_up_left*spin_down_right) - sqrt(spin_down_left*spin_up_right);
  double asy1 = sqrt(spin_up_left*spin_down_right) + sqrt(spin_down_left*spin_up_right);
  double asy = asy0/asy1/ypol;
//  cout <<"asy : "<<asy<<endl;
  han->Fill(asy);
  return EVENT_OK;
}

bool mMpcExDataAna::isSingleBuffered(){
  int multibuffer = _mpcex_eventheader->getStack();
  if(multibuffer == 1) return true;
  else return false; 
}

int mMpcExDataAna::mpcex_shower_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  hnshower[0] = new TH3F("hnshower_arm0","Number of Showers",50,0,100,50,0,100,50,0,100);
  hnshower[0]->GetXaxis()->SetTitle("Number of showers");
  hnshower[0]->GetYaxis()->SetTitle("Number of fired towers");
  hnshower[0]->GetZaxis()->SetTitle("Number of clusters in MPC");
  hm->registerHisto(hnshower[0]);

  hnshower[1] = new TH3F("hnshower_arm1","Number of Showers",50,0,100,50,0,100,50,0,100);
  hnshower[1]->GetXaxis()->SetTitle("Number of showers");
  hnshower[1]->GetYaxis()->SetTitle("Number of fired towers");
  hnshower[1]->GetZaxis()->SetTitle("Number of clusters in MPC");
  hm->registerHisto(hnshower[1]);

  hshower_e[0] = new TH3F("shower_e_arm0","Energy of shower arm 0",50,0,5,100,0,100,100,0,100);
  hshower_e[0]->GetXaxis()->SetTitle("MPCEX Energy/GeV");
  hshower_e[0]->GetYaxis()->SetTitle("MPC E3x3/GeV");
  hshower_e[0]->GetZaxis()->SetTitle("Shower Total E/GeV");
  hm->registerHisto(hshower_e[0]);

  hshower_e[1] = new TH3F("shower_e_arm1","Energy of shower arm 1",50,0,5,100,0,100,100,0,100);
  hshower_e[1]->GetXaxis()->SetTitle("MPCEX Energy/GeV");
  hshower_e[1]->GetYaxis()->SetTitle("MPC E3x3/GeV");
  hshower_e[1]->GetZaxis()->SetTitle("Shower Total E/GeV");
  hm->registerHisto(hshower_e[1]);



  return EVENT_OK;
}

int mMpcExDataAna::mpcex_shower_study(){
/*
 unsigned int Nshowers = _mpcex_shower_container->size();

//  cout <<"mpcex shower number: "<<Nshowers<<endl;
  int nshowers[2] = {0};
  for(unsigned int i = 0;i < Nshowers;i++){
    TMpcExShower* shower = _mpcex_shower_container->getShower(i);
    int arm = shower->get_arm();
//    double e_low = shower->get_e_low();
    double e_high = shower->get_e_high();
    double total_e = shower->get_roughTotE();
    double mpcE3x3 = shower->get_mpcE3x3();
//    double mpcE5x5 = shower->get_mpcE5x5();
    hshower_e[arm]->Fill(e_high,mpcE3x3,total_e);
    if(total_e > 0.5){
      nshowers[arm]++;
    }

    cout <<"shower "<<i<<" arm "<<arm
         <<" high e "<<e_high
	 <<" low e "<<e_low
	 <<" total e "<<total_e
	 <<" mpc E 3x3 "<<mpcE3x3
	 <<" mpc E 5x5 "<<mpcE5x5
	 <<endl;

  }

  int NMpcTowers = _mpc_tower_container->size();
  int ntowers[2] = {0};
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower > 0.1){
      ntowers[arm]++;
    }
  }

  int Nclus = _mpc_cluster_container->size();
  int nclusters[2] = {0};
  for(int iclus = 0;iclus < Nclus;iclus++){
    mpcClusterContent *clus = _mpc_cluster_container->getCluster(iclus);
    int arm = clus->arm();
    nclusters[arm]++;
  }
  
  for(int arm = 0;arm < 2;arm++){
    hnshower[arm]->Fill(nshowers[arm],ntowers[arm],nclusters[arm]);
  }
 */ 
  return EVENT_OK;
}

void mMpcExDataAna::event_display(){
//mpcex part
//initialize the grammy
  for(int i = 0;i < 2; i++){
    if(grammyl[i]) delete grammyl[i];
    if(grammyh[i]) delete grammyh[i];
    char name[50];
    sprintf(name,"Arm %d for low q",i);
    grammyl[i] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
    sprintf(name,"Arm %d for high q",i);
    grammyh[i] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
  }


  TMpcExHitSet<>cal_hits(_mpcex_hit_container);
  TMpcExHitSet<>::const_iterator begin = cal_hits.get_iterator();
  TMpcExHitSet<>::const_iterator end = cal_hits.end();
//  int hits_num = 0;
  for(;begin != end;begin++){
    TMpcExHit* mpcex_hit = *begin;
    int arm = mpcex_hit->arm();
    unsigned int key = mpcex_hit->key();
    if(mpcex_hit->state_combined()!=TMpcExHit::INVALID){ 
      double hit_q = mpcex_hit->combined();
      grammyh[arm]->FillEx(key,hit_q);
//      cout <<key<<" "<<hit_q<<endl;
//      hits_num++;
    } 
  }

//mpc part
  for(int i = 0;i < 2;i++){
    if(hmpc_gridxy[i]) delete hmpc_gridxy[i];
    char name[100];
    sprintf(name,"hmpc_gridxy_arm%d",i);
    hmpc_gridxy[i] =  new TH2F(name,name,600,-24,24,600,-24,24);
  }

  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0.) continue;
//    int gridx = _mpc_map->getGridX(tow_ch);
//    int gridy = _mpc_map->getGridY(tow_ch);
//    hmpc_gridxy[arm]->Fill(gridx,gridy,e_tower); 
    double x = _mpc_map->getX(tow_ch); 
    double y = _mpc_map->getY(tow_ch);
    int binx0 = hmpc_gridxy[arm]->GetXaxis()->FindBin(x-0.9);
    int binx1 = hmpc_gridxy[arm]->GetXaxis()->FindBin(x+0.9);
    int biny0 = hmpc_gridxy[arm]->GetYaxis()->FindBin(y-0.9);
    int biny1 = hmpc_gridxy[arm]->GetYaxis()->FindBin(y+0.9);
    for(int i = binx0;i <=binx1;i++){
      for(int j = biny0;j <=biny1;j++){
	hmpc_gridxy[arm]->SetBinContent(i,j,e_tower);
      }
    }
  }

//  if(hits_num == 0) return;
//display part
//draw all layers
  
  TCanvas* c_mpc0 = new TCanvas("c_mpc0","c_mpc0",1500,800);
  c_mpc0->Divide(2,1);
  c_mpc0->cd(1);
  hmpc_gridxy[0]->Draw("colz");
  c_mpc0->cd(2);
  grammyh[0]->Project3D("yx")->DrawCopy("colz");
//  grammyh[0]->DrawLayer(7,"colz");
  TCanvas* c_mpc1 = new TCanvas("c_mpc1","c_mpc1",1500,800);
  c_mpc1->Divide(2,1);
  c_mpc1->cd(1);
  hmpc_gridxy[1]->Draw("colz");
  c_mpc1->cd(2);
  grammyh[1]->Project3D("yx")->DrawCopy("colz");
//  grammyh[1]->DrawLayer(7,"colz");



/*  
  TCanvas* c_all_h = new TCanvas("c_all_h","all layers for high q",1500,800);
  c_all_h->Divide(2,1);
  c_all_h->cd(1);
  grammyh[0]->Project3D("yx")->DrawCopy("colz");
  c_all_h->cd(2);
  grammyh[1]->Project3D("yx")->DrawCopy("colz");
*/
//  TCanvas* c_all_l = new TCanvas("c_all_l","all layers for low q",1500,800);
//  c_all_l->Divide(2,1);
//  c_all_l->cd(1);
//  grammyl[0]->Project3D("yx")->DrawCopy("colz");
//  c_all_l->cd(2);
//  grammyl[1]->Project3D("yx")->DrawCopy("colz");



//mpcex
//  TCanvas* cl0 = new TCanvas("disgraml0","disgraml1",1500,800);
//  cl0->Divide(4,2);
//  TCanvas* cl1 = new TCanvas("disgraml1","disgraml1",1500,800);
//  cl1->Divide(4,2);

  TCanvas* ch0 = new TCanvas("disgramh0","disgramh0",1500,800);
  ch0->Divide(4,2);
  TCanvas* ch1 = new TCanvas("disgramh1","disgramh1",1500,800);
  ch1->Divide(4,2);



  for(int i = 0;i < 8;i++){
//    cl0->cd(i+1);
//    grammyl[0]->DrawLayer(i,"colz");
//    cl1->cd(i+1);
//    grammyl[1]->DrawLayer(i,"colz");
    ch0->cd(i+1);
    grammyh[0]->DrawLayer(i,"colz");
    ch1->cd(i+1);
    grammyh[1]->DrawLayer(i,"colz");
  }



  return;
}


int mMpcExDataAna::mpcex_hitmap_test(){
//check MpcExHitMap 

/*
  MpcExHitMap mpcex_hitmap; 
  for(int i = 0;i < 2; i++){
    if(grammyl[i]) delete grammyl[i];
    if(grammyh[i]) delete grammyh[i];
    char name[50];
    sprintf(name,"Arm %d for low q",i);
    grammyl[i] = new Exogram(name,name,1800,-24,24,1800,-24,24,8,-0.5,7.5);
    sprintf(name,"Arm %d for high q",i);
    grammyh[i] = new Exogram(name,name,1800,-24,24,1800,-24,24,8,-0.5,7.5);
  }
*/
  for(int arm = 0;arm < 2;arm++){
    for(int layer = 0;layer < 8;layer++){
      char name[100];
      sprintf(name,"hmpcex_nxy_layer%d_arm%d",layer,arm);
      if(hmpcex_nxy[arm][layer]) delete hmpcex_nxy[arm][layer];
      int Nx = 198;
      float ScaleX = 1;
      int Ny = 24;
      float ScaleY = 7.5;
      if(layer%2 == 1){
        Nx = 24;
	ScaleX = 7.5;
	Ny = 198;
	ScaleY = 1;
      }
      hmpcex_nxy[arm][layer] = new TH2F(name,name,Nx,-0.5*ScaleX,(Nx-0.5)*ScaleX,Ny,-0.5*ScaleY,(Ny-0.5)*ScaleY);
    }
  }

  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  for(int arm =0;arm<2;arm++){
    for(int layer=0;layer<8;layer++){
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      for(;iter!=mpcex_hitmap.get_layer_first(arm,layer+1);iter++){
	TMpcExHit* hit = (*iter).second;
        float high_q = hit->high();
        int nx = mpcex_hitmap.get_nx(hit);
        int ny = mpcex_hitmap.get_ny(hit);
        if(high_q > 2)hmpcex_nxy[arm][layer]->SetBinContent(nx+1,ny+1,high_q);
      }
    }
  }


/*
  for(int arm =0;arm<2;arm++){
    for(int layer=0;layer<8;layer++){
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      for(;iter!=mpcex_hitmap.get_layer_first(arm,layer+1);iter++){
	TMpcExHit* hit = (*iter).second;
        float high_q = hit->high();
        unsigned int key = hit->key();
        int nx = mpcex_hitmap.get_nx(hit);
        int ny = mpcex_hitmap.get_ny(hit);
        hmpcex_nxy[arm][layer]->SetBinContent(nx+1,ny+1,high_q);
	grammyh[arm]->FillEx(key,high_q);
	grammyl[arm]->FillEx(key,high_q);
      }
    }
  }
*/
/*
  MpcExHitMap::const_iterator iter = mpcex_hitmap.get_begin();
  for(;iter != mpcex_hitmap.get_end();iter++){
    TMpcExHit* hit = (*iter).second;
    float high_q = hit->high();
    unsigned int key = hit->key();
    int nx = mpcex_hitmap.get_nx(hit);
    int ny = mpcex_hitmap.get_ny(hit);
    int arm = hit->arm();
    int layer = hit->layer();
    hmpcex_nxy[arm][layer]->SetBinContent(nx+1,ny+1,high_q);
    grammyh[arm]->FillEx(key,high_q);
    grammyl[arm]->FillEx(key,high_q);
  }
*/
//draw graph

  TCanvas* c_nxy_0 = new TCanvas("c_nxy0","c_nxy0",1200,800);
  c_nxy_0->Divide(4,2);
  TCanvas* c_nxy_1 = new TCanvas("c_nxy1","c_nxy1",1200,800);
  c_nxy_1->Divide(4,2);
  for(int i = 0;i < 8;i++){
    c_nxy_0->cd(i+1);
    hmpcex_nxy[0][i]->Draw("colz");
    c_nxy_1->cd(i+1);
    hmpcex_nxy[1][i]->Draw("colz");
  }

/*
  TCanvas* cl0 = new TCanvas("disgraml0","disgraml1",1500,800);
  cl0->Divide(4,2);
  TCanvas* cl1 = new TCanvas("disgraml1","disgraml1",1500,800);
  cl1->Divide(4,2);
  TCanvas* ch0 = new TCanvas("disgramh0","disgramh0",1500,800);
  ch0->Divide(4,2);
  TCanvas* ch1 = new TCanvas("disgramh1","disgramh1",1500,800);
  ch1->Divide(4,2);

  for(int i = 0;i < 8;i++){
    cl0->cd(i+1);
    grammyl[0]->DrawLayer(i,"colz");
    cl1->cd(i+1);
    grammyl[1]->DrawLayer(i,"colz");
    ch0->cd(i+1);
    grammyh[0]->DrawLayer(i,"colz");
    ch1->cd(i+1);
    grammyh[1]->DrawLayer(i,"colz");
  }
*/
  return EVENT_OK;
}

int mMpcExDataAna::mpcex_myshower(){

/*
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  typedef vector<TMpcExHit*> myshower;
  vector<myshower> shower_list;
  for(unsigned int arm = 0;arm < 2;arm++){
    MpcExHitMap::iterator iter_begin = mpcex_hitmap.get_layer_first(arm,0);
    MpcExHitMap::iterator iter_end = mpcex_hitmap.get_layer_first(arm+1,0);
    for(;iter_begin != iter_end;iter_begin++){
      TMpcExHit* hit = iter_begin->second;
            
    }
  }
*/
  return EVENT_OK;
}

int mMpcExDataAna::mpc_tower_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  hmpc_fired_tower[0] =  new TH2D("hmpc_fired_tower_arm0","mpc tower arm 0",600,-24,24,600,-24,24);
  hmpc_fired_tower[0]->GetXaxis()->SetTitle("X/cm");
  hmpc_fired_tower[0]->GetYaxis()->SetTitle("Y/cm");
  hm->registerHisto(hmpc_fired_tower[0]);

  hmpc_fired_tower[1] =  new TH2D("hmpc_fired_tower_arm1","mpc tower arm 1",600,-24,24,600,-24,24);
  hmpc_fired_tower[1]->GetXaxis()->SetTitle("X/cm");
  hmpc_fired_tower[1]->GetYaxis()->SetTitle("Y/cm");
  hm->registerHisto(hmpc_fired_tower[1]);

  hmpc_fired_tower_e[0] =  new TH3D("hmpc_fired_tower_e_arm0","mpc tower arm 0",600,-24,24,600,-24,24,200,0,200);
  hmpc_fired_tower_e[0]->GetXaxis()->SetTitle("X/cm");
  hmpc_fired_tower_e[0]->GetYaxis()->SetTitle("Y/cm");
  hmpc_fired_tower_e[0]->GetZaxis()->SetTitle("tower e/GeV");
  hm->registerHisto(hmpc_fired_tower_e[0]);

  hmpc_fired_tower_e[1] =  new TH3D("hmpc_fired_tower_e_arm1","mpc tower arm 1",600,-24,24,600,-24,24,200,0,200);
  hmpc_fired_tower_e[1]->GetXaxis()->SetTitle("X/cm");
  hmpc_fired_tower_e[1]->GetYaxis()->SetTitle("Y/cm");
  hmpc_fired_tower_e[1]->GetZaxis()->SetTitle("tower e/GeV");
  hm->registerHisto(hmpc_fired_tower_e[1]);

  hmpc_peak_tower[0] = new TH3D("hmpc_peak_tower_arm0","peak tower arm 0",200,0,150,200,0,2,288,-0.5,287.5);
  hmpc_peak_tower[0]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_peak_tower[0]->GetYaxis()->SetTitle("energy ratio");
  hmpc_peak_tower[0]->GetZaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_peak_tower[0]);

  hmpc_peak_tower[1] = new TH3D("hmpc_peak_tower_arm1","peak tower arm 1",200,0,150,200,0,2,288,-0.5,287.5);
  hmpc_peak_tower[1]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_peak_tower[1]->GetYaxis()->SetTitle("energy ratio");
  hmpc_peak_tower[1]->GetZaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_peak_tower[1]);

  hmpc_tower_e[0] = new TH2D("hmpc_tower_e_arm0","tower energy distribution arm 0",300,0,30,288,-0.5,287.5);
  hmpc_tower_e[0]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_tower_e[0]->GetYaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_tower_e[0]);

  hmpc_tower_e[1] = new TH2D("hmpc_tower_e_arm1","tower energy distribution arm 1",300,0,30,288,-0.5,287.5);
  hmpc_tower_e[1]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_tower_e[1]->GetYaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_tower_e[1]);
 

  return EVENT_OK;
}

int mMpcExDataAna::mpc_tower_study(){
  int NMpcTowers = _mpc_tower_container->size();
  set<int> bad_tower_list;
  double sum_e[2] = {0.,0.};
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
//    int mtow_ch = tow_ch;
//    if(arm == 1) mtow_ch = tow_ch - 288;
    if(e_tower < 0.0) continue;

  //it is peak or not
    int gridx = _mpc_map->getGridX(tow_ch);
    int gridy = _mpc_map->getGridY(tow_ch);
    bool is_peak = true;
    double sum3x3 = e_tower;
    for(int ix = -1;ix <= 1;ix++){
      for(int iy = -1;iy <= 1;iy++){
        if((ix == 0) && (iy == 0)) continue;
        int near_ch = _mpc_map->getFeeCh(gridx+ix,gridy+iy,arm);
	if(near_ch < 0) continue;
        int index = _mpc_tower_container->findTower(near_ch);
	if(index < 0) continue;
	mpcTowerContent* tcont = _mpc_tower_container->getTower(index);
	double t_e = tcont->get_energy();
	if(t_e < 0.0) continue;
	sum3x3 += t_e;
	if(t_e > e_tower) is_peak = false;
      }//ix
    }//iy
    if(is_peak){
      int fill_ch = tow_ch;
      if(arm == 1) fill_ch = tow_ch - 288;
      hmpc_peak_tower[arm]->Fill(e_tower,e_tower/sum3x3,fill_ch);
      if(e_tower/sum3x3 > 0.95) hmpc_tower_e[arm]->Fill(e_tower,fill_ch);
    }

    if(is_peak && (e_tower/sum3x3 > 0.90) && (e_tower > 13)){
      bad_tower_list.insert(itower);
      continue;
    }
    sum_e[arm] += e_tower;
  }
//  cout <<"mpc tower study , Ntowers: "<<NMpcTowers<<endl;
  for(int itower = 0; itower < NMpcTowers;itower++){
    if(bad_tower_list.find(itower)!= bad_tower_list.end()) continue; 
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    int fill_ch = tow_ch;
    if(arm == 1) fill_ch = tow_ch - 288;
//    hmpc_tower_e[arm]->Fill(e_tower,fill_ch);
    if(e_tower < 0.0) continue;

    double x = _mpc_map->getX(tow_ch); 
    double y = _mpc_map->getY(tow_ch);
    int binx0 = hmpc_fired_tower[arm]->GetXaxis()->FindBin(x-0.9);
    int binx1 = hmpc_fired_tower[arm]->GetXaxis()->FindBin(x+0.9);
    int biny0 = hmpc_fired_tower[arm]->GetYaxis()->FindBin(y-0.9);
    int biny1 = hmpc_fired_tower[arm]->GetYaxis()->FindBin(y+0.9);
    for(int i = binx0;i <=binx1;i++){
      for(int j = biny0;j <=biny1;j++){
//	cout<<"bin "<<bin<<endl;
//	cout <<"tower e "<<e_tower<<endl;  
        double content = hmpc_fired_tower[arm]->GetBinContent(i,j);
	if(content < 0 )cout<<"bad content "<<endl;
	hmpc_fired_tower[arm]->SetBinContent(i,j,content + e_tower);
      }
    }
    binx0 = hmpc_fired_tower_e[arm]->GetXaxis()->FindBin(x-0.9);
    binx1 = hmpc_fired_tower_e[arm]->GetXaxis()->FindBin(x+0.9);
    biny0 = hmpc_fired_tower_e[arm]->GetYaxis()->FindBin(y-0.9);
    biny1 = hmpc_fired_tower_e[arm]->GetYaxis()->FindBin(y+0.9);
    int binz  = hmpc_fired_tower_e[arm]->GetZaxis()->FindBin(e_tower);
    for(int i = binx0;i <=binx1;i++){
      for(int j = biny0;j <=biny1;j++){
        double content = hmpc_fired_tower_e[arm]->GetBinContent(i,j,binz);
	hmpc_fired_tower_e[arm]->SetBinContent(i,j,binz,e_tower+content);
      }//j
    }//i
  }//itower
 
  return EVENT_OK;
}

int mMpcExDataAna::mpc_tower_shape_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hmpc_tower_shape[0] = new TH3D("hmpc_tower_shape_arm0","tower adc shape arm 0",12,-0.5,11.5,500,-50.5,449.5,288,-0.5,287.5);
  hmpc_tower_shape[0]->GetXaxis()->SetTitle("sample point");
  hmpc_tower_shape[0]->GetYaxis()->SetTitle("ADC - Ped");
  hmpc_tower_shape[0]->GetZaxis()->SetTitle("channel");
  hm->registerHisto(hmpc_tower_shape[0]);

  hmpc_tower_shape[1] = new TH3D("hmpc_tower_shape_arm1","tower adc shape arm 1",12,-0.5,11.5,500,-50.5,449.5,288,-0.5,287.5);
  hmpc_tower_shape[1]->GetXaxis()->SetTitle("sample point");
  hmpc_tower_shape[1]->GetYaxis()->SetTitle("ADC - Ped");
  hmpc_tower_shape[1]->GetZaxis()->SetTitle("channel");
  hm->registerHisto(hmpc_tower_shape[1]);

  hmpc_peak_sample[0] = new TH2D("hmpc_peak_sample_arm0","Tower Peak sample arm 0",12,-0.5,11.5,288,-0.5,287.5);
  hmpc_peak_sample[0]->GetXaxis()->SetTitle("Sample point");
  hmpc_peak_sample[0]->GetYaxis()->SetTitle("channel");
  hm->registerHisto(hmpc_peak_sample[0]);

  hmpc_peak_sample[1] = new TH2D("hmpc_peak_sample_arm1","Tower Peak sample arm 1",12,-0.5,11.5,288,-0.5,287.5);
  hmpc_peak_sample[1]->GetXaxis()->SetTitle("Sample point");
  hmpc_peak_sample[1]->GetYaxis()->SetTitle("channel");
  hm->registerHisto(hmpc_peak_sample[1]);

  hmpc_tower_chi2[0] = new TH3D("hmpc_tower_chi2_arm0","tower fit chi2 arm0",300,0,150,50,-0.5,49.5,288,-0.5,287.5);
  hmpc_tower_chi2[0]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_tower_chi2[0]->GetYaxis()->SetTitle("chi2/NDF");
  hmpc_tower_chi2[0]->GetZaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_tower_chi2[0]);

  hmpc_tower_chi2[1] = new TH3D("hmpc_tower_chi2_arm1","tower fit chi2 arm1",300,0,150,50,-0.5,49.5,288,-0.5,287.5);
  hmpc_tower_chi2[1]->GetXaxis()->SetTitle("tower energy/GeV");
  hmpc_tower_chi2[1]->GetYaxis()->SetTitle("chi2/NDF");
  hmpc_tower_chi2[1]->GetZaxis()->SetTitle("tower channel");
  hm->registerHisto(hmpc_tower_chi2[1]);


  return EVENT_OK;
}


int mMpcExDataAna::mpc_tower_shape_study(){
  typedef pair<int,int> Sample;
  typedef vector<Sample> Samples;
  map<int,Samples> tower_samples;
  short nentries = _mpc_sample_container->get_nentries();
  for(int isample=0; isample<nentries; isample++){
    mpcSample *samp = _mpc_sample_container->GetSample(isample);
    if ( samp==0 ) continue;
    int ch = samp->get_ch();
    if( _mpc_map->getGridX(ch)<0 ) continue;
    int sample = (int) samp->get_sample();
    int adc = samp->get_adc();
//    cout <<ch<<" "<<sample<<" "<<adc<<" "<<ped<<endl;
    if(tower_samples.find(ch) != tower_samples.end()){
      map<int,Samples>::iterator iter = tower_samples.find(ch);
      Samples* tsampes = &(iter->second);
      tsampes->push_back(Sample(sample,adc));
    }
    else{
      Samples new_samples;
      new_samples.push_back(Sample(sample,adc));
      tower_samples.insert(pair<int,Samples>(ch,new_samples));
    }
  }

  
  map<int,int> tower_raw_list;
  int NMpcRawTowers = _mpc_raw_container->size();
  for(int iraw = 0;iraw < NMpcRawTowers;iraw++){
     mpcRawContent *raw = _mpc_raw_container->getTower(iraw);
     int raw_ch = raw->get_ch();
     int chi2 = raw->get_quality();
     tower_raw_list.insert(pair<int,int>(raw_ch,chi2));
  }
  
  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    float ped = _mpc_calib->get_ped(tow_ch);
    int fill_ch = tow_ch;
    if(arm == 1) fill_ch = tow_ch - 288;
    
    map<int,int>::iterator raw_iter = tower_raw_list.find(tow_ch);
    hmpc_tower_chi2[arm]->Fill(e_tower,raw_iter->second,fill_ch);
//    float ped_rms = _mpc_calib->get_pedrms(tow_ch);
//    cout <<"arm "<<arm<<" tower channel "<<tow_ch<<" energy "<<e_tower<<" ped "<<ped<<" pedrms "<<ped_rms<<endl;
    map<int,Samples>::iterator iter = tower_samples.find(tow_ch);
    Samples* tsampes = &(iter->second);
    if(e_tower < 1 && e_tower > 0.8){
      int peak_sample = 0;
      int peak_adc = 0;
      for(unsigned int i = 0;i < tsampes->size();i++){
//        cout << (*tsampes)[i].first<<" "<<(*tsampes)[i].second<<endl;
//        hmpc_tower_shape[arm]->Fill((*tsampes)[i].first,(*tsampes)[i].second - ped,fill_ch);
	if((*tsampes)[i].second > peak_adc) {
           peak_sample = (*tsampes)[i].first;
	   peak_adc = (*tsampes)[i].second;
	}
      }
      hmpc_peak_sample[arm]->Fill(peak_sample,fill_ch);
      int delta_sample = peak_sample - 7;
      for(unsigned int i = 0;i < tsampes->size();i++){
        int fill_sample = (*tsampes)[i].first;
	fill_sample = fill_sample - delta_sample;
	if(fill_sample < 0 || fill_sample > 11) continue;
        hmpc_tower_shape[arm]->Fill(fill_sample,(*tsampes)[i].second - ped,fill_ch);
      }
    }
  }
  return EVENT_OK;
}

void mMpcExDataAna::new_shower_module_display(){
  
  cout <<"draw new shower display"<<endl;
  //clean the hist vector
  for(unsigned int i = 0;i < _hist_list.size();i++){
    if(_hist_list[i]) delete _hist_list[i];
  }
  _hist_list.clear();
  
  //clean the canvas vector
  for(unsigned int i = 0;i < _c_list.size();i++){
    if(_c_list[i]) delete _c_list[i];
  }
  _c_list.clear();

  TH2D* hhitxy[2] = {NULL,NULL};
  hhitxy[0] = new TH2D("hhitxy0","hhitxy_arm0",600,-24,24,600,-24,24);
  hhitxy[0]->GetXaxis()->SetTitle("cm");
  hhitxy[0]->GetYaxis()->SetTitle("cm");

  hhitxy[1] = new TH2D("hhitxy1","hhitxy_arm1",600,-24,24,600,-24,24);
  hhitxy[1]->GetXaxis()->SetTitle("cm");
  hhitxy[1]->GetYaxis()->SetTitle("cm");

  TH2D* htowerxy[2] = {NULL,NULL};
  htowerxy[0] = new TH2D("htowerxy0","htowerxy_arm0",600,-24,24,600,-24,24);
  htowerxy[0]->GetXaxis()->SetTitle("cm");
  htowerxy[0]->GetYaxis()->SetTitle("cm");

  htowerxy[1] = new TH2D("htowerxy1","htowerxy_arm1",600,-24,24,600,-24,24);
  htowerxy[1]->GetXaxis()->SetTitle("cm");
  htowerxy[1]->GetYaxis()->SetTitle("cm");


  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double tow_x = _mpc_map->getX(tow_ch);
    double tow_y = _mpc_map->getY(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0) continue;
    int binx0 = htowerxy[arm]->GetXaxis()->FindBin(tow_x - 1.);
    int binx1 = htowerxy[arm]->GetXaxis()->FindBin(tow_x + 1.);
    int biny0 = htowerxy[arm]->GetYaxis()->FindBin(tow_y - 1.);
    int biny1 = htowerxy[arm]->GetYaxis()->FindBin(tow_y + 1.);
    for(int ibinx = binx0;ibinx <= binx1;ibinx++){
      for(int ibiny = biny0;ibiny <= biny1;ibiny++){
        double content = htowerxy[arm]->GetBinContent(ibinx,ibiny); 
	htowerxy[arm]->SetBinContent(ibinx,ibiny,content+e_tower);
      }//ibinx
    }//ibiny
  }

  _hist_list.push_back(htowerxy[0]);
  _hist_list.push_back(htowerxy[1]);



  for(unsigned int i = 0;i < _mpcex_hit_container->size();i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    double hit_x = hit->x();
    double hit_y = hit->y();
    int layer = hit->layer();
    int arm = hit->arm();
    double delta_x = MpcExConstants::MINIPAD_SHORT_LENGTH;
    double delta_y = MpcExConstants::MINIPAD_LONG_LENGTH;
    if(layer%2 == 1){
          delta_x = MpcExConstants::MINIPAD_LONG_LENGTH;
	  delta_y = MpcExConstants::MINIPAD_SHORT_LENGTH;
    }
    int binx0 = hhitxy[arm]->GetXaxis()->FindBin(hit_x - delta_x);
    int binx1 = hhitxy[arm]->GetXaxis()->FindBin(hit_x + delta_x);
    int biny0 = hhitxy[arm]->GetYaxis()->FindBin(hit_y - delta_y);
    int biny1 = hhitxy[arm]->GetYaxis()->FindBin(hit_y + delta_y);
    for(int ibinx = binx0;ibinx <= binx1;ibinx++){
      for(int ibiny = biny0;ibiny <= biny1;ibiny++){
        double content = hhitxy[arm]->GetBinContent(ibinx,ibiny); 
        double good_q = get_good_q(hit);
	if(good_q < 0) continue;
	hhitxy[arm]->SetBinContent(ibinx,ibiny,content+good_q);
        content = htowerxy[arm]->GetBinContent(ibinx,ibiny); 
	htowerxy[arm]->SetBinContent(ibinx,ibiny,content+good_q);
      }//ibinx
    }//ibiny
  }

  _hist_list.push_back(hhitxy[0]);
  _hist_list.push_back(hhitxy[1]);


  unsigned int NLshowers = _mpcex_lshower_container->size();
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int Nfired_layers = lshower->get_n_fired_layers();
    int arm = lshower->get_arm();
    int n_fired_layers = lshower->get_n_fired_layers();
    int n_fired_towers = lshower->get_n_fired_towers3x3();
//    int n_hits = lshower->get_hits_num();
    int first_fired_layer = lshower->get_first_fired_layer();

//    cout <<"fired layers "<<Nfired_layers<<endl; 
    if(n_fired_towers == 0 && (n_fired_layers == 8 - first_fired_layer) && (n_fired_layers > 7)){ 
      double hsx_rms = lshower->get_rms_hsx();
      double hsy_rms = lshower->get_rms_hsy();
      cout <<"shower index: "<<i<<" fired layers: "<<Nfired_layers<<" mpcex e: "<<lshower->get_mpcex_e()<<endl;
      cout <<"number of hits: "<<lshower->get_hits_num()<<" hsx_rms "<<hsx_rms<<" hsy_rms "<<hsy_rms<<endl;
      
      char name[100];
      sprintf(name,"htemp%d_arm%d",i,arm);
      TH2D* htemp = new TH2D(name,name,600,-24,24,600,-24,24);
      htemp->GetXaxis()->SetTitle("X/cm");
      htemp->GetYaxis()->SetTitle("Y/cm");
      //draw the mpcex hits
      for(unsigned int j = 0;j < lshower->get_hits_num();j++){
        unsigned int hit_key = lshower->get_hit(j);
	TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
	double hit_x = hit->x();
	double hit_y = hit->y();
	int layer = hit->layer();
	double delta_x = MpcExConstants::MINIPAD_SHORT_LENGTH;
	double delta_y = MpcExConstants::MINIPAD_LONG_LENGTH;
	if(layer%2 == 1){
          delta_x = MpcExConstants::MINIPAD_LONG_LENGTH;
	  delta_y = MpcExConstants::MINIPAD_SHORT_LENGTH;
	}
        int binx0 = htemp->GetXaxis()->FindBin(hit_x - delta_x);
	int binx1 = htemp->GetXaxis()->FindBin(hit_x + delta_x);
	int biny0 = htemp->GetYaxis()->FindBin(hit_y - delta_y);
	int biny1 = htemp->GetYaxis()->FindBin(hit_y + delta_y);
	for(int ibinx = binx0;ibinx <= binx1;ibinx++){
          for(int ibiny = biny0;ibiny <= biny1;ibiny++){
            double content = htemp->GetBinContent(ibinx,ibiny); 
            double good_q = get_good_q(hit);
	    if(good_q < 0) continue;
	    htemp->SetBinContent(ibinx,ibiny,content+good_q);
	  }//ibinx
	}//ibiny
      }//j
      //draw the mpc tower
      for(int itx = 0;itx < 5;itx++){
        for(int ity = 0;ity < 5;ity++){
          int tower_ch = lshower->get_mpc_towers_ch(itx,ity);
	  if(tower_ch < 0) continue;
	  double tower_x = _mpc_map->getX(tower_ch);
	  double tower_y = _mpc_map->getY(tower_ch);
          int binx0 = htemp->GetXaxis()->FindBin(tower_x - 1.0);
	  int binx1 = htemp->GetXaxis()->FindBin(tower_x + 1.0);
	  int biny0 = htemp->GetYaxis()->FindBin(tower_y - 1.0);
	  int biny1 = htemp->GetYaxis()->FindBin(tower_y + 1.0);
          double tower_e = lshower->get_mpc_towers_e(itx,ity);
	  if(tower_e < 0) continue;
	  cout <<"tower e "<<tower_e
	       <<" tower ch "<<tower_ch
	       <<" gridx: "<<_mpc_map->getGridX(tower_ch)
	       <<" gridy: "<<_mpc_map->getGridY(tower_ch)
	       <<" tower_x: "<<tower_x
	       <<" tower_y: "<<tower_y
	       <<endl;
	  for(int ibinx = binx0;ibinx <= binx1;ibinx++){
            for(int ibiny = biny0;ibiny <= biny1;ibiny++){
              double content = htemp->GetBinContent(ibinx,ibiny); 
	      htemp->SetBinContent(ibinx,ibiny,content+tower_e);
	    }//ibinx
	  }//ibiny
	}
      }//draw mpc
      _hist_list.push_back(htemp);
    }//Nfired_layers
  }//i
  //draw hist gram
  for(unsigned int ic = 0;ic < _hist_list.size();ic++){
    char name[100];
    sprintf(name,"c_%d",ic);
    TCanvas* c = new TCanvas(name,name,800,800);
    c->cd();
    _hist_list[ic]->Draw("colz");
  }

}

double mMpcExDataAna::get_good_q(TMpcExHit* hit){
/*  
  if(!hit) return -9999.9;  
  bool good_high = hit->isGoodHighHit();
  bool good_low = hit->isGoodLowHit();
  bool state_high = (hit->state_high()== TMpcExHit::PEDESTAL_SUBTRACTED);
  bool state_low = (hit->state_low()== TMpcExHit::PEDESTAL_SUBTRACTED);

  
  unsigned int key = hit->key();
  double high_q = hit->high();
  double low_q = hit->low();

  TMpcExCalib* calib = _mpcex_calibs->get(key);
  double high_ped = calib->high_pedestal();
  double low_ped = calib->low_pedestal();

  double hit_q = -9999.9;
  if(high_q > 9 && (high_q+high_ped) < 254 && good_high && state_high) hit_q = high_q;
  else if(low_q > 1. && (low_q+low_ped) < 254 && good_low && state_low) hit_q = low_q*4.5; //assume high low ration is 4.5

//assume MIP is located at 150KeV,ADC-Ped = 19

  return hit_q; 
  
//  hit_q = hit_q*150./19.;
  
//  return hit_q/1.0e6/SAMPLING_FRACTION;
*/
  if(!hit) return -9999.9;
//  cout <<"high "<<hit->high()<<" low "<<hit->low()<<" combined "<<hit->combined()<<endl;
  double hit_q = -9999.9;
  if(hit->state_combined()!=TMpcExHit::INVALID) hit_q = hit->combined();
  if(hit_q < 0./2.) hit_q = -9999.9;
  return hit_q;

}

int mMpcExDataAna::mpcex_new_shower_module_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hshower_hxy_match[0] = new TH3D("hshower_hxy_match_arm0","hxy match arm 0",100,-0.1,0.1,100,-0.1,0.1,200,0,0.2);
  hshower_hxy_match[0]->GetXaxis()->SetTitle("hx");
  hshower_hxy_match[0]->GetYaxis()->SetTitle("hy");
  hshower_hxy_match[0]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hxy_match[0]);

  hshower_hxy_match[1] = new TH3D("hshower_hxy_match_arm1","hxy match arm 1",100,-0.1,0.1,100,-0.1,0.1,200,0,0.2);
  hshower_hxy_match[1]->GetXaxis()->SetTitle("hx");
  hshower_hxy_match[1]->GetYaxis()->SetTitle("hy");
  hshower_hxy_match[1]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hxy_match[1]);

  hshower_hxy_match2[0] = new TH3D("hshower_hxy_match2_arm0","hxy match arm 0",100,-0.1,0.1,100,-0.1,0.1,200,0,0.2);
  hshower_hxy_match2[0]->GetXaxis()->SetTitle("hx");
  hshower_hxy_match2[0]->GetYaxis()->SetTitle("hy");
  hshower_hxy_match2[0]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hxy_match2[0]);

  hshower_hxy_match2[1] = new TH3D("hshower_hxy_match2_arm1","hxy match arm 1",100,-0.1,0.1,100,-0.1,0.1,200,0,0.2);
  hshower_hxy_match2[1]->GetXaxis()->SetTitle("hx");
  hshower_hxy_match2[1]->GetYaxis()->SetTitle("hy");
  hshower_hxy_match2[1]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hxy_match2[1]);

  hshower_hits_match[0] = new TH3D("hshower_hits_match_arm0","shower hits match arm 0",200,0,2,200,0,2,200,0,0.2);
  hshower_hits_match[0]->GetXaxis()->SetTitle("number of hits");
  hshower_hits_match[0]->GetYaxis()->SetTitle("mpcex energy");
  hshower_hits_match[0]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hits_match[0]);

  hshower_hits_match[1] = new TH3D("hshower_hits_match_arm1","shower hits match arm 1",200,0,2,200,0,2,200,0,0.2);
  hshower_hits_match[1]->GetXaxis()->SetTitle("number of hits");
  hshower_hits_match[1]->GetYaxis()->SetTitle("mpcex energy");
  hshower_hits_match[1]->GetZaxis()->SetTitle("rms");
  hm->registerHisto(hshower_hits_match[1]);



  return EVENT_OK;
}


int mMpcExDataAna::mpcex_new_shower_module_study(){
  
  //calcluate the hough space of all hit on mpcex 
  double hsx[2] = {0.0,0.0};
  double hsy[2] = {0.0,0.0};
  double hsx2[2] = {0.0,0.0};
  double hsy2[2] = {0.0,0.0};
  double norm_x[2] = {0.0,0.0};
  double norm_y[2] = {0.0,0.0};
  int hit_num[2] = {0,0};
  double mpcex_tot_e[2] = {0.0,0.0};
  for(unsigned int i = 0;i < _mpcex_hit_container->size();i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    double hit_hsx = hit->hsx(_vertex);
    double hit_hsy = hit->hsy(_vertex);
    int arm = hit->arm();
    double good_q = get_good_q(hit);
    if(good_q < 0) continue;
    hit_num[arm]++;
    mpcex_tot_e[arm] += good_q;
    int layer = hit->layer();
    if(layer%2 == 0){
      hsx[arm] += 8.0*good_q*hit_hsx;
      hsx2[arm] += 8.0*good_q*hit_hsx*hit_hsx;
      norm_x[arm] += 8.0*good_q;

      hsy[arm] += good_q*hit_hsy;
      hsy2[arm] += good_q*hit_hsy*hit_hsy;
      norm_y[arm] += good_q;
    }
    else{
      hsx[arm] += good_q*hit_hsx;
      hsx2[arm] += good_q*hit_hsx*hit_hsx;
      norm_x[arm] += good_q;

      hsy[arm] += 8*good_q*hit_hsy;
      hsy2[arm] += 8*good_q*hit_hsy*hit_hsy;
      norm_y[arm] += 8*good_q;
    }
  }

  int max_shower_index[2] = {-1,-1};
  double max_e[2] = {0,0};
  unsigned int NLshowers = _mpcex_lshower_container->size();
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    double mpcex_e = lshower->get_mpcex_e();
    int arm = lshower->get_arm();
    if(mpcex_e > max_e[arm]){
      max_e[arm] = mpcex_e;
      max_shower_index[arm] = i;
    }
  }
  
  //mpc part
  int NMpcTowers = _mpc_tower_container->size();
  double mpc_hsx[2] = {0.0,0.0};
  double mpc_hsy[2] = {0.0,0.0};
  double mpc_norm[2] = {0.0,0.0};
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double tower_x = _mpc_map->getX(tow_ch);
    double tower_y = _mpc_map->getY(tow_ch);
    double tower_z = _mpc_map->getZ(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0) continue;
    mpc_hsx[arm] += tower_x/(tower_z - _vertex)*e_tower;
    mpc_hsy[arm] += tower_y/(tower_z - _vertex)*e_tower;
    mpc_norm[arm] += e_tower;
  }
  
  for(int arm = 0;arm < 2;arm++){
    if(max_shower_index[arm] < 0 ) continue; 
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(max_shower_index[arm]);
    double ls_hsx = lshower->get_hsx();
    double ls_hsy = lshower->get_hsy();
    int shower_hits_num = lshower->get_hits_num();
    double mpcex_e = lshower->get_mpcex_e();
    hsx[arm] = hsx[arm]/norm_x[arm];
    hsy[arm] = hsy[arm]/norm_y[arm];
    hsx2[arm] = hsx2[arm]/norm_x[arm];
    hsy2[arm] = hsy2[arm]/norm_y[arm];
    mpc_hsx[arm] = mpc_hsx[arm]/mpc_norm[arm];
    mpc_hsy[arm] = mpc_hsy[arm]/mpc_norm[arm];
    
    double rms_hsx = sqrt(hsx2[arm] - hsx[arm]*hsx[arm]);
    double rms_hsy = sqrt(hsy2[arm] - hsy[arm]*hsy[arm]);
    double rms_r = sqrt(rms_hsx*rms_hsx + rms_hsy*rms_hsy);
//   double dhsx = _hsx_prim - hsx[arm];
//   double dhsy = _hsy_prim - hsy[arm];
//    double dr = sqrt(dhsx*dhsx+dhsy*dhsy);
    if(fabs(_eta) < 3.1 || fabs(_eta) > 3.9) continue;
//    cout <<"dhsx: "<<ls_hsx - hsx[arm]<<" dhsy: "<<ls_hsy - hsy[arm]<<endl
//         <<" hit number ratio: "<<shower_hits_num*1./hit_num[arm]
//	 <<" energy ratio: "<<mpcex_e/mpcex_tot_e[arm]
//	 <<" dr: "<<dr<<endl;
    hshower_hxy_match[arm]->Fill(ls_hsx - hsx[arm],ls_hsy - hsy[arm],rms_r);
    hshower_hxy_match2[arm]->Fill(ls_hsx - mpc_hsx[arm],ls_hsy - mpc_hsy[arm],rms_r);
    hshower_hits_match[arm]->Fill(shower_hits_num*1./hit_num[arm],mpcex_e/mpcex_tot_e[arm],rms_r);
  }

  return EVENT_OK;
}

int mMpcExDataAna::lshower_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
 
  hdr[0] = new TH3D("hdr0","dr arm 0",100,0,20,50,-3.14159,3.14159,20,2.5,4.5);
  hdr[0]->GetXaxis()->SetTitle("dr/cm");
  hdr[0]->GetYaxis()->SetTitle("phi");
  hdr[0]->GetZaxis()->SetTitle("Eta");
  hm->registerHisto(hdr[0]);


  hdr[1] = new TH3D("hdr1","dr arm 1",100,0,20,50,-3.14159,3.14159,20,2.5,4.5);
  hdr[1]->GetXaxis()->SetTitle("dr/cm");
  hdr[1]->GetYaxis()->SetTitle("phi");
  hdr[1]->GetZaxis()->SetTitle("Eta");
  hm->registerHisto(hdr[1]);

  hdxy[0] = new TH3D("hdxy0","dx,dy,arm 0",100,-10,10,100,-10,10,200,0,2);
  hdxy[0]->GetXaxis()->SetTitle("dx/cm");
  hdxy[0]->GetYaxis()->SetTitle("dy/cm");
  hdxy[0]->GetZaxis()->SetTitle("mpcex e/GeV");
  hm->registerHisto(hdxy[0]);

  hdxy[1] = new TH3D("hdxy1","dx,dy,arm 1",100,-10,10,100,-10,10,200,0,2);
  hdxy[1]->GetXaxis()->SetTitle("dx/cm");
  hdxy[1]->GetYaxis()->SetTitle("dy/cm");
  hdxy[1]->GetZaxis()->SetTitle("mpcex e/GeV");
  hm->registerHisto(hdxy[1]);

  hdxy2[0] = new TH3D("hdxy20","dx,dy,arm 0",100,-10,10,100,-10,10,200,0,2);
  hdxy2[0]->GetXaxis()->SetTitle("dx/cm");
  hdxy2[0]->GetYaxis()->SetTitle("dy/cm");
  hdxy2[0]->GetZaxis()->SetTitle("first layer e/total e");
  hm->registerHisto(hdxy2[0]);

  hdxy2[1] = new TH3D("hdxy21","dx,dy,arm 1",100,-10,10,100,-10,10,200,0,2);
  hdxy2[1]->GetXaxis()->SetTitle("dx/cm");
  hdxy2[1]->GetYaxis()->SetTitle("dy/cm");
  hdxy2[1]->GetZaxis()->SetTitle("first layer e/total e");
  hm->registerHisto(hdxy2[1]);

  hdxy3[0] = new TH3D("hdxy30","dx,dy,arm 0",100,-10,10,100,-10,10,200,0,2);
  hdxy3[0]->GetXaxis()->SetTitle("dx/cm");
  hdxy3[0]->GetYaxis()->SetTitle("dy/cm");
  hdxy3[0]->GetZaxis()->SetTitle("rms/total e");
  hm->registerHisto(hdxy3[0]);

  hdxy3[1] = new TH3D("hdxy31","dx,dy,arm 1",100,-10,10,100,-10,10,200,0,2);
  hdxy3[1]->GetXaxis()->SetTitle("dx/cm");
  hdxy3[1]->GetYaxis()->SetTitle("dy/cm");
  hdxy3[1]->GetZaxis()->SetTitle("rms/total e");
  hm->registerHisto(hdxy3[1]);

  hlayer_e[0] = new TH3D("hlayer_e_0","layer e arm 0",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[0]->GetXaxis()->SetTitle("layer e");
  hlayer_e[0]->GetYaxis()->SetTitle("layer");
  hlayer_e[0]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[0]);

  hlayer_e[1] = new TH3D("hlayer_e_1","layer e arm 1",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[1]->GetXaxis()->SetTitle("layer e");
  hlayer_e[1]->GetYaxis()->SetTitle("layer");
  hlayer_e[1]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[1]);

  hlayer_e_rms[0] = new TH3D("hlayer_e_rms0","layer e arm 0",1000,0,0.1,8,-0.5,7.5,200,0,10);
  hlayer_e_rms[0]->GetXaxis()->SetTitle("layer e");
  hlayer_e_rms[0]->GetYaxis()->SetTitle("layer");
  hlayer_e_rms[0]->GetZaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hlayer_e_rms[0]);

  hlayer_e_rms[1] = new TH3D("hlayer_e_rms1","layer e arm 1",1000,0,0.1,8,-0.5,7.5,200,0,10);
  hlayer_e_rms[1]->GetXaxis()->SetTitle("layer e");
  hlayer_e_rms[1]->GetYaxis()->SetTitle("layer");
  hlayer_e_rms[1]->GetZaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hlayer_e_rms[1]);

  hfired[0] = new TH3D("hfired_arm0","fired arm 0",10,-0.5,9.5,9,-0.5,8.5,9,-0.5,8.5);
  hfired[0]->GetXaxis()->SetTitle("n fired towers3x3");
  hfired[0]->GetYaxis()->SetTitle("n fired layers");
  hfired[0]->GetZaxis()->SetTitle("n missed layers");
  hm->registerHisto(hfired[0]);

  hfired[1] = new TH3D("hfired_arm1","fired arm 1",10,-0.5,9.5,9,-0.5,8.5,9,-0.5,8.5);
  hfired[1]->GetXaxis()->SetTitle("n fired towers3x3");
  hfired[1]->GetYaxis()->SetTitle("n fired layers");
  hfired[1]->GetZaxis()->SetTitle("n missed layers");
  hm->registerHisto(hfired[1]);

  hlsxy[0] = new TH3D("hlsxy_arm0","lshower xy arm 0",100,-24,24,100,-24,24,1000,0,5);
  hlsxy[0]->GetXaxis()->SetTitle("X/cm");
  hlsxy[0]->GetYaxis()->SetTitle("Y/cm");
  hlsxy[0]->GetZaxis()->SetTitle("E/GeV");
  hm->registerHisto(hlsxy[0]);

  hlsxy[1] = new TH3D("hlsxy_arm1","lshower xy arm 1",100,-24,24,100,-24,24,1000,0,5);
  hlsxy[1]->GetXaxis()->SetTitle("X/cm");
  hlsxy[1]->GetYaxis()->SetTitle("Y/cm");
  hlsxy[1]->GetZaxis()->SetTitle("E/GeV");
  hm->registerHisto(hlsxy[1]);

  hrms_e[0] = new TH2D("hrms_e_0","RMS vs E arm 0",1000,0,1,200,0,10);
  hrms_e[0]->GetXaxis()->SetTitle("mpcex E/GeV");
  hrms_e[0]->GetYaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hrms_e[0]);

  hrms_e[1] = new TH2D("hrms_e_1","RMS vs E arm 1",1000,0,1,200,0,10);
  hrms_e[1]->GetXaxis()->SetTitle("mpcex E/GeV");
  hrms_e[1]->GetYaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hrms_e[1]);

  hkey_e[0] = new TH3D("key_e_0","Key vs E arm 0",26000,-0.5,26000-0.5,255,-0.5,254.5,9,-0.5,8.5);
  hkey_e[0]->GetXaxis()->SetTitle("key");
  hkey_e[0]->GetYaxis()->SetTitle("mpcex E/GeV");
  hkey_e[0]->GetZaxis()->SetTitle("fired layers");
  hm->registerHisto(hkey_e[0]);

  hkey_e[1] = new TH3D("key_e_1","Key vs E arm 1",26000,24000-0.5,50000-0.5,255,-0.5,254.5,9,-0.5,8.5);
  hkey_e[1]->GetXaxis()->SetTitle("key");
  hkey_e[1]->GetYaxis()->SetTitle("mpcex E/GeV");
  hkey_e[1]->GetZaxis()->SetTitle("fired layers");
  hm->registerHisto(hkey_e[1]);

  hhigh_low[0] = new TH2D("hhigh_low_arm0","high vs low arm 0",350,-60.5,289.5,350,-60.5,289.5);
  hhigh_low[0]->GetXaxis()->SetTitle("high q");
  hhigh_low[0]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hhigh_low[0]);

  hhigh_low[1] = new TH2D("hhigh_low_arm1","high vs low arm 1",350,-60.5,289.5,350,-60.5,289.5);
  hhigh_low[1]->GetXaxis()->SetTitle("high q");
  hhigh_low[1]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hhigh_low[1]);


//  hgrammy[0] = new Exogram("hgrammy0","Exogram arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
//  hm->registerHisto(hgrammy[0]);

//  hgrammy[1] = new Exogram("hgrammy1","Exogram arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
//  hm->registerHisto(hgrammy[1]);


  return EVENT_OK;
}

int mMpcExDataAna::lshower_study(){
  unsigned int NLshowers = _mpcex_lshower_container->size();
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    double mpc_z = MpcExConstants::MPC_REFERENCE_Z;
    if(arm == 0) mpc_z = -mpc_z;
    double hsx = lshower->get_hsx();
    double hsy = lshower->get_hsy();
    double ls_x = hsx*(mpc_z - _vertex);
    double ls_y = hsy*(mpc_z - _vertex);
    int Nhits = lshower->get_hits_num();
//    int center_tower_ch = lshower->get_mpc_towers_ch(2,2);
//    double closest_tx = -1;
//    double closest_ty = -1;
    double mpcex_e = lshower->get_mpcex_e();
    int n_fired_layers = lshower->get_n_fired_layers();
    int n_fired_towers = lshower->get_n_fired_towers3x3();
    int first_fired_layer = lshower->get_first_fired_layer();
//    double last_layer_e = lshower->get_e_layer(7);
    if(n_fired_layers > 3){
      double mpc_x = 0;
      double mpc_y = 0;
      double norm = 0;
      for(int ix = 1;ix < 4;ix++){
        for(int iy = 1;iy < 4;iy++){
          int tower_ch = lshower->get_mpc_towers_ch(ix,iy);
	  double tower_e = lshower->get_mpc_towers_e(ix,iy);
	  if(tower_ch < 0 || tower_e < 0) continue;
          double tow_x = _mpc_map->getX(tower_ch);
	  double tow_y = _mpc_map->getY(tower_ch);
          mpc_x += tow_x*tower_e;
	  mpc_y += tow_y*tower_e;
	  norm += tower_e;
	}//iy
      }//ix

      mpc_x = mpc_x/norm;
      mpc_y = mpc_y/norm;

      double dx = ls_x - mpc_x;
      double dy = ls_y - mpc_y;
      double dr = sqrt(dx*dx+dy*dy);
      double phi = atan2(ls_y,ls_x);
      double theta = atan(sqrt(hsx*hsx+hsy*hsy));
      double eta = -log(tan(theta/2.));
      //cut on eta
      if(fabs(eta) > 3.8 || fabs(eta) < 3.1) continue;
//      int first_fired_layer = lshower->get_first_fired_layer();
//      double first_layer_e = lshower->get_e_layer(first_fired_layer);
      double rms_hsx = lshower->get_rms_hsx();
      double rms_hsy = lshower->get_rms_hsy();
      double rms = sqrt(rms_hsx*rms_hsx + rms_hsy*rms_hsy);
//      cout <<"eta: "<<eta<<endl;
      hdr[arm]->Fill(dr,phi,fabs(eta));
      hdxy[arm]->Fill(dx,dy,mpcex_e);
      hdxy2[arm]->Fill(dx,dy,mpcex_e/(mpcex_e+norm));
      hdxy3[arm]->Fill(dx,dy,rms/mpcex_e);
    }
    
    
    hfired[arm]->Fill(n_fired_towers,n_fired_layers,8 - first_fired_layer - n_fired_layers);
    if((n_fired_layers == 8) && (n_fired_layers == Nhits) ){
      for(int ilayer = first_fired_layer;ilayer < MpcExConstants::NLAYERS;ilayer++){
        double le = lshower->get_e_layer(ilayer);
	if(le <= 0 ) continue;
	double rms_hsx = lshower->get_rms_hsx()*(mpc_z - _vertex);
        double rms_hsy = lshower->get_rms_hsy()*(mpc_z - _vertex);
        double rms = sqrt(rms_hsx*rms_hsx + rms_hsy*rms_hsy);

	hlayer_e[arm]->Fill(le,ilayer - first_fired_layer,mpcex_e);
        hlsxy[arm]->Fill(ls_x,ls_y,mpcex_e);
	hlayer_e_rms[arm]->Fill(le,ilayer - first_fired_layer,rms);
        hrms_e[arm]->Fill(mpcex_e,rms);

        for(int ihit = 0;ihit < Nhits;ihit++){
          unsigned int hit_key = lshower->get_hit(ihit);
          TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
//          double good_q = get_good_q(hit);
//          if(good_q < 0) continue;
          hkey_e[arm]->Fill(hit_key,hit->high(),n_fired_layers);	
	  hhigh_low[arm]->Fill(hit->high(),hit->low());
        }	
      }
    }
  }//i
  return EVENT_OK; 
}

int mMpcExDataAna::lshower_mip_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
   
  hlayer_e[0] = new TH3D("hlayer_e_0","layer e arm 0",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[0]->GetXaxis()->SetTitle("layer e");
  hlayer_e[0]->GetYaxis()->SetTitle("layer");
  hlayer_e[0]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[0]);

  hlayer_e[1] = new TH3D("hlayer_e_1","layer e arm 1",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[1]->GetXaxis()->SetTitle("layer e");
  hlayer_e[1]->GetYaxis()->SetTitle("layer");
  hlayer_e[1]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[1]);

  hlayer_adc[0] = new TH3D("hlayer_adc_arm0","layer ADC arm 0",300,-20-0.5,280-0.5,16,-4-0.5,12-0.5,1000,0,2040);
  hlayer_adc[0]->GetXaxis()->SetTitle("layer ADC");
  hlayer_adc[0]->GetYaxis()->SetTitle("layer");
  hlayer_adc[0]->GetZaxis()->SetTitle("total ADC");
  hm->registerHisto(hlayer_adc[0]);

  hlayer_adc[1] = new TH3D("hlayer_adc_arm1","layer ADC arm 1",300,-20-0.5,280-0.5,16,-4-0.5,12-0.5,1000,0,2040);
  hlayer_adc[1]->GetXaxis()->SetTitle("layer ADC");
  hlayer_adc[1]->GetYaxis()->SetTitle("layer");
  hlayer_adc[1]->GetZaxis()->SetTitle("total ADC");
  hm->registerHisto(hlayer_adc[1]);

  hpair_channel[0] = new TH2D("hpair_channel_arm0","pair channels high ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel[0]->GetXaxis()->SetTitle("MIP high ADC");
  hpair_channel[0]->GetYaxis()->SetTitle("Fake MIP high ADC");
  hm->registerHisto(hpair_channel[0]);

  hpair_channel[1] = new TH2D("hpair_channel_arm1","pair channels high ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel[1]->GetXaxis()->SetTitle("MIP high ADC");
  hpair_channel[1]->GetYaxis()->SetTitle("Fake MIP high ADC");
  hm->registerHisto(hpair_channel[1]);

  hpair_channel2[0] = new TH2D("hpair_channel2_arm0","pair channels low ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel2[0]->GetXaxis()->SetTitle("MIP low ADC");
  hpair_channel2[0]->GetYaxis()->SetTitle("Fake MIP low ADC");
  hm->registerHisto(hpair_channel2[0]);

  hpair_channel2[1] = new TH2D("hpair_channel2_arm1","pair channels low ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel2[1]->GetXaxis()->SetTitle("MIP low ADC");
  hpair_channel2[1]->GetYaxis()->SetTitle("Fake MIP low ADC");
  hm->registerHisto(hpair_channel2[1]);


  hrms_e[0] = new TH2D("hrms_e_0","RMS vs E arm 0",1000,0,1,200,0,10);
  hrms_e[0]->GetXaxis()->SetTitle("mpcex E/GeV");
  hrms_e[0]->GetYaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hrms_e[0]);

  hrms_e[1] = new TH2D("hrms_e_1","RMS vs E arm 1",1000,0,1,200,0,10);
  hrms_e[1]->GetXaxis()->SetTitle("mpcex E/GeV");
  hrms_e[1]->GetYaxis()->SetTitle("RMS/cm");
  hm->registerHisto(hrms_e[1]);

  hkey_high[0] = new TH2D("key_high_0","Key vs high arm 0",26000,-0.5,26000-0.5,255,-0.5,254.5);
  hkey_high[0]->GetXaxis()->SetTitle("key");
  hkey_high[0]->GetYaxis()->SetTitle("high adc");
  hm->registerHisto(hkey_high[0]);

  hkey_high[1] = new TH2D("key_high_1","Key vs high arm 1",26000,24000-0.5,50000-0.5,255,-0.5,254.5);
  hkey_high[1]->GetXaxis()->SetTitle("key");
  hkey_high[1]->GetYaxis()->SetTitle("high adc");
  hm->registerHisto(hkey_high[1]);

  hkey_low[0] = new TH2D("key_low_0","Key vs low arm 0",26000,-0.5,26000-0.5,295,-40.5,254.5);
  hkey_low[0]->GetXaxis()->SetTitle("key");
  hkey_low[0]->GetYaxis()->SetTitle("low adc");
  hm->registerHisto(hkey_low[0]);

  hkey_low[1] = new TH2D("key_low_1","Key vs low arm 1",26000,24000-0.5,50000-0.5,295,-40.5,254.5);
  hkey_low[1]->GetXaxis()->SetTitle("key");
  hkey_low[1]->GetYaxis()->SetTitle("low adc");
  hm->registerHisto(hkey_low[1]);

  hmpcex_mip_bbc2[0] = new TH2D("hmpcex_mip_bbc2_arm0","number of mip vs bbc charge arm 0",100,-0.5,100-0.5,100,-0.5,100-0.5);
  hmpcex_mip_bbc2[0]->GetYaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc2[0]->GetXaxis()->SetTitle("bbc charge");
  hm->registerHisto(hmpcex_mip_bbc2[0]);

  hmpcex_mip_bbc2[1] = new TH2D("hmpcex_mip_bbc2_arm1","number of mip vs bbc charge arm 1",100,-0.5,100-0.5,100,-0.5,100-0.5);
  hmpcex_mip_bbc2[1]->GetYaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc2[1]->GetXaxis()->SetTitle("bbc charge");
  hm->registerHisto(hmpcex_mip_bbc2[1]);

  hhigh_low[0] = new TH2D("hhigh_low_arm0","high vs low arm 0",350,-60.5,289.5,350,-60.5,289.5);
  hhigh_low[0]->GetXaxis()->SetTitle("high q");
  hhigh_low[0]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hhigh_low[0]);

  hhigh_low[1] = new TH2D("hhigh_low_arm1","high vs low arm 1",350,-60.5,289.5,350,-60.5,289.5);
  hhigh_low[1]->GetXaxis()->SetTitle("high q");
  hhigh_low[1]->GetYaxis()->SetTitle("low q");
  hm->registerHisto(hhigh_low[1]);

  hneg_total[0] = new TH2D("hneg_total_arm0","zero supressed vs total arm 0",200,0,2000,200,0,2000);
  hneg_total[0]->GetXaxis()->SetTitle("total hits");
  hneg_total[0]->GetYaxis()->SetTitle("zero supressed hits");
  hm->registerHisto(hneg_total[0]);

  hneg_total[1] = new TH2D("hneg_total_arm1","zero supressed vs total arm 1",200,0,2000,200,0,2000);
  hneg_total[1]->GetXaxis()->SetTitle("total hits");
  hneg_total[1]->GetYaxis()->SetTitle("zero supressed hits");
  hm->registerHisto(hneg_total[1]);


//  hgrammy[0] = new Exogram("hgrammy0","Exogram arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
//  hm->registerHisto(hgrammy[0]);

//  hgrammy[1] = new Exogram("hgrammy1","Exogram arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
//  hm->registerHisto(hgrammy[1]);


  return EVENT_OK;
}

int mMpcExDataAna::lshower_mip_study(){
  int MpcEx_Nhits = _mpcex_hit_container->size();
  double Ntotal_hits[2] ={0,0};
  double Nnegtv_hits[2] ={0,0};
  for(int i = 0;i < MpcEx_Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    double high_q = hit->high();
    if(high_q < 3) continue;
    int arm = hit->arm();
    Ntotal_hits[arm]++;
    double low_q = hit->low();
    if(low_q < -10)Nnegtv_hits[arm]++;
  }
 
  unsigned int NLshowers = _mpcex_lshower_container->size();
  int n_mip[2] = {0,0};
  std::set<unsigned int>used_key;
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    int Nhits = lshower->get_hits_num();
    double mpcex_e = lshower->get_mpcex_e();
    double mpc_z = MpcExConstants::MPC_REFERENCE_Z;
    if(arm == 0) mpc_z = -mpc_z;
    int n_fired_layers = lshower->get_n_fired_layers();
    int first_fired_layer = lshower->get_first_fired_layer();
    if((n_fired_layers == 8) ){
      //debugging code:
      for(int ilayer = first_fired_layer;ilayer < MpcExConstants::NLAYERS;ilayer++){
        double le = lshower->get_e_layer(ilayer);
	if(le <= 0 ) continue;
     	double rms_hsx = lshower->get_rms_hsx()*(mpc_z - _vertex);
        double rms_hsy = lshower->get_rms_hsy()*(mpc_z - _vertex);
        double rms = sqrt(rms_hsx*rms_hsx + rms_hsy*rms_hsy);
	hlayer_e[arm]->Fill(le,ilayer - first_fired_layer,mpcex_e);
        hrms_e[arm]->Fill(mpcex_e,rms);
      }
    
      bool good_high = true;     
      double total_adc = 0;
      bool wanted_track = false;
      double layer_high_q[8] = {0.};
      for(int ihit = 0;ihit < Nhits;ihit++){
        unsigned int hit_key = lshower->get_hit(ihit);
        TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
	double high_q = hit->high();
	int layer = hit->layer();
	layer_high_q[layer] = high_q;
//	high_q = get_good_q(hit);
//	double low_q = hit->low();
	total_adc += high_q;
	if(high_q <= 0 || _dead_map[hit->key()][0] != 0) {
	  good_high = false; 
//	  break;
	}
      }	
      
//      if((Ntotal_hits[arm] > 1) && (Nnegtv_hits[arm]/Ntotal_hits[arm] < 0.8))wanted_track = true;
//      if((layer_high_q[0] > 9 && (layer_high_q[7] > 9))) wanted_track = true;
      wanted_track = true;
      if(good_high && wanted_track){
        hneg_total[arm]->Fill(Ntotal_hits[arm],Nnegtv_hits[arm]);
        for(int ihit = 0;ihit < Nhits;ihit++){
          unsigned int hit_key = lshower->get_hit(ihit);
          TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
	  double high_q = hit->high();
	  int layer = hit->layer(); 
          hlayer_adc[arm]->Fill(high_q,layer-first_fired_layer,total_adc);
          hkey_high[arm]->Fill(hit_key,high_q);
	  hkey_low[arm]->Fill(hit_key,hit->low());
	  hhigh_low[arm]->Fill(hit->high(),hit->low());

	  //pair channel part
	  int chain = hit->chain();
	  unsigned int pair_key = hit_key;
	  if(chain == 0 || chain == 2) pair_key = pair_key + 12*64;
	  else pair_key = pair_key - 12*64;
          
	  TMpcExHit* pair_hit = _mpcex_hit_container->get_hit_by_key(pair_key);
	  if(pair_hit){
	    if(used_key.find(hit_key)!=used_key.end() ||used_key.find(pair_key)!=used_key.end()) continue;
	    used_key.insert(hit_key);
	    used_key.insert(pair_key);
	    hpair_channel[arm]->Fill(hit->high(),pair_hit->high());
	    hpair_channel2[arm]->Fill(hit->low(),pair_hit->low());
	  }
//	  else cout <<PHWHERE<<"No pair hit"<<endl;
        }	
//        double avg_q = total_adc/n_fired_layers;
//	if(fabs(avg_q - 19) < 6.4 ) 
 	n_mip[arm]++;
      }
    }
  }
  hmpcex_mip_bbc2[0]->Fill(_bbc_charge[0],n_mip[0]);
  hmpcex_mip_bbc2[1]->Fill(_bbc_charge[1],n_mip[1]);
  return EVENT_OK;
}

int mMpcExDataAna::mpcex_hit2_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  hmpcex_hit2[0] = new TH3D("hmpcex_hit2_arm0","mpcex hit arm 0",300,-40-0.5,260-0.5,100,0,1.5,8,-0.5,7.5);
  hmpcex_hit2[0]->GetXaxis()->SetTitle("low q");
  hmpcex_hit2[0]->GetYaxis()->SetTitle("ratio");
  hmpcex_hit2[0]->GetZaxis()->SetTitle("layer");
  hm->registerHisto(hmpcex_hit2[0]);

  hmpcex_hit2[1] = new TH3D("hmpcex_hit2_arm1","mpcex hit arm 1",300,-40-0.5,260-0.5,100,0,1.5,8,-0.5,7.5);
  hmpcex_hit2[1]->GetXaxis()->SetTitle("low q");
  hmpcex_hit2[1]->GetYaxis()->SetTitle("ratio");
  hmpcex_hit2[1]->GetZaxis()->SetTitle("layer");
  hm->registerHisto(hmpcex_hit2[1]);

  hmpcex_mip_bbc[0] = new TH3D("hmpcex_mip_bbc_arm0","number of mip vs bbc charge arm 0",100,-0.5,100-0.5,100,-0.5,100-0.5,8,-0.5,7.5);
  hmpcex_mip_bbc[0]->GetXaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc[0]->GetYaxis()->SetTitle("bbc charge");
  hmpcex_mip_bbc[0]->GetZaxis()->SetTitle("layer");
  hm->registerHisto(hmpcex_mip_bbc[0]);

  hmpcex_mip_bbc[1] = new TH3D("hmpcex_mip_bbc_arm1","number of mip vs bbc charge arm 1",100,-0.5,100-0.5,100,-0.5,100-0.5,8,-0.5,7.5);
  hmpcex_mip_bbc[1]->GetXaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc[1]->GetYaxis()->SetTitle("bbc charge");
  hmpcex_mip_bbc[1]->GetZaxis()->SetTitle("layer");
  hm->registerHisto(hmpcex_mip_bbc[1]);

  hmpcex_mip_bbc2[0] = new TH2D("hmpcex_mip_bbc2_arm0","number of mip vs bbc charge arm 0",100,-0.5,100-0.5,100,-0.5,100-0.5);
  hmpcex_mip_bbc2[0]->GetYaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc2[0]->GetXaxis()->SetTitle("bbc charge");
  hm->registerHisto(hmpcex_mip_bbc2[0]);

  hmpcex_mip_bbc2[1] = new TH2D("hmpcex_mip_bbc2_arm1","number of mip vs bbc charge arm 1",100,-0.5,100-0.5,100,-0.5,100-0.5);
  hmpcex_mip_bbc2[1]->GetYaxis()->SetTitle("number of MIP");
  hmpcex_mip_bbc2[1]->GetXaxis()->SetTitle("bbc charge");
  hm->registerHisto(hmpcex_mip_bbc2[1]);

  return EVENT_OK;
}

int mMpcExDataAna::mpcex_hit2_study(){
  MpcExHitMap* mpcex_hit_map = new MpcExHitMap(_mpcex_hit_container);
  
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    int n_mip2 = 0;
    double n_total_hits = 0;
    double n_neg_hits = 0;
    for(unsigned int layer = 0;layer < MpcExConstants::NLAYERS;layer++){
      MpcExHitMap::const_iterator iter = mpcex_hit_map->get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = mpcex_hit_map->get_layer_first(arm,layer+1);
      int n_mip = 0;
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
        int suround[4][2] = {{1,0},{-1,0},{0,1},{0,-1}}; 
	double low_adc = hit->low();
	n_total_hits++;
	if(low_adc < -10) n_neg_hits++;
	double hit_high_q = get_good_q(hit);
	if(hit_high_q < 0) continue;
	if(fabs(hit_high_q-19) < 6.4){
	  n_mip++;
	  n_mip2++;
	}
        bool is_peak = true;
	double total_q = hit_high_q;
	int nx = mpcex_hit_map->get_nx(hit);
	int ny = mpcex_hit_map->get_ny(hit);
	for(int i = 0;i < 4;i++){
	  //if nx or ny is out of range will return 0
          unsigned int index0 = mpcex_hit_map->get_index(arm,layer,nx+suround[i][0],ny+suround[i][1]);
	  //if index0 is invalid, hit0 will be Null;
	  TMpcExHit* hit0 = mpcex_hit_map->get_hit(index0);
	  if(hit0 == NULL) continue;
	  //if hit0 is Null, q will be -9999.9;
//	  double hit0_q = get_good_q(hit0);
	  double hit0_high_q = get_good_q(hit0);
	  if(hit0_high_q > 0) total_q += hit0_high_q;
	  if(hit0_high_q > hit_high_q){
  	    is_peak = false;
	  }
	}//i
        hmpcex_hit2[arm]->Fill(hit_high_q,hit_high_q/total_q,layer);
      }//iter 
      hmpcex_mip_bbc[arm]->Fill(n_mip,_bbc_charge[arm],layer);
    }//layer
    hmpcex_mip_bbc2[arm]->Fill(_bbc_charge[arm],n_mip2); 
  }//arm
  delete mpcex_hit_map;
  return EVENT_OK;
}

int mMpcExDataAna::lshower_track_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
   
  hlayer_e[0] = new TH3D("hlayer_e_0","layer e arm 0",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[0]->GetXaxis()->SetTitle("layer e");
  hlayer_e[0]->GetYaxis()->SetTitle("layer");
  hlayer_e[0]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[0]);

  hlayer_e[1] = new TH3D("hlayer_e_1","layer e arm 1",1000,0,0.1,8,-0.5,7.5,1000,0,1);
  hlayer_e[1]->GetXaxis()->SetTitle("layer e");
  hlayer_e[1]->GetYaxis()->SetTitle("layer");
  hlayer_e[1]->GetZaxis()->SetTitle("total mpcex e");
  hm->registerHisto(hlayer_e[1]);
 
  hlshower_bbc[0] = new TH2D("hlshower_bbc_arm0","lshower vs bbc charge arm0",200,-0.5,199.5,200,0,1000);
  hlshower_bbc[0]->GetXaxis()->SetTitle("number of showers");
  hlshower_bbc[0]->GetYaxis()->SetTitle("bbc charge");
  hm->registerHisto(hlshower_bbc[0]);
  
  hlshower_bbc[1] = new TH2D("hlshower_bbc_arm1","lshower vs bbc charge arm0",200,-0.5,199.5,200,0,1000);
  hlshower_bbc[1]->GetXaxis()->SetTitle("number of showers");
  hlshower_bbc[1]->GetYaxis()->SetTitle("bbc charge");
  hm->registerHisto(hlshower_bbc[1]);


  return EVENT_OK;
}

int mMpcExDataAna::lshower_track_study(){
  unsigned int NLshowers = _mpcex_lshower_container->size();
  int Nlshower[2] = {0,0};
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    int Ntracks = lshower->get_tracks_num();
    double mpcex_e = lshower->get_mpcex_e();
    if(mpcex_e > 0.006) Nlshower[arm]++;
    for(int itrack = 0;itrack < Ntracks;itrack++){
      TMpcExLTrack* ltrack = lshower->get_track(itrack);
      int Ntrack_hits = ltrack->get_hits_num();
      int Nfired_layers = ltrack->get_n_fired_layers();
      if(Nfired_layers == 8 && (Nfired_layers == Ntrack_hits)){
        int first_fired_layer = ltrack->get_first_fired_layer();
	double ltrack_e = ltrack->get_mpcex_e();
	for(int ilayer = first_fired_layer;ilayer < MpcExConstants::NLAYERS;ilayer++){
          double layer_e = ltrack->get_e_layer(ilayer);
          hlayer_e[arm]->Fill(layer_e,ilayer-first_fired_layer,ltrack_e);
	}
      }
    }
  }
  hlshower_bbc[0]->Fill(Nlshower[0],_bbc_charge[0]);
  hlshower_bbc[1]->Fill(Nlshower[1],_bbc_charge[1]);
  return EVENT_OK;
}

int mMpcExDataAna::bbc_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hcharge_vs_pmt[0] = new TH2D("hcharge_vs_pmt_arm0","BBC charge vs number of fired PMT South",50,-0.5,49.5,200,0,100);
  hcharge_vs_pmt[0]->GetXaxis()->SetTitle("N PMT");
  hcharge_vs_pmt[0]->GetYaxis()->SetTitle("BBC Charge");
  hm->registerHisto(hcharge_vs_pmt[0]);

  hcharge_vs_pmt[1] = new TH2D("hcharge_vs_pmt_arm1","BBC charge vs number of fired PMT North",50,-0.5,49.5,200,0,100);
  hcharge_vs_pmt[1]->GetXaxis()->SetTitle("N PMT");
  hcharge_vs_pmt[1]->GetYaxis()->SetTitle("BBC Charge");
  hm->registerHisto(hcharge_vs_pmt[1]);

  hNpmt = new TH2D("hNpmt","Number of PMT North vs South",50,-0.5,49.5,50,-0.5,49.5);
  hNpmt->GetXaxis()->SetTitle("South N PMT");
  hNpmt->GetYaxis()->SetTitle("North N PMT");
  hm->registerHisto(hNpmt);

  return EVENT_OK;
}

int mMpcExDataAna::bbc_study(PHCompositeNode* topNode){
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout){
    cout << PHWHERE <<"NO BBC !!!"<<endl;
    return EVENT_OK;
  }
  hcharge_vs_pmt[0]->Fill(bbcout->get_nPmt(Bbc::South),bbcout->get_ChargeSum(Bbc::South));
  hcharge_vs_pmt[1]->Fill(bbcout->get_nPmt(Bbc::North),bbcout->get_ChargeSum(Bbc::North));
  hNpmt->Fill(bbcout->get_nPmt(Bbc::South),bbcout->get_nPmt(Bbc::North)); 
  return EVENT_OK;
}

int mMpcExDataAna::lshower_mip_mpc_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hmip_mpc[0] = new TH3D("hmip_mpc_arm0","MIP Arm South",50,0,5,300,0,300,100,0,10);
  hmip_mpc[0]->GetXaxis()->SetTitle("distance/cm");
  hmip_mpc[0]->GetYaxis()->SetTitle("MPCEX average ADC");
  hmip_mpc[0]->GetZaxis()->SetTitle("MPC tower energy");
  hm->registerHisto(hmip_mpc[0]);

  hmip_mpc[1] = new TH3D("hmip_mpc_arm1","MIP Arm South",50,0,5,300,0,300,100,0,10);
  hmip_mpc[1]->GetXaxis()->SetTitle("distance/cm");
  hmip_mpc[1]->GetYaxis()->SetTitle("MPCEX average ADC");
  hmip_mpc[1]->GetZaxis()->SetTitle("MPC tower energy");
  hm->registerHisto(hmip_mpc[1]);

  hmip_tower[0] = new TH3D("hmip_tower_arm0","MIP on tower Arm 0",288,-0.5,287.5,300,0,300,100,0,10);
  hmip_tower[0]->GetXaxis()->SetTitle("tower channel");
  hmip_tower[0]->GetYaxis()->SetTitle("MIP ADC");
  hmip_tower[0]->GetZaxis()->SetTitle("tower e/GeV");
  hm->registerHisto(hmip_tower[0]);

  hmip_tower[1] = new TH3D("hmip_tower_arm1","MIP on tower Arm 1",288,-0.5,287.5,300,0,300,100,0,10);
  hmip_tower[1]->GetXaxis()->SetTitle("tower channel");
  hmip_tower[1]->GetYaxis()->SetTitle("MIP ADC");
  hmip_tower[1]->GetZaxis()->SetTitle("tower e/GeV");
  hm->registerHisto(hmip_tower[1]);
 

  return EVENT_OK;
}

int mMpcExDataAna::lshower_mip_mpc_study(){
  unsigned int NLshowers = _mpcex_lshower_container->size();
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    int Nhits = lshower->get_hits_num();
    double mpc_z = MpcExConstants::MPC_REFERENCE_Z;
    if(arm == 0) mpc_z = -mpc_z;
    int n_fired_layers = lshower->get_n_fired_layers();
    if((n_fired_layers > 4) && (n_fired_layers == Nhits) ){
      bool good_high = true;     
      double total_adc = 0;
      for(int ihit = 0;ihit < Nhits;ihit++){
        unsigned int hit_key = lshower->get_hit(ihit);
        TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
	double high_q = hit->high();
//	high_q = get_good_q(hit);
	double low_q = hit->low();
	total_adc += high_q;
	if(high_q <= 0 || _dead_map[hit->key()][0] != 0 || low_q < -10) {
	  good_high = false; 
//	  break;
	}
      }	
      int n_fired_tower = lshower->get_n_fired_towers5x5();
      double peak_tower_e = lshower->get_mpc_towers_e(2,2);
      int peak_tower_ch = lshower->get_mpc_towers_ch(2,2);
      if(good_high && (n_fired_tower == 1) && (peak_tower_e > 0) && (total_adc/n_fired_layers > 6)){
        double dx = lshower->get_closest_tower_dx();
	double dy = lshower->get_closest_tower_dy();
	double dr = sqrt(dx*dx+dy*dy);
	hmip_mpc[arm]->Fill(dr,total_adc/n_fired_layers,peak_tower_e);
	if(arm == 1) peak_tower_ch = peak_tower_ch - 288;
	hmip_tower[arm]->Fill(peak_tower_ch,total_adc/n_fired_layers,peak_tower_e);
      }
    }
  }
  return EVENT_OK;
}

int mMpcExDataAna::pair_channel_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
 
  hpair_channel[0] = new TH2D("hpair_channel_arm0","pair channels high ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel[0]->GetXaxis()->SetTitle("bigger key high ADC");
  hpair_channel[0]->GetYaxis()->SetTitle("high ADC");
  hm->registerHisto(hpair_channel[0]);

  hpair_channel[1] = new TH2D("hpair_channel_arm1","pair channels high ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel[1]->GetXaxis()->SetTitle("bigger key high ADC");
  hpair_channel[1]->GetYaxis()->SetTitle("high ADC");
  hm->registerHisto(hpair_channel[1]);

  hpair_channel2[0] = new TH2D("hpair_channel2_arm0","pair channels low ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel2[0]->GetXaxis()->SetTitle("bigger key low ADC");
  hpair_channel2[0]->GetYaxis()->SetTitle("low ADC");
  hm->registerHisto(hpair_channel2[0]);

  hpair_channel2[1] = new TH2D("hpair_channel2_arm1","pair channels low ADC",300,-40-0.5,260-0.5,300,-40-0.5,260-0.5);
  hpair_channel2[1]->GetXaxis()->SetTitle("bigger key low ADC");
  hpair_channel2[1]->GetYaxis()->SetTitle("low ADC");
  hm->registerHisto(hpair_channel2[1]);

  hbad_pair_channel[0] = new TH1D("hbad_pair_channel_arm0","bad pair channel key arm 0",26000,-0.5,26000-0.5);
  hbad_pair_channel[0]->GetXaxis()->SetTitle("bad pair channel key");
  hm->registerHisto(hbad_pair_channel[0]);

  hbad_pair_channel[1] = new TH1D("hbad_pair_channel_arm1","bad pair channel key arm 1",26000,24000-0.5,50000-0.5);
  hbad_pair_channel[1]->GetXaxis()->SetTitle("bad pair channel key");
  hm->registerHisto(hbad_pair_channel[1]);

  hodd_pair_channel_high[0] = new TH1D("hodd_pair_channel_high_arm0","Odd pair channel high key arm 0",26000,-0.5,26000-0.5);
  hodd_pair_channel_high[0]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hodd_pair_channel_high[0]);

  hodd_pair_channel_high[1] = new TH1D("hodd_pair_channel_high_arm1","Odd pair channel high key arm 1",26000,24000-0.5,50000-0.5);
  hodd_pair_channel_high[1]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hodd_pair_channel_high[1]);

  hodd_pair_channel_low[0] = new TH1D("hodd_pair_channel_low_arm0","Odd pair channel low key arm 0",26000,-0.5,26000-0.5);
  hodd_pair_channel_low[0]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hodd_pair_channel_low[0]);

  hodd_pair_channel_low[1] = new TH1D("hodd_pair_channel_low_arm1","Odd pair channel low key arm 1",26000,24000-0.5,50000-0.5);
  hodd_pair_channel_low[1]->GetXaxis()->SetTitle("key");
  hm->registerHisto(hodd_pair_channel_low[1]);

  return EVENT_OK;
}

int mMpcExDataAna::pair_channel_study(){
  TMpcExHitSet<>cal_hits(_mpcex_hit_container);
  TMpcExHitSet<>::const_iterator begin = cal_hits.get_iterator();
  TMpcExHitSet<>::const_iterator end = cal_hits.end();
  std::set<unsigned int> used_key;
  for(;begin != end;begin++){
    TMpcExHit* mpcex_hit = *begin;
    unsigned int key = mpcex_hit->key();
    int arm = mpcex_hit->arm();
    if(used_key.find(key)!=used_key.end())continue; 
    used_key.insert(key);
    int chain = mpcex_hit->chain();
    unsigned int pair_key = key;
    if(chain == 0 || chain == 1) pair_key = pair_key + 2*12*64;
    else pair_key = pair_key - 2*12*64;
    TMpcExHit* pair_hit = _mpcex_hit_container->get_hit_by_key(pair_key);
    if(pair_hit){
      if(used_key.find(pair_key)!=used_key.end())cout<<"strange happenned!!!"<<endl;
      used_key.insert(pair_key);
      double high_q = mpcex_hit->high();
      double low_q = mpcex_hit->low();
      double pair_high_q = pair_hit->high();
      double pair_low_q = pair_hit->low();
//      if(_dead_map[key][0]!=0 || _dead_map[key][1]!=0 || _dead_map[pair_key][0]!=0 || _dead_map[pair_key][1]!=0) continue;
      if(pair_key < key) hpair_channel[arm]->Fill(high_q,pair_high_q);
      else hpair_channel[arm]->Fill(pair_high_q,high_q);

      if(pair_key < key) hpair_channel2[arm]->Fill(low_q,pair_low_q);
      else hpair_channel2[arm]->Fill(pair_low_q,low_q);

      if(high_q < 2 && pair_high_q < 2 && high_q > -10 && pair_high_q > -10) {
        hodd_pair_channel_high[arm]->Fill(key);
	hodd_pair_channel_high[arm]->Fill(pair_key);
      }
      if(low_q < 2 && pair_low_q < 2 && low_q > -10 && pair_low_q > -10){ 
        hodd_pair_channel_low[arm]->Fill(key);
        hodd_pair_channel_low[arm]->Fill(pair_key);
      }
    }
    else hbad_pair_channel[arm]->Fill(pair_key);
  }
  return EVENT_OK;
}

int mMpcExDataAna::zero_surpress_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  hneg_adc_ratio[0] = new TH2D("hneg_adc_ratio_arm0","negtive low adc ratio South",100,-1.5,1.5,300,-40-0.5,260-0.5);
  hneg_adc_ratio[0]->GetXaxis()->SetTitle("ratio");
  hneg_adc_ratio[0]->GetYaxis()->SetTitle("high q");
  hm->registerHisto(hneg_adc_ratio[0]);

  hneg_adc_ratio[1] = new TH2D("hneg_adc_ratio_arm1","negtive low adc ratio North",100,-1.5,1.5,300,-40-0.5,260-0.5);
  hneg_adc_ratio[1]->GetXaxis()->SetTitle("ratio");
  hneg_adc_ratio[1]->GetYaxis()->SetTitle("high q");
  hm->registerHisto(hneg_adc_ratio[1]);

  hneg_total[0] = new TH2D("hneg_total_arm0","zero supressed vs total arm 0",200,0,2000,200,0,2000);
  hneg_total[0]->GetXaxis()->SetTitle("total hits");
  hneg_total[0]->GetYaxis()->SetTitle("zero supressed hits");
  hm->registerHisto(hneg_total[0]);

  hneg_total[1] = new TH2D("hneg_total_arm1","zero supressed vs total arm 1",200,0,2000,200,0,2000);
  hneg_total[1]->GetXaxis()->SetTitle("total hits");
  hneg_total[1]->GetYaxis()->SetTitle("zero supressed hits");
  hm->registerHisto(hneg_total[1]);

  hratio_mpc[0] = new TH2D("hratio_mpc_arm0","ratio vs Mpc Energy South",200,-0.5,1.5,200,0,100);
  hratio_mpc[0]->GetXaxis()->SetTitle("ratio");
  hratio_mpc[0]->GetYaxis()->SetTitle("Mpc Energy/GeV");
  hm->registerHisto(hratio_mpc[0]);

  hratio_mpc[1] = new TH2D("hratio_mpc_arm1","ratio vs Mpc Energy North",200,-0.5,1.5,200,0,100);
  hratio_mpc[1]->GetXaxis()->SetTitle("ratio");
  hratio_mpc[1]->GetYaxis()->SetTitle("Mpc Energy/GeV");
  hm->registerHisto(hratio_mpc[1]);


/*
  hratio_evt[0] = new TH1D("ratio_evt_arm0","ratio vs event South",65000,-0.5,65000-0.5);
  hratio_evt[0]->GetXaxis()->SetTitle("event");
  hm->registerHisto(hratio_evt[0]);

  hratio_evt[1] = new TH1D("ratio_evt_arm1","ratio vs event North",65000,-0.5,65000-0.5);
  hratio_evt[1]->GetXaxis()->SetTitle("event");
  hm->registerHisto(hratio_evt[1]);
*/

/*
  hneg_total_adc_high[0] = new TH3D("hneg_total_adc_high_arm0","Zero Supressed hits vs total hits",200,0,2000,200,0,2000,300,-40-0.5,260-0.5);
  hneg_total_adc_high[0]->GetXaxis()->SetTitle("total hits");
  hneg_total_adc_high[0]->GetYaxis()->SetTitle("low q zero supressed hits");
  hneg_total_adc_high[0]->GetZaxis()->SetTitle("high ADC");
  hm->registerHisto(hneg_total_adc_high[0]);

  hneg_total_adc_high[1] = new TH3D("hneg_total_adc_high_arm1","Zero Supressed hits vs total hits",200,0,2000,200,0,2000,300,-40-0.5,260-0.5);
  hneg_total_adc_high[1]->GetXaxis()->SetTitle("total hits");
  hneg_total_adc_high[1]->GetYaxis()->SetTitle("low q zero supressed hits");
  hneg_total_adc_high[1]->GetZaxis()->SetTitle("high ADC");
  hm->registerHisto(hneg_total_adc_high[1]);
*/

  return EVENT_OK;
}

int mMpcExDataAna::zero_surpress_study(){
  int NMpcTowers = _mpc_tower_container->size();
  double sum_e[2] = {0.,0.};
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0) continue;
    sum_e[arm] += e_tower;
  } 
  
  int Nhits = _mpcex_hit_container->size();
  double Ntotal_hits[2] ={0,0};
  double Nnegtv_hits[2] ={0,0};
  for(int i = 0;i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    double high_q = hit->high();
    if(high_q < 3) continue;
    int arm = hit->arm();
    Ntotal_hits[arm]++;
    double low_q = hit->low();
    if(low_q < -10)Nnegtv_hits[arm]++;
  }
  
  if(Ntotal_hits[0] > 1)hratio_mpc[0]->Fill(Nnegtv_hits[0]/Ntotal_hits[0],sum_e[0]);
  if(Ntotal_hits[1] > 1)hratio_mpc[1]->Fill(Nnegtv_hits[1]/Ntotal_hits[1],sum_e[1]);
    
//  cout << Ntotal_hits[0] <<" "<<Ntotal_hits[1]<<endl;
//  cout << Nnegtv_hits[0] <<" "<<Nnegtv_hits[1]<<endl;
  hneg_total[0]->Fill(Ntotal_hits[0],Nnegtv_hits[0]);
  hneg_total[1]->Fill(Ntotal_hits[1],Nnegtv_hits[1]);
 
  for(int i = 0;i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    int arm = hit->arm();
    double high_q = hit->high();
    if(high_q < 3) continue;
    hneg_adc_ratio[arm]->Fill(Nnegtv_hits[arm]/Ntotal_hits[arm],high_q);
//    hneg_total_adc_high[arm]->Fill(Ntotal_hits[arm],Nnegtv_hits[arm],high_q);
  }
  return EVENT_OK;
}

int mMpcExDataAna::sim_mpc_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
  
  hsim_mpc_cluster[0] = new TH3D("hsim_mpc_cluster_arm0","Mpc Cluster arm0",100,-24,24,100,-24,24,100,0,100);
  hsim_mpc_cluster[0]->GetXaxis()->SetTitle("X/cm");
  hsim_mpc_cluster[0]->GetYaxis()->SetTitle("Y/cm");
  hsim_mpc_cluster[0]->GetZaxis()->SetTitle("energy/GeV");
  hm->registerHisto(hsim_mpc_cluster[0]);

  hsim_mpc_cluster[1] = new TH3D("hsim_mpc_cluster_arm1","Mpc Cluster arm1",100,-24,24,100,-24,24,100,0,100);
  hsim_mpc_cluster[1]->GetXaxis()->SetTitle("X/cm");
  hsim_mpc_cluster[1]->GetYaxis()->SetTitle("Y/cm");
  hsim_mpc_cluster[1]->GetZaxis()->SetTitle("energy/GeV");
  hm->registerHisto(hsim_mpc_cluster[1]);
 
  hsim_mpc_tower[0] = new TH3D("hsim_mpc_tower_arm0","Mpc Tower",20,-0.5,19.5,20,-0.5,19.5,100,0,100);
  hsim_mpc_tower[0]->GetXaxis()->SetTitle("Grid X");
  hsim_mpc_tower[0]->GetYaxis()->SetTitle("Grid Y");
  hsim_mpc_tower[0]->GetZaxis()->SetTitle("energy/GeV");
  hm->registerHisto(hsim_mpc_tower[0]);

  hsim_mpc_tower[1] = new TH3D("hsim_mpc_tower_arm1","Mpc Tower",20,-0.5,19.5,20,-0.5,19.5,100,0,100);
  hsim_mpc_tower[1]->GetXaxis()->SetTitle("Grid X");
  hsim_mpc_tower[1]->GetYaxis()->SetTitle("Grid Y");
  hsim_mpc_tower[1]->GetZaxis()->SetTitle("energy/GeV");
  hm->registerHisto(hsim_mpc_tower[1]);

  hprim_photon[0] = new TH2D("hprim_photon_arm0","Photon Energy South",50,0,5,200,0,100);
  hprim_photon[0]->GetYaxis()->SetTitle("Energy/GeV");
  hprim_photon[0]->GetXaxis()->SetTitle("Eta");
  hm->registerHisto(hprim_photon[0]);

  hprim_photon[1] = new TH2D("hprim_photon_arm1","Photon Energy North",50,0,5,200,0,100);
  hprim_photon[1]->GetYaxis()->SetTitle("Energy/GeV");
  hprim_photon[1]->GetXaxis()->SetTitle("Eta");
  hm->registerHisto(hprim_photon[1]);

  hphoton_theta = new TH1D("hphoton_theta","prim photon theta",200,-3.2*6,3.2*6);
  hphoton_theta->GetXaxis()->SetTitle("theta");
  hm->registerHisto(hphoton_theta);

  return EVENT_OK;
}

int mMpcExDataAna::sim_mpc_study(PHCompositeNode* topNode){
  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0) continue;
    int gridx = _mpc_map->getGridX(tow_ch);
    int gridy = _mpc_map->getGridY(tow_ch);
    hsim_mpc_tower[arm]->Fill(gridx,gridy,e_tower);
  } 

  int NMpcClusters = _mpc_cluster_container->size();
  for(int icluster = 0;icluster < NMpcClusters;icluster++){
    mpcClusterContent* cluster = _mpc_cluster_container->getCluster(icluster);
    int arm = cluster->arm();
    float x = cluster->x();
    float y = cluster->y();
    float e = cluster->e();
    hsim_mpc_cluster[arm]->Fill(x,y,e);
  }
  
  fkinWrapper* fkin = findNode::getClass<fkinWrapper>(topNode, "fkin");
  size_t fkinrows = fkin->RowCount();
  for(size_t ifkin=0; ifkin<fkinrows; ifkin++){
    int gen_track = 0;
    int fkin_true_track = fkin->get_true_track(ifkin);
    if (fkin->get_idpart(ifkin)!= 1) continue; //photon
    //primary particle has a parent track number==0
    if(fkin->get_itparent(ifkin)==0){
      float theta = fkin->get_pthet(ifkin);
      float ptot = fkin->get_ptot(ifkin);
      hphoton_theta->Fill(theta);
      int arm = 0;
      if(theta < 3.1415926/2.) arm = 1;
      float eta = -log(tan(theta/2.));
      hprim_photon[arm]->Fill(fabs(eta),ptot);
    }
  }
  

  return EVENT_OK;
}

void mMpcExDataAna::lshower_evt_display(){
  for(int i = 0;i < 2; i++){
    if(grammy2[i]) delete grammy2[i];
    char name[50];
    sprintf(name,"Arm_%d_for_combined_q",i);
    grammy2[i] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
  }

  for(int i = 0;i < 2;i++){
    if(hmpc_gridxy2[i]) delete hmpc_gridxy2[i];
    char name[100];
    sprintf(name,"hmpc_gridxy2_arm%d",i);
    hmpc_gridxy2[i] =  new TH2F(name,name,600,-24,24,600,-24,24);
  }

  bool find_it = false;
  unsigned int NLshowers = _mpcex_lshower_container->size();
  std::set<int> used_tower;
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    int Nhits = lshower->get_hits_num();
    int nlayers = lshower->get_n_fired_layers();
    float hsx = lshower->get_hsx();
    float hsy = lshower->get_hsy();
    float hsr = sqrt(hsx*hsx+hsy*hsy);
    float theta = atan(hsr);
    float eta = -log(tan(theta/2.));
    if(arm == 0 && (fabs(eta) < 3.2) || (fabs(eta) > 3.7)) continue;
    if(arm == 1 && (fabs(eta) < 3.2) || (fabs(eta) > 4.0)) continue;
    if(nlayers < 4) continue;
    float mpcE3x3 = lshower->get_mpc_e3x3();
    float total_e = mpcE3x3;
    for(int ilayer = 0;ilayer < 8;ilayer++){
      float layer_e = lshower->get_e_layer(ilayer);
      if(layer_e > 0)total_e += layer_e;
    }
    if(total_e < 4) continue;
    int ctw_ch = lshower->get_mpc_towers_ch(2,2);
    float ctw_e = lshower->get_mpc_towers_e(2,2);
    if(ctw_e > 0 || ctw_ch > 0) continue;
    cout <<"find the wanted shower "<< endl;
    find_it = true;

    for(int ihit = 0;ihit < Nhits;ihit++){
      unsigned int hit_key = lshower->get_hit(ihit);
      TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
      double hit_q = hit->combined();
      grammy2[arm]->FillEx(hit_key,hit_q);
    }

    for(int ix = 0;ix < 5;ix++){
      for(int iy = 0;iy < 5;iy++){
        int tow_ch = lshower->get_mpc_towers_ch(ix,iy);
	double e_tower = lshower->get_mpc_towers_e(ix,iy);
	if(tow_ch < 0 || e_tower < 0) continue;
	if(used_tower.find(tow_ch) != used_tower.end()) continue;
	used_tower.insert(tow_ch);
	double x = _mpc_map->getX(tow_ch); 
        double y = _mpc_map->getY(tow_ch);
        int binx0 = hmpc_gridxy2[arm]->GetXaxis()->FindBin(x-0.9);
        int binx1 = hmpc_gridxy2[arm]->GetXaxis()->FindBin(x+0.9);
        int biny0 = hmpc_gridxy2[arm]->GetYaxis()->FindBin(y-0.9);
        int biny1 = hmpc_gridxy2[arm]->GetYaxis()->FindBin(y+0.9);
        for(int ib = binx0;ib <=binx1;ib++){
          for(int jb = biny0;jb <=biny1;jb++){
	    hmpc_gridxy2[arm]->SetBinContent(ib,jb,e_tower);
          }//j
        }//i   
      }//iy
    }//ix

  }//i shower
  if(find_it){
    TCanvas* cc_mpc0 = new TCanvas("cc_mpc0","cc_mpc0",1500,800);
    cc_mpc0->Divide(2,1);
    cc_mpc0->cd(1);
    hmpc_gridxy2[0]->DrawCopy("colz");
    cc_mpc0->cd(2);
    grammy2[0]->Project3D("yx")->DrawCopy("colz");
    TCanvas* cc_mpc1 = new TCanvas("cc_mpc1","cc_mpc1",1500,800);
    cc_mpc1->Divide(2,1);
    cc_mpc1->cd(1);
    hmpc_gridxy2[1]->DrawCopy("colz");
    cc_mpc1->cd(2);
    grammy2[1]->Project3D("yx")->DrawCopy("colz");
    event_display();
  }
}

int mMpcExDataAna::primary_particle(PHCompositeNode* topNode){
  fkinWrapper* fkin = findNode::getClass<fkinWrapper>(topNode, "fkin");
  primaryWrapper* primary = findNode::getClass<primaryWrapper>(topNode, "primary");
  if(!fkin || !primary ){
    cout <<"No fkinWrapper or primaryWrapper !!!"<<endl;
    return EVENT_OK;
  }
  size_t primrows = primary->RowCount();
  size_t fkinrows = fkin->RowCount();


  for(size_t ifkin=0; ifkin<fkinrows; ifkin++){
    int gen_track = 0;
    int fkin_true_track = fkin->get_true_track(ifkin);
    int idpart = fkin->get_idpart(ifkin);
    int idparent = fkin->get_idparent(ifkin);
    //to see if it comes from previous particle
    int itparent = fkin->get_itparent(ifkin);
    if(idparent !=8 && idpart != 8) continue;
    cout <<"Truck Number: "<<fkin_true_track<<" "
         <<"Particle ID: "<<idpart<<" "
	 <<"Parent ID: "<<idparent<<" "
	 <<"it Parent: "<<itparent<<" "
	 <<"Energy: "<<fkin->get_ptot(ifkin)<<" "
	 <<"Z vertex: "<<fkin->get_z_vertex(ifkin)<<" "
	 <<"SubEvent "<<fkin->get_subevent(ifkin)<<" "
	 <<"true track "<<fkin_true_track<<" "
	 <<endl;
     //parimary part
     for(size_t i=0; i<primrows; i++){
       if(primary->get_true_track(i)==fkin_true_track){
       //find particle line # in the generator phhijing/phpythia
         gen_track=primary->get_event_track(i);
         float px=primary->get_px_momentum(i);
         float py=primary->get_py_momentum(i);
         float pz=primary->get_pz_momentum(i);
         float tot=sqrt(px*px+py*py+pz*pz);
         cout <<"primary particle " << i << " id " << primary->get_idpart(i)
            <<" momentum x, y, z "<< px<<" "<<py<<" "<<pz<< " "<< "total " << tot 
	    <<" gen_track: "<<gen_track
	    <<" sub_event: "<<primary->get_subevent(i)
	    <<" sub_track: "<<primary->get_subevent_track(i)
	    <<endl;
       }
     }    
  }

/*
     for(size_t i=0; i<primrows; i++){
//       if(primary->get_true_track(i)==fkin_true_track){
       //find particle line # in the generator phhijing/phpythia
         int gen_track=primary->get_event_track(i);
         float px=primary->get_px_momentum(i);
         float py=primary->get_py_momentum(i);
         float pz=primary->get_pz_momentum(i);
         float tot=sqrt(px*px+py*py+pz*pz);
         cout <<"primary particle " << i << " id " << primary->get_idpart(i)
            <<" momentum x, y, z "<< px<<" "<<py<<" "<<pz<< " "<< "total " << tot 
	    <<" gen_track: "<<gen_track
	    <<" sub_event: "<<primary->get_subevent(i)
	    <<" sub_track: "<<primary->get_subevent_track(i)
	    <<endl;
//       }
     }    
*/
  return EVENT_OK;
}

int mMpcExDataAna::mpc_cluster_study(){
  cout<<"process mMpcExDataAna::mpc_cluster_study() "<<endl;
  for(int i = 0;i < 2;i++){
    if(hmpc_gridxy2[i]) delete hmpc_gridxy2[i];
    char name[100];
    sprintf(name,"hmpc_gridxy2_arm%d",i);
    hmpc_gridxy2[i] =  new TH2F(name,name,600,-24,24,600,-24,24);
  }

  cout<<"1"<<endl;
  int NMpcClusters = _mpc_cluster_container->size();
  double max_e[2] = {0,0};
  mpcClusterContent* max_e_cluster[2] = {NULL,NULL};
  for(int icluster = 0;icluster < NMpcClusters;icluster++){
    mpcClusterContent* cluster = _mpc_cluster_container->getCluster(icluster);
    int arm = cluster->arm();
//    double e = cluster->e();
    int ntowers = cluster->multiplicity();
    if(ntowers > max_e[arm]){
      max_e[arm] = ntowers;
      max_e_cluster[arm] = cluster;
    }
  }
  cout<<"2"<<endl;  
  double clus_x[2] = {0,0};
  double clus_y[2] = {0,0};
  for(int iarm = 0;iarm < 2;iarm++){
    if(!max_e_cluster[iarm]) continue;
    int NTowers = max_e_cluster[iarm]->multiplicity();
    clus_x[iarm] = max_e_cluster[iarm]->x();
    clus_y[iarm] = max_e_cluster[iarm]->y();
    cout<<"arm  "<<iarm<<" x "<<clus_x[iarm]<<" y "<<clus_y[iarm]<<endl;
    for(int itower = 0;itower < NTowers;itower++){
      int tower_ch = max_e_cluster[iarm]->towerid(itower);
      cout<<"tower channel "<<tower_ch<<endl;
      double x = _mpc_map->getX(tower_ch); 
      double y = _mpc_map->getY(tower_ch);
      int tower_index = _mpc_tower_container->findTower(tower_ch);
      cout<<"tower index "<<tower_index<<endl;
      mpcTowerContent* ctwer = _mpc_tower_container->getTower(tower_index);
      double tower_e = ctwer->get_energy();  
      if(tower_e < 0) continue;
      int binx0 = hmpc_gridxy2[iarm]->GetXaxis()->FindBin(x-0.9);
      int binx1 = hmpc_gridxy2[iarm]->GetXaxis()->FindBin(x+0.9);
      int biny0 = hmpc_gridxy2[iarm]->GetYaxis()->FindBin(y-0.9);
      int biny1 = hmpc_gridxy2[iarm]->GetYaxis()->FindBin(y+0.9);
      for(int ib = binx0;ib <=binx1;ib++){
        for(int jb = biny0;jb <=biny1;jb++){
	  hmpc_gridxy2[iarm]->SetBinContent(ib,jb,tower_e);
        }//j
      }//i   
    }
  }
  cout<<"3"<<endl;
  TCanvas* cc_mpc0 = new TCanvas("cc_mpc0","cc_mpc0",800,800);
  cc_mpc0->cd();
  hmpc_gridxy2[0]->DrawCopy("colz");
  TBox tbox(clus_x[0]-0.3,clus_y[0]-0.3,clus_x[0]+0.3,clus_y[0]+0.3);
  cout << clus_x[0] <<" "<<clus_y[0]<<endl;
  tbox.SetLineColor(kBlack);
  tbox.SetFillColor(kBlack);
//  tbox.SetFillStyle(3003);
  tbox.DrawClone("same");

  TCanvas* cc_mpc1 = new TCanvas("cc_mpc1","cc_mpc1",800,800);
  cc_mpc1->cd();
  hmpc_gridxy2[1]->DrawCopy("colz");
  TBox tbox1(clus_x[1]-0.3,clus_y[1]-0.3,clus_x[1]+0.3,clus_y[1]+0.3);
  tbox1.SetLineColor(kBlack);
  tbox1.SetFillColor(kBlack);
//  tbox1.SetFillStyle(3003);
  tbox1.DrawClone("same");

  return EVENT_OK;
}

int mMpcExDataAna::HW_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  
  hMpc_tot_e[0] = new TH1D("hMpc_tot_e_arm0","Total Energy of MPC Arm 0",500,0,250); 
  hMpc_tot_e[0]->GetXaxis()->SetTitle("Total E/GeV");
  hm->registerHisto(hMpc_tot_e[0]);
  
  hMpc_tot_e[1] = new TH1D("hMpc_tot_e_arm1","Total Energy of MPC Arm 1",500,0,250); 
  hMpc_tot_e[1]->GetXaxis()->SetTitle("Total E/GeV");
  hm->registerHisto(hMpc_tot_e[1]);

  hMpc_twr_e[0] = new TH1D("hMpc_twr_e_arm0","Mpc Mean Tower E Arm 0",500,0,10);
  hMpc_twr_e[0]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpc_twr_e[0]);

  hMpc_twr_e[1] = new TH1D("hMpc_twr_e_arm1","Mpc Mean Tower E Arm 1",500,0,10);
  hMpc_twr_e[1]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpc_twr_e[1]);

  hMpcEx_tot_e[0] = new TH1D("hMpcEx_tot_e_arm0","MpcEx total E Arm 0",1000,0,1);  
  hMpcEx_tot_e[0]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpcEx_tot_e[0]);

  hMpcEx_tot_e[1] = new TH1D("hMpcEx_tot_e_arm1","MpcEx total E Arm 1",1000,0,1);  
  hMpcEx_tot_e[1]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpcEx_tot_e[1]);

  hMpcEx_mpad_e[0] = new TH1D("hMpcEx_mpad_e_arm0","MpcEx Minipad Mean E Arm 0",1000,0,0.02);
  hMpcEx_mpad_e[0]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpcEx_mpad_e[0]);

  hMpcEx_mpad_e[1] = new TH1D("hMpcEx_mpad_e_arm1","MpcEx Minipad Mean E Arm 1",1000,0,0.02);
  hMpcEx_mpad_e[1]->GetXaxis()->SetTitle("E/GeV");
  hm->registerHisto(hMpcEx_mpad_e[1]);

  return EVENT_OK;
}

int mMpcExDataAna::HW_study(){
  int Nhits = _mpcex_hit_container->size();
  int Arm_Nhits[2] = {0,0};
  double Arm_e[2] = {0,0};
  for(int i = 0;i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    if(hit->state_combined()== TMpcExHit::INVALID) continue;
    double hit_q = hit->combined();
    int arm = hit->arm();
    Arm_e[arm] += hit_q;
    Arm_Nhits[arm]++;
  }

  int Ntowers = _mpc_tower_container->size();
  int Arm_Ntowers[2] = {0,0};
  double Arm_tower_e[2] = {0,0};
  for(int i = 0;i < Ntowers;i++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(i);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0) continue;
    Arm_Ntowers[arm]++;
    Arm_tower_e[arm] += e_tower; 
  }
  for(int iarm = 0;iarm < 2;iarm++){
    if(Arm_Nhits[iarm] > 0){
      hMpcEx_tot_e[iarm]->Fill(Arm_e[iarm]/(1.0e6));
      hMpcEx_mpad_e[iarm]->Fill(Arm_e[iarm]/Arm_Nhits[iarm]/(1.0e6));
    }
    if(Arm_Ntowers[iarm] > 0){
      hMpc_tot_e[iarm]->Fill(Arm_tower_e[iarm]);
      hMpc_twr_e[iarm]->Fill(Arm_tower_e[iarm]/Arm_Ntowers[iarm]);
    }
  }
  return EVENT_OK;
}

int mMpcExDataAna::run16_mpcex_hit_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }
 
  hkey_adc_high = new TH2D("hkey_adc_high","MpcEx high ADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_adc_high->GetXaxis()->SetTitle("Key");
  hkey_adc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_adc_high);

  hkey_adc_low = new TH2D("hkey_adc_low","MpcEx low ADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_adc_high->GetXaxis()->SetTitle("Key");
  hkey_adc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_adc_low);

  hbbc_Nhits[0] = new TH2D("hbbc_Nhits_arm0","Number of hits vs BBC Arm 0",400,0,2000,500,0,2000);
  hbbc_Nhits[0]->GetXaxis()->SetTitle("BBC charge");
  hbbc_Nhits[0]->GetYaxis()->SetTitle("Number of hits");
  hm->registerHisto(hbbc_Nhits[0]);

  hbbc_Nhits[1] = new TH2D("hbbc_Nhits_arm1","Number of hits vs BBC Arm 1",400,0,2000,500,0,2000);
  hbbc_Nhits[1]->GetXaxis()->SetTitle("BBC charge");
  hbbc_Nhits[1]->GetYaxis()->SetTitle("Number of hits");
  hm->registerHisto(hbbc_Nhits[1]);


  hbbc_adc[0] = new TH2D("hbbc_adc_arm0","Mean ADC vs BBC charge Arm 0",400,0,2000,300,0,3000);
  hbbc_adc[0]->GetXaxis()->SetTitle("BBC charge");
  hbbc_adc[0]->GetYaxis()->SetTitle("Mean ADC value");
  hm->registerHisto(hbbc_adc[0]);

  hbbc_adc[1] = new TH2D("hbbc_adc_arm1","Mean ADC vs BBC charge Arm 1",400,0,2000,300,0,3000);
  hbbc_adc[1]->GetXaxis()->SetTitle("BBC charge");
  hbbc_adc[1]->GetYaxis()->SetTitle("Mean ADC value");
  hm->registerHisto(hbbc_adc[1]);


  hgrammy[0] = new Exogram("hgrammy0","Exogram arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy[0]);

  hgrammy[1] = new Exogram("hgrammy1","Exogram arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy[1]);


   return EVENT_OK;
}

int mMpcExDataAna::run16_mpcex_hit_study(){
  TMpcExHitSet<>raw_hits(_mpcex_raw_hits);
  TMpcExHitSet<>::const_iterator begin = raw_hits.get_iterator();
  TMpcExHitSet<>::const_iterator end = raw_hits.end();

  begin = raw_hits.get_iterator();
  end = raw_hits.end();
  while(begin != end){
    TMpcExHit* mpcex_raw_hit = *begin;
    float high_adc = mpcex_raw_hit->high();
    float low_adc = mpcex_raw_hit->low();
    int key = mpcex_raw_hit->key();
    hkey_adc_high->Fill(key,high_adc);
    hkey_adc_low->Fill(key,low_adc);
    begin++;
  }

  int Nhits = _mpcex_hit_container->size();
  int Arm_Nhits[2] = {0,0};
  double Arm_totAdc[2] = {0,0};
  for(int i = 0; i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    int arm = hit->arm();
    double good_q = -9999.9;
    if(hit->isGoodHighHit()) good_q = hit->high();
//    if(good_q > 230 && hit->isGoodLowHit()) good_q = hit->low()*4.5;
    int key = hit->key();
    if(good_q < 0 )continue;
//    cout <<"good_q: "<<good_q<<endl;
    hgrammy[arm]->FillEx(key,good_q);
    Arm_Nhits[arm]++;
    Arm_totAdc[arm] += good_q;
  }

  for(int iarm = 0;iarm < 2;iarm++){
    hbbc_adc[iarm]->Fill(_bbc_charge[iarm],Arm_totAdc[iarm]);
    hbbc_Nhits[iarm]->Fill(_bbc_charge[iarm],Arm_Nhits[iarm]);
  }

  return EVENT_OK;
}

//simply calibrated when the calibration is not available
int mMpcExDataAna::run16_MpcExHit_calib(){
  int Nhits = _mpcex_hit_container->size();
  double Layer_MIP[2][8] = {{19,21,15.8,16.8,17.7,17.6,17.6,16.9},
                            {20,22,16.5,17.2,17.1,18.3,18.1,17.1}};
  TRandom3* r3 = new TRandom3(0);
  bool calib_arm[2] = {true,true};
  for(int iarm = 0;iarm < 2;iarm++){
//    if(_bbc_charge[iarm] > 50 || _bbc_charge[iarm] < 5) calib_arm[iarm] = false;
  }
//  if(Nhits > 2000) return EVENT_OK;
  for(int i = 0; i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    int arm = hit->arm();
//    if(!calib_arm[arm]) continue;
    int layer = hit->layer();
    float MIP_sensor = Layer_MIP[arm][layer];
    float HL_ratio = 4.5;
    if((hit->state_low()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_low()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)){
      hit->set_low((hit->low()+r3->Rndm())*(MpcExConstants::MIP_IN_keV/MIP_sensor)*HL_ratio);
      hit->set_state_low(TMpcExHit::GAIN_CALIBRATED);
    }
    
    if((hit->state_high()==TMpcExHit::PEDESTAL_SUBTRACTED)||(hit->state_high()==TMpcExHit::PEDESTAL_AND_CMN_SUBTRACTED)){
      hit->set_high((hit->high()+r3->Rndm())*(MpcExConstants::MIP_IN_keV/MIP_sensor));
      hit->set_state_high(TMpcExHit::GAIN_CALIBRATED);
    }
    double EMAX_HIGH = ((250.0-20)/MIP_sensor)*MpcExConstants::MIP_IN_keV;
    if( hit->isGoodGCLowHit() && hit->isGoodGCHighHit() ){          
      if(hit->high() < EMAX_HIGH) hit->set_combined(hit->high());
      else hit->set_combined(hit->low());
      hit->set_state_combined(TMpcExHit::VALID); 
    }
    else if( hit->isGoodGCLowHit() ){
      hit->set_combined(hit->low());            
      hit->set_state_combined(TMpcExHit::VALID_LOW_ONLY); 
    }
    else if( hit->isGoodGCHighHit() ){
      hit->set_combined(hit->high());           
      hit->set_state_combined(TMpcExHit::VALID_HIGH_ONLY); 
    }
    else{
      hit->set_combined(0.0);     
      hit->set_state_combined(TMpcExHit::INVALID); 
    }
  }
  delete r3;
  return EVENT_OK;
}
