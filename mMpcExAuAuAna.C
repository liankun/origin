#include "mMpcExAuAuAna.h"
#include <MpcExConstants.h>
#include <TMpcExHitContainer.h>
#include <TMpcExHit.h>
#include <TMpcExHitSet.h>
#include <TMpcExShowerContainer.h>
#include <TMpcExShower.h>
#include <Exogram.h>
#include "TMpcExLShower.h"
#include "TMpcExLShowerContainer.h"
#include <TMpcExCalibContainer.h>
#include <TMpcExCalib.h>
#include <MpcExRawHit.h>

#include "PHIODataNode.h"
#include "getClass.h"
#include "BbcOut.h"
#include "PHGlobal.h"
#include "Bbc.hh"
#include "TriggerHelper.h"

#include <MpcMap.h>
#include <MpcCalib.h>
#include <mpcSampleContainer.h>
#include <mpcClusterContent.h>
#include <mpcClusterContainer.h>
#include <mpcClusterContentV1.h>
#include <mpcTowerContainer.h>
#include <mpcTowerContent.h>
#include <mpcTowerContentV1.h> 
#include <mpcRawContainer.h>
#include <mpcRawContent.h>

#include <primary.h>
#include <primaryWrapper.h>
#include <fkinWrapper.h>

#include <Fun4AllReturnCodes.h>
#include <Fun4AllServer.h>
#include <Fun4AllHistoManager.h>
#include <recoConsts.h>

#include <iostream>
#include <string>
#include <vector>

#include <TH2D.h>
#include <TCanvas.h>

using namespace std;
using namespace findNode;

mMpcExAuAuAna::mMpcExAuAuAna(const char* name) :
  SubsysReco(name)
{
  _vertex = -9999.0;
  _mpcex_hit_container = NULL;
  _mpcex_shower_container = NULL;
  _mpcex_lshower_container = NULL;
  _mpc_cluster_container = NULL;
  _mpc_tower_container = NULL;
  _mpc_map = NULL;
  used = true;

  for(int iarm = 0;iarm < 2;iarm++){
    grammyh[iarm] = NULL;
    grammyl[iarm] = NULL;
    hmpc_gridxy[iarm] = NULL;
  }

}

int mMpcExAuAuAna::End(PHCompositeNode* topNode){
  return EVENT_OK;
}

mMpcExAuAuAna::~mMpcExAuAuAna(){

}

int mMpcExAuAuAna::Init(PHCompositeNode* topNode){
  general_data_check_init();
//  lshower_init();
  return EVENT_OK;
}

int mMpcExAuAuAna::InitRun(PHCompositeNode* topNode){
  set_interface_ptrs(topNode);
  return EVENT_OK;
}

void mMpcExAuAuAna::set_interface_ptrs(PHCompositeNode* topNode){
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

  _mpcex_calib_container = getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(!_mpcex_calib_container){
    cout << PHWHERE <<":: No TMpcExCalibContainer !!!"<<endl;
//    exit(1);  
  }

  _mpcex_shower_container = getClass<TMpcExShowerContainer>(topNode,"TMpcExShowerContainer");
  if(!_mpcex_shower_container){
    cout << PHWHERE <<":: No TMpcExShowerContainer !!!"<<endl;
//    exit(1);
  }
  
  _mpcex_lshower_container = getClass<TMpcExLShowerContainer>(topNode,"TMpcExLShowerContainer");
  if(!_mpcex_lshower_container){
    cout << PHWHERE <<":: No TMpcExLShowerContainer !!!"<<endl;
//    exit(1);
  }

  _mpc_cluster_container = getClass<mpcClusterContainer>(topNode,"mpcClusterContainer");
  if(!_mpc_cluster_container){
    cout <<PHWHERE <<":: No mpcClusterContainer!!!"<<endl;
//   exit(1);
  }

  _mpc_tower_container = getClass<mpcTowerContainer>(topNode,"mpcTowerContainer");
  if(!_mpc_tower_container){
    cout << PHWHERE <<":: No mpcTowerContainer!!!"<<endl;
//    exit(1);
  }

}

int mMpcExAuAuAna::process_event(PHCompositeNode* topNode){
//  TriggerHelper* myTH = new TriggerHelper(topNode);
//  int fire_minbias = myTH->trigScaled("BBCLL1(>1 tubes)");
//  int fire_ultra = myTH->trigScaled("UltraPeriphMPC");
//  if(!fire_minbias) return EVENT_OK;
//  if(!fire_ultra) return EVENT_OK;
  PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout && !phglobal){
    cout <<"No BbcOut or PHGlobal !!!"<<endl;
    exit(1);
  }
  _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();

  _bbc_charge[1] = bbcout->get_ChargeSum(Bbc::North);
  _bbc_charge[0] = bbcout->get_ChargeSum(Bbc::South);

  //check the shower works or not
//  int Nshowers = _mpcex_shower_container->size();
//  cout <<"Nshowers: "<< Nshowers<<endl;
//  int Nshowers = _mpcex_lshower_container->size();
//  cout << "NLshowers: "<<Nshowers<<endl;

//  event_display();
  
  general_data_check_study();
//  lshower_study();
//  lshower_random_match();
//  shower_study();
  return EVENT_OK;
}

int mMpcExAuAuAna::general_data_check_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("AuAuAna");
  if(!hm){
    hm = new Fun4AllHistoManager("AuAuAna");
    se->registerHistoManager(hm);
  }
 
  hbbc_nhits[0] = new TH2D("hbbc_nhits_arm0","Number of Hits vs BBC arm 0",200,0,2000,2000,0,50000);
  hbbc_nhits[0]->GetXaxis()->SetTitle("BBC charge");
  hbbc_nhits[0]->GetYaxis()->SetTitle("Number of hits");
  hm->registerHisto(hbbc_nhits[0]);

  hbbc_nhits[1] = new TH2D("hbbc_nhits_arm1","Number of Hits vs BBC arm 1",200,0,2000,2000,0,50000);
  hbbc_nhits[1]->GetXaxis()->SetTitle("BBC charge");
  hbbc_nhits[1]->GetYaxis()->SetTitle("Number of hits");
  hm->registerHisto(hbbc_nhits[1]);


  hkey_adc_high = new TH2D("hkey_adc_high","MpcEx high ADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_adc_high->GetXaxis()->SetTitle("Key");
  hkey_adc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_adc_high);

  hkey_adc_low = new TH2D("hkey_adc_low","MpcEx low ADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_adc_high->GetXaxis()->SetTitle("Key");
  hkey_adc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_adc_low);

  hkey_rawadc_high = new TH2D("hkey_rawadc_high","MpcEx high rawADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_rawadc_high->GetXaxis()->SetTitle("Key");
  hkey_rawadc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_rawadc_high);

  hkey_rawadc_low = new TH2D("hkey_rawadc_low","MpcEx raw low ADC vs key",50000,-0.5,49999.5,300,-40.5,259.5);
  hkey_rawadc_high->GetXaxis()->SetTitle("Key");
  hkey_rawadc_high->GetYaxis()->SetTitle("ADC");
  hm->registerHisto(hkey_rawadc_low);


  hbbc_adc[0] = new TH2D("hbbc_adc_arm0","ADC vs BBC charge Arm 0",400,0,4000,400,0,800000);
  hbbc_adc[0]->GetXaxis()->SetTitle("BBC charge");
  hbbc_adc[0]->GetYaxis()->SetTitle("ADC value");
  hm->registerHisto(hbbc_adc[0]);

  hbbc_adc[1] = new TH2D("hbbc_adc_arm1","ADC vs BBC charge Arm 1",400,0,4000,400,0,800000);
  hbbc_adc[1]->GetXaxis()->SetTitle("BBC charge");
  hbbc_adc[1]->GetYaxis()->SetTitle("ADC value");
  hm->registerHisto(hbbc_adc[1]);

  hgrammy_high[0] = new Exogram("hgrammy_high0","Exogram high arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy_high[0]);

  hgrammy_high[1] = new Exogram("hgrammy_high1","Exogram high arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy_high[1]);

  hgrammy_low[0] = new Exogram("hgrammy_low0","Exogram low arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy_low[0]);

  hgrammy_low[1] = new Exogram("hgrammy_low1","Exogram low arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
  hm->registerHisto(hgrammy_low[1]);

  hlayer_adc_high[0] = new TH2D("hlayer_adc_high_arm0","Layer ADC high Arm 0",8,-0.5,7.5,300,-40.5,259.5);
  hlayer_adc_high[0]->GetXaxis()->SetTitle("layer");
  hlayer_adc_high[0]->GetYaxis()->SetTitle("ADC per Minipad");
  hm->registerHisto(hlayer_adc_high[0]);

  hlayer_adc_high[1] = new TH2D("hlayer_adc_high_arm1","Layer ADC high Arm 1",8,-0.5,7.5,300,-40.5,259.5);
  hlayer_adc_high[1]->GetXaxis()->SetTitle("layer");
  hlayer_adc_high[1]->GetYaxis()->SetTitle("ADC per Minipad");
  hm->registerHisto(hlayer_adc_high[1]);

  hlayer_adc_low[0] = new TH2D("hlayer_adc_low_arm0","Layer ADC low Arm 0",8,-0.5,7.5,300,-40.5,259.5);
  hlayer_adc_low[0]->GetXaxis()->SetTitle("layer");
  hlayer_adc_low[0]->GetYaxis()->SetTitle("ADC per Minipad");
  hm->registerHisto(hlayer_adc_low[0]);

  hlayer_adc_low[1] = new TH2D("hlayer_adc_low_arm1","Layer ADC low Arm 1",8,-0.5,7.5,300,-40.5,259.5);
  hlayer_adc_low[1]->GetXaxis()->SetTitle("layer");
  hlayer_adc_low[1]->GetYaxis()->SetTitle("ADC per Minipad");
  hm->registerHisto(hlayer_adc_low[1]);


  htower_e = new TH2D("htower_e","MPC Tower E",576,-0.5,576-0.5,400,0,100);
  htower_e->GetXaxis()->SetTitle("MPC Channel");
  htower_e->GetYaxis()->SetTitle("Tower E");
  hm->registerHisto(htower_e);

  hadc_mpc_e[0] = new TH2D("hadc_mpc_e_arm0","MPC E vs ADC Arm 0",400,0,800000,400,0,2000);
  hadc_mpc_e[0]->GetXaxis()->SetTitle("MPCEX ADC");
  hadc_mpc_e[0]->GetYaxis()->SetTitle("MPC E/GeV");
  hm->registerHisto(hadc_mpc_e[0]);

  hadc_mpc_e[1] = new TH2D("hadc_mpc_e_arm1","MPC E vs ADC Arm 1",400,0,800000,400,0,2000);
  hadc_mpc_e[1]->GetXaxis()->SetTitle("MPCEX ADC");
  hadc_mpc_e[1]->GetYaxis()->SetTitle("MPC E/GeV");
  hm->registerHisto(hadc_mpc_e[1]);


  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char hname[200];
      sprintf(hname,"hadc_sensor_high_layer%d_arm%d",ilayer,iarm);
      hadc_sensor_layer_high[iarm][ilayer] = new TH2D(hname,hname,24,-0.5,24-0.5,350,-50-0.5,300-0.5);
      hadc_sensor_layer_high[iarm][ilayer]->GetXaxis()->SetTitle("Sensor");
      hadc_sensor_layer_high[iarm][ilayer]->GetYaxis()->SetTitle("high ADC");
      hm->registerHisto(hadc_sensor_layer_high[iarm][ilayer]);
          
      sprintf(hname,"hadc_sensor_low_layer%d_arm%d",ilayer,iarm);
      hadc_sensor_layer_low[iarm][ilayer] = new TH2D(hname,hname,24,-0.5,24-0.5,350,-50-0.5,300-0.5);
      hadc_sensor_layer_low[iarm][ilayer]->GetXaxis()->SetTitle("Sensor");
      hadc_sensor_layer_low[iarm][ilayer]->GetYaxis()->SetTitle("low ADC");
      hm->registerHisto(hadc_sensor_layer_low[iarm][ilayer]);
      for(int isen = 0;isen< 24;isen++){
        sprintf(hname,"hHL_sensor_arm%d_layer%d_sensor%d",iarm,ilayer,isen);
	hHL_sensor[iarm][ilayer][isen] = new TH2D(hname,hname,350,-50-0.5,300-0.5,350,-50-0.5,300-0.5);
	hHL_sensor[iarm][ilayer][isen]->GetYaxis()->SetTitle("High ADC");
	hHL_sensor[iarm][ilayer][isen]->GetXaxis()->SetTitle("Low ADC");
	hm->registerHisto(hHL_sensor[iarm][ilayer][isen]);
        
	sprintf(hname,"hHL_sensor_raw_arm%d_layer%d_sensor%d",iarm,ilayer,isen);
	hHL_sensor_raw[iarm][ilayer][isen] = new TH2D(hname,hname,350,-50-0.5,300-0.5,350,-50-0.5,300-0.5);
	hHL_sensor_raw[iarm][ilayer][isen]->GetYaxis()->SetTitle("High ADC");
	hHL_sensor_raw[iarm][ilayer][isen]->GetXaxis()->SetTitle("Low ADC");
	hm->registerHisto(hHL_sensor_raw[iarm][ilayer][isen]);
      }
    }
  }


  return EVENT_OK;
}

int mMpcExAuAuAna::general_data_check_study(){
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
    int arm = mpcex_raw_hit->arm();
    int quadrant = mpcex_raw_hit->quadrant();
    int sensor = mpcex_raw_hit->sensor_in_quadrant();
    int sensor_index = sensor+6*quadrant;
    int layer = mpcex_raw_hit->layer();
    hHL_sensor_raw[arm][layer][sensor_index]->Fill(low_adc,high_adc);
    
    TMpcExCalib *calib = _mpcex_calib_container->get(key);
    if(calib->high_dead_hot_status()>0)hkey_rawadc_high->Fill(key,high_adc);
    if(calib->low_dead_hot_status()>0)hkey_rawadc_low->Fill(key,low_adc);
//    if(high_adc > 0){
//      hgrammy_high[arm]->FillEx(key,high_adc);
//    }
//    if(low_adc > 0){
//      hgrammy_low[arm]->FillEx(key,low_adc);
//    }
//    TMpcExCalib *calib = _mpcex_calib_container->get(key);
//    if(calib->low_dead_hot_status()>0){
//      cout <<"bad low "<<endl;
//      hgrammy_low[arm]->FillEx(key,1);
//    }
//    if(calib->high_dead_hot_status()>0){
//      hgrammy_high[arm]->FillEx(key,1);
//      cout <<"bad high "<<endl;
//    }
    begin++;
  }

  
  int Nhits = _mpcex_hit_container->size();
  double Arm_totAdc[2] = {0,0};
  int Arm_Nhits_layer_high[2][8] = {{0.}};
  int Arm_Nhits_layer_low[2][8] = {{0.}};
  int Arm_Nhits[2] = {0,0};
  double layer_adc_high[2][8] = {{0.}};
  double layer_adc_low[2][8] = {{0.}};

//cout <<"1"<<endl;
  for(int i = 0;i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    int arm = hit->arm();
    int key = hit->key();
    int quadrant = hit->quadrant();
    int sensor = hit->sensor_in_quadrant();
    int sensor_index = sensor+6*quadrant;
    int layer = hit->layer();
    double high_q = hit->high();
    double low_q = hit->low();
    hHL_sensor[arm][layer][sensor_index]->Fill(low_q,high_q);

    Arm_Nhits[arm]++;
//      if(high_q > 0)hgrammy_high[arm]->FillEx(key,high_q);
//      if(low_q > 0)hgrammy_low[arm]->FillEx(key,low_q);
    TMpcExCalib *calib = _mpcex_calib_container->get(key);
    if(calib->high_dead_hot_status()>0)hkey_adc_high->Fill(key,high_q);
    if(calib->low_dead_hot_status()>0)hkey_adc_low->Fill(key,low_q);

    if(hit->isGoodHighHit()){
      hadc_sensor_layer_high[arm][layer]->Fill(sensor_index,high_q);
      Arm_totAdc[arm] += high_q;
      layer_adc_high[arm][layer] += high_q; 
      Arm_Nhits_layer_high[arm][layer]++;
      if(high_q > 0)hgrammy_high[arm]->FillEx(key,high_q);
//      hkey_adc_high->Fill(key,high_q);
    }
    if(hit->isGoodLowHit()){
      hadc_sensor_layer_low[arm][layer]->Fill(sensor_index,low_q);
      layer_adc_low[arm][layer] += low_q;
      Arm_Nhits_layer_low[arm][layer]++;
      if(low_q > 0)hgrammy_low[arm]->FillEx(key,low_q);
//      hkey_adc_low->Fill(key,low_q);
    }
  }
  
//cout<<"2"<<endl;
  //MPC part
  int Ntowers = _mpc_tower_container->size();
  double Tot_Mpc_e[2] = {0,0};
  for(int i = 0;i < Ntowers;i++){
    mpcTowerContent* tower = _mpc_tower_container->getTower(i);
    double tower_e = tower->get_energy();
    double tower_ch = tower->get_ch();
    int arm = 0;
    if(tower_ch > 288) arm = 1;
    if(tower_e > 0) Tot_Mpc_e[arm] += tower_e;
    htower_e->Fill(tower_ch,tower_e);
  }

  for(int iarm = 0;iarm < 2;iarm++){
    hbbc_adc[iarm]->Fill(_bbc_charge[iarm],Arm_totAdc[iarm]);
    hadc_mpc_e[iarm]->Fill(Arm_totAdc[iarm],Tot_Mpc_e[iarm]);
//    cout <<Arm_totAdc[iarm]<<" "<<_bbc_charge[iarm]<<" "<<Tot_Mpc_e[iarm]<<endl;
    hbbc_nhits[iarm]->Fill(_bbc_charge[iarm],Arm_Nhits[iarm]);
    for(int ilayer = 0;ilayer < 8;ilayer++){
      if(Arm_Nhits_layer_high[iarm][ilayer] > 0){
        double mean_adc = layer_adc_high[iarm][ilayer]/Arm_Nhits_layer_high[iarm][ilayer];
	hlayer_adc_high[iarm]->Fill(ilayer,mean_adc);
      }
      if(Arm_Nhits_layer_low[iarm][ilayer] > 0){
        double mean_adc = layer_adc_low[iarm][ilayer]/Arm_Nhits_layer_low[iarm][ilayer];
	hlayer_adc_low[iarm]->Fill(ilayer,mean_adc);
      }
    }
  }

//cout<<"3"<<endl;
  return EVENT_OK;
}

int mMpcExAuAuAna::lshower_init(){
  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("AuAuAna");
  if(!hm){
    hm = new Fun4AllHistoManager("AuAuAna");
    se->registerHistoManager(hm);
  }
 
  hlshowr_layer_e[0] = new TH2D("hlshowr_layer_e0","Layer E Arm 0",8,-0.5,7.5,1000,0,0.5);
  hlshowr_layer_e[0]->GetXaxis()->SetTitle("Layer");
  hlshowr_layer_e[0]->GetYaxis()->SetTitle("Layer E");
  hm->registerHisto(hlshowr_layer_e[0]);

  hlshowr_layer_e[1] = new TH2D("hlshowr_layer_e1","Layer E Arm 1",8,-0.5,7.5,1000,0,0.5);
  hlshowr_layer_e[1]->GetXaxis()->SetTitle("Layer");
  hlshowr_layer_e[1]->GetYaxis()->SetTitle("Layer E");
  hm->registerHisto(hlshowr_layer_e[1]);

  hclosest_cluster[0] = new TH2D("hclosest_cluster0","Closest Cluster Arm 0",100,-24,24,100,-24,24);
  hclosest_cluster[0]->GetXaxis()->SetTitle("dX/cm");
  hclosest_cluster[0]->GetYaxis()->SetTitle("dY/cm");
  hm->registerHisto(hclosest_cluster[0]);

  hclosest_cluster[1] = new TH2D("hclosest_cluster1","Closest Cluster Arm 1",100,-24,24,100,-24,24);
  hclosest_cluster[1]->GetXaxis()->SetTitle("dX/cm");
  hclosest_cluster[1]->GetYaxis()->SetTitle("dY/cm");
  hm->registerHisto(hclosest_cluster[1]);

  hclosest_tower[0] = new TH2D("hclosest_tower0","Closest Tower Arm 0",100,-24,24,100,-24,24);
  hclosest_tower[0]->GetXaxis()->SetTitle("dX/cm");
  hclosest_tower[0]->GetYaxis()->SetTitle("dY/cm");
  hm->registerHisto(hclosest_tower[0]);

  hclosest_tower[1] = new TH2D("hclosest_tower1","Closest Tower Arm 1",100,-24,24,100,-24,24);
  hclosest_tower[1]->GetXaxis()->SetTitle("dX/cm");
  hclosest_tower[1]->GetYaxis()->SetTitle("dY/cm");
  hm->registerHisto(hclosest_tower[1]);

  hclosest_clus_dx_bbc[0] = new TH2D("hclosest_clus_dx_bbc0","Closest Cluster dX vs BBC Arm0",200,0,2000,100,-24,24);
  hclosest_clus_dx_bbc[0]->GetXaxis()->SetTitle("BBC charge");
  hclosest_clus_dx_bbc[0]->GetYaxis()->SetTitle("dX/cm");
  hm->registerHisto(hclosest_clus_dx_bbc[0]);

  hclosest_clus_dx_bbc[1] = new TH2D("hclosest_clus_dx_bbc1","Closest Cluster dX vs BBC Arm1",200,0,2000,100,-24,24);
  hclosest_clus_dx_bbc[1]->GetXaxis()->SetTitle("BBC charge");
  hclosest_clus_dx_bbc[1]->GetYaxis()->SetTitle("dX/cm");
  hm->registerHisto(hclosest_clus_dx_bbc[1]);

  hclosest_clus_dy_bbc[0] = new TH2D("hclosest_clus_dy_bbc0","Closest Cluster dY vs BBC Arm0",200,0,2000,100,-24,24);
  hclosest_clus_dy_bbc[0]->GetXaxis()->SetTitle("BBC charge");
  hclosest_clus_dy_bbc[0]->GetYaxis()->SetTitle("dY/cm");
  hm->registerHisto(hclosest_clus_dy_bbc[0]);

  hclosest_clus_dy_bbc[1] = new TH2D("hclosest_clus_dy_bbc1","Closest Cluster dY vs BBC Arm1",200,0,2000,100,-24,24);
  hclosest_clus_dy_bbc[1]->GetXaxis()->SetTitle("BBC charge");
  hclosest_clus_dy_bbc[1]->GetYaxis()->SetTitle("dX/cm");
  hm->registerHisto(hclosest_clus_dy_bbc[1]);

  return EVENT_OK;
}

int mMpcExAuAuAna::lshower_study(){
  int NLShowers = _mpcex_lshower_container->size();
  for(int i = 0;i < NLShowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    int arm = lshower->get_arm();
    double mpcE3x3 = lshower->get_mpc_e3x3();
    double mpcex_e = lshower->get_mpcex_e();
//    if(mpcex_e < 1.5) continue;
//    cout <<"fired layers: "<<lshower->get_n_fired_layers()<<endl;
    if(lshower->get_n_fired_layers() < 7) continue;
    double towr_dx = lshower->get_closest_tower_dx();
    double towr_dy = lshower->get_closest_tower_dy();
    double clus_dx = lshower->get_closest_cluster_dx();
    double clus_dy = lshower->get_closest_cluster_dy();
    hclosest_tower[arm]->Fill(towr_dx,towr_dy);
    hclosest_clus_dx_bbc[arm]->Fill(_bbc_charge[arm],clus_dx);
    hclosest_clus_dy_bbc[arm]->Fill(_bbc_charge[arm],clus_dy);
    if(mpcex_e < 1.5 &&_bbc_charge[arm] < 100)hclosest_cluster[arm]->Fill(clus_dx,clus_dy);
    if(lshower->get_n_fired_layers() == 8 && lshower->get_hits_num() == 8){
      for(int ilayer = 0;ilayer < 8;ilayer++){
        double layer_e = lshower->get_e_layer(ilayer);
//	cout<<"layer e: "<<layer_e<<endl;
	if(layer_e > 0){
          hlshowr_layer_e[arm]->Fill(ilayer,layer_e);
	}
      }
    }
  }

  return EVENT_OK;
}

int mMpcExAuAuAna::event_display(){
  for(int i = 0;i < 2; i++){
    if(grammyl[i]) delete grammyl[i];
    if(grammyh[i]) delete grammyh[i];
    char name[50];
    sprintf(name,"Arm %d for low q",i);
    grammyl[i] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
    sprintf(name,"Arm %d for high q",i);
    grammyh[i] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
  }
cout <<"1"<<endl;
  int Nhits = _mpcex_hit_container->size();
  cout << "Number of Hits: "<<Nhits<<endl;
  for(int i = 0;i < Nhits;i++){
    TMpcExHit* hit = _mpcex_hit_container->getHit(i);
    double high_q = hit->high();
    double low_q = hit->low();
    int key = hit->key();
    int arm = hit->arm();
    if(hit->isGoodHighHit()){
      if(high_q > 5) grammyh[arm]->FillEx(key,high_q);
    }
    if(hit->isGoodLowHit()){
      if(low_q > 5) grammyl[arm]->FillEx(key,low_q);
    }
  }

cout <<"2"<<endl;
//mpc part
  for(int i = 0;i < 2;i++){
    if(hmpc_gridxy[i]) delete hmpc_gridxy[i];
    char name[100];
    sprintf(name,"hmpc_gridxy_arm%d",i);
    hmpc_gridxy[i] =  new TH2D(name,name,600,-24,24,600,-24,24);
  }

cout <<"3"<<endl;
  _mpc_map = MpcMap::instance();
  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0.) continue;
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
 
cout<<"4"<<endl;

  TCanvas* c_mpc0 = new TCanvas("c_mpc0","c_mpc0",1500,800);
  c_mpc0->Divide(2,1);
  c_mpc0->cd(1);
  hmpc_gridxy[0]->Draw("colz");
  c_mpc0->cd(2);
  grammyh[0]->Project3D("yx")->DrawCopy("colz");
  TCanvas* c_mpc1 = new TCanvas("c_mpc1","c_mpc1",1500,800);
  c_mpc1->Divide(2,1);
  c_mpc1->cd(1);
  hmpc_gridxy[1]->Draw("colz");
  c_mpc1->cd(2);
  grammyh[1]->Project3D("yx")->DrawCopy("colz");

  return EVENT_OK;
}

int mMpcExAuAuAna::shower_init(){
  
  return EVENT_OK;
}

int mMpcExAuAuAna::shower_study(){
  int Nshowers = _mpcex_shower_container->size();
//  cout <<"Number of Showers: "<<Nshowers<<endl;
  for(int i = 0;i < Nshowers;i++){
    TMpcExShower* shower = _mpcex_shower_container->getShower(i);
    double clus_hdx = shower->get_ClosestMPCClusterDistanceX();
    double clus_hdy = shower->get_ClosestMPCClusterDistanceY();
    int arm = shower->get_arm();
    double mpcex_e = shower->get_esum();
    int nlayers = shower->get_nlayers();
    if(nlayers < 3) continue;
    if(mpcex_e < 1.5) continue;
    double lz = 220.9 - _vertex;
    if(arm == 0) lz = -220.9 - _vertex;
    hclosest_cluster[arm]->Fill(clus_hdx*lz,clus_hdy*lz);
  }
  return EVENT_OK;
}

int mMpcExAuAuAna::lshower_random_match(){
  
  if(mycluster_list.size() > 0){
    int NLShowers = _mpcex_lshower_container->size();
    for(int i = 0;i < NLShowers;i++){
      TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
      int arm = lshower->get_arm();
      if(_bbc_charge[arm] > 200) continue;
      if(lshower->get_n_fired_layers() < 7) continue;
      if(lshower->get_mpcex_e() < 1.5) continue;
      double hsx = lshower->get_hsx();
      double hsy = lshower->get_hsy();
      double lz = 220.9 - _vertex;
      if(arm == 0) lz = -220.9 - _vertex;
      double shower_x = hsx*lz;
      double shower_y = hsy*lz;
      double sm_dx = 9999;
      double sm_dy = 9999;
      double sm_dr = 9999;
      int NMyCluster = mycluster_list.size();
      for(int j = 0;j < NMyCluster;j++){
        MyCluster myclus = mycluster_list[j];
	int clus_arm = myclus.arm;
	if(clus_arm != arm) continue;
	double clus_x = myclus.x;
	double clus_y = myclus.y;
	double dr = (shower_x-clus_x)*(shower_x-clus_x)+(shower_y-clus_y)*(shower_y-clus_y);
	dr = sqrt(dr);
	if(dr < sm_dr){
          sm_dr = dr;
	  sm_dx = shower_x - clus_x;
	  sm_dy = shower_y - clus_y;
	}
	used = true;
      }//j
      if(sm_dr < 200){
        hclosest_cluster[arm]->Fill(sm_dx,sm_dy);
      }
    }//i
  }//if size
 
  if(used){
    mycluster_list.clear();
    int Nclusters = _mpc_cluster_container->size();
    for(int i = 0;i < Nclusters;i++){
      mpcClusterContent* clus = _mpc_cluster_container->getCluster(i);
      int arm = clus->arm();
      if(_bbc_charge[arm]>200) continue;
      double x = clus->x();
      double y = clus->y();
      MyCluster myclus;
      myclus.arm = arm;
      myclus.x = x;
      myclus.y = y;
      mycluster_list.push_back(myclus);
      used = false;
    }
  }
  return EVENT_OK;
}
