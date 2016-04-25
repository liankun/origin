#include "mMpcExCreateTree.h"
#include "TMpcExLShowerContainer.h"
#include <TMpcExShowerContainer.h>
#include <TMpcExShower.h>
#include "TMpcExLShower.h"
#include <MpcExSpin.h>
#include "TrigLvl1.h"
#include <PHIODataNode.h>
#include <getClass.h>
#include <PHGlobal.h>
#include <BbcOut.h>
#include <Fun4AllReturnCodes.h>
#include "MpcExEventQuality.h"
#include <stdlib.h>
#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TMpcExHit.h>
#include <TMpcExHitContainer.h>
#include <mpcClusterContainer.h>
#include <mpcTowerContainer.h>
#include <mpcClusterContent.h>
#include <mpcTowerContent.h>
#include "SpinDataEventOut.h"

using namespace std;
using namespace findNode;

mMpcExCreateTree::mMpcExCreateTree(const char* name) : SubsysReco(name)
{
  _mpcex_shower_container = NULL;
  _mpcex_lshower_container = NULL;
  mytree = NULL;
  omyfile = NULL;
  file_name = NULL;
  _evt_quality = NULL;
  _mpcex_hit_container = NULL;
  _mpc_cluster_container = NULL;
  _mpc_tower_container = NULL;
  _mpcex_spin = NULL;
  _triglvl1 = NULL;
  _triglvl1_clock_cross = -999;
}

mMpcExCreateTree::~mMpcExCreateTree(){

}

int mMpcExCreateTree::Init(PHCompositeNode* topNode){
  string file_name2 = file_name;
//  file_name2.append(file_name);
  omyfile = new TFile(file_name2.c_str(),"RECREATE");
  omyfile->SetCompressionSettings(9);
  mytree = new TTree("mpcex_showers","mpcex_showers");
//  mytree->Branch("EventInfo",&evtinfo.run_number,"RunNumber/I:FillNumber/I:Lvl1Cross/I:GL1Cross/I:CrossShift/I:YPat/I:BPat/I:YPol/F:BPol/F:YPolEr/F:BPolEr/F");
  mytree->Branch("showers","std::vector<MyShower>",&shower_list);
  mytree->Branch("lshowers","std::vector<MyLShower>",&lshower_list);
  mytree->Branch("clusters","std::vector<MyMpcCluster>",&mpc_cluster_list);
  mytree->Branch("towers","std::vector<MyMpcTower>",&mpc_tower_list);
//  mytree->Branch("hits","std::vector<MyMpcExHit>",&mpcex_shower_hits);
//  mytree->Branch("lhits","std::vector<MyMpcExHit>",&mpcex_lshower_hits);

  return EVENT_OK;
}

void mMpcExCreateTree::set_interface_ptrs(PHCompositeNode* topNode){
  _mpcex_hit_container = getClass<TMpcExHitContainer>(topNode,"TMpcExHitContainer");
  if(!_mpcex_hit_container){
    cout << PHWHERE <<":: No TMpcExHitContainer!!!"<<endl;
    exit(1);
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

  _evt_quality = getClass<MpcExEventQuality>(topNode,"MpcExEventQuality");
  if(!_evt_quality){
    cout << PHWHERE <<"No MpcExEventQuality !!!"<<endl;
    exit(1);
  }

  _mpc_cluster_container = getClass<mpcClusterContainer>(topNode,"mpcClusterContainer");
  if(!_mpc_cluster_container){
    cout <<PHWHERE <<":: No mpcClusterContainer!!!"<<endl;
    exit(1);
  }
  
  _mpc_tower_container = getClass<mpcTowerContainer>(topNode,"mpcTowerContainer");
  if(!_mpc_tower_container){
    cout << PHWHERE <<":: No mpcTowerContainer!!!"<<endl;
    exit(1);
  }

   _triglvl1 = getClass<TrigLvl1>(topNode,"TrigLvl1");
   if(!_triglvl1){
      cout << PHWHERE <<":: No TrigLvl1 !!!"<<endl;
      exit(1);
   }

   _mpcex_spin = MpcExSpin::instance();
   if(!_mpcex_spin){
     cout <<"Get MpcExSpin Failled !!!"<<endl;
     exit(1);
   }
   cout <<"print the MpcExSpin: "<<endl;
   _mpcex_spin->Print();

}

int mMpcExCreateTree::InitRun(PHCompositeNode* topNode){
  set_interface_ptrs(topNode);
  return EVENT_OK;
}

int mMpcExCreateTree::process_event(PHCompositeNode* topNode){
  if(!_evt_quality->IsEventWanted()) return EVENT_OK; 
//  cout <<"process mMpcExCreateTree "<<endl;

/*
  _triglvl1_clock_cross = _triglvl1->get_lvl1_clock_cross();
  SpinDataEventOut* SpinData = getClass<SpinDataEventOut> (topNode, "SpinDataEventOut");
  evtinfo.gl1_cross_id = -1;
  evtinfo.lvl1_clock_cross = _triglvl1_clock_cross;
  if(!SpinData) cout <<"No SpinDataEventOut !!!"<<endl;
  else{
    _triglvl1_clock_cross = SpinData->GetGL1CrossingID();
    evtinfo.gl1_cross_id = _triglvl1_clock_cross;
  }
  
  float ypol = _mpcex_spin->GetPolYellow(_triglvl1_clock_cross);
  float bpol = _mpcex_spin->GetPolBlue(_triglvl1_clock_cross);
  int ypat = _mpcex_spin->GetSpinPatternYellow(_triglvl1_clock_cross);
  int bpat = _mpcex_spin->GetSpinPatternBlue(_triglvl1_clock_cross);
  int fillnumber = _mpcex_spin->GetFillNumber();
  int runnumber = _mpcex_spin->GetRunNumber();
  evtinfo.run_number = runnumber;
  evtinfo.fill_number = fillnumber;
  evtinfo.ypat = ypat;
  evtinfo.bpat = bpat;
  evtinfo.ypol = ypol;
  evtinfo.bpol = bpol;
  evtinfo.cross_shift = _mpcex_spin->GetCrossingShift();
  evtinfo.bpolerr = _mpcex_spin->GetPolErrorBlue(_triglvl1_clock_cross);
  evtinfo.ypolerr = _mpcex_spin->GetPolErrorYellow(_triglvl1_clock_cross);
*/
  lshower_list.clear();
  shower_list.clear();
  mpcex_shower_hits.clear();
  mpcex_lshower_hits.clear();
  unsigned int NLshowers = _mpcex_lshower_container->size();
//  cout <<"Number of lshowers: "<<NLshowers<<endl;
  lshower_list.reserve(NLshowers);
//  cout <<"1"<<endl;
  for(unsigned int i = 0;i < NLshowers;i++){
    TMpcExLShower* lshower = _mpcex_lshower_container->get_shower(i);
    MyLShower mylshower;
    
    int arm = lshower->get_arm();
    int Nhits = lshower->get_hits_num();
    double x_mean[8] = {0.};
    double y_mean[8] = {0.};
    double x2_mean[8] = {0.};
    double y2_mean[8] = {0.};
    double norm[8] = {0.};
//    cout <<"1.1"<<endl;
//    cout <<"Number of Hits: "<<Nhits<<endl;
//    std::vector<MyMpcExHit>* p_hits = &(mylshower.hits);
//    p_hits->clear();
//    p_hits->reserve(Nhits);
    for(int ihit = 0;ihit < Nhits;ihit++){
      int hit_key = lshower->get_hit(ihit);
//      cout <<"1.1.1"<<endl;
      TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
//      cout <<"1.1.2"<<endl;
      double hit_q = hit->combined();
      
//      MyMpcExHit myhit;
//      myhit.key = hit_key;
//      myhit.q = hit_q;
//      p_hits->push_back(myhit);
//      myhit.shower = i;
//      mpcex_lshower_hits.push_back(myhit);
       
      double x = hit->x();
      double y = hit->y();

//      cout <<"x "<<x<<" y "<<y<<endl;
      int layer = hit->layer();
      x_mean[layer] += x*hit_q;
      x2_mean[layer] += x*x*hit_q;
      y_mean[layer] += y*hit_q;
      y2_mean[layer] += y*y*hit_q;     
      norm[layer] += hit_q;
    }
//    cout <<"1.2"<<endl; 
    mylshower.arm = arm;
    mylshower.first_layer = lshower->get_first_fired_layer();
    mylshower.nlayers = lshower->get_n_fired_layers();
    mylshower.mpc_e3x3 = lshower->get_mpc_e3x3();
    mylshower.mpc_e5x5 = lshower->get_mpc_e5x5();
    mylshower.mpcN3x3 = lshower->get_n_fired_towers3x3();
    mylshower.mpcN5x5 = lshower->get_n_fired_towers5x5();
    mylshower.hsx = lshower->get_hsx();
    mylshower.hsy = lshower->get_hsy();

    for(int ix = 0;ix < 5;ix++){
      for(int iy = 0;iy < 5;iy++){
        mylshower.mpc_towers_ch[ix][iy] = lshower->get_mpc_towers_ch(ix,iy);
        mylshower.mpc_towers_e[ix][iy] = lshower->get_mpc_towers_e(ix,iy);
      }
    }

    PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
    BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
    if(!bbcout && !phglobal){
      cout <<"No BbcOut or PHGlobal !!!"<<endl;
      exit(1);
    }
    float vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
    mylshower.vertex = vertex;
 //   float z = MpcExConstants::MPC_REFERENCE_Z - vertex;
//    if(arm == 0) z = -(MpcExConstants::MPC_REFERENCE_Z) - vertex;
    mylshower.ClosestMPCClusterDistanceX = lshower->get_closest_cluster_dx();
    mylshower.ClosestMPCClusterDistanceY = lshower->get_closest_cluster_dy();

    for(int ilayer = 0;ilayer < 8;ilayer++){
      double layer_e = lshower->get_e_layer(ilayer);
      mylshower.rms_x[ilayer] = 0;
      mylshower.rms_y[ilayer] = 0;
      mylshower.e_layer[ilayer] = 0;
      if(layer_e < 0 )continue;
      mylshower.e_layer[ilayer] = layer_e;
      if(norm[ilayer] <= 0) continue;
      x_mean[ilayer] = x_mean[ilayer]/norm[ilayer];
      y_mean[ilayer] = y_mean[ilayer]/norm[ilayer];
      x2_mean[ilayer] = x2_mean[ilayer]/norm[ilayer];
      y2_mean[ilayer] = y2_mean[ilayer]/norm[ilayer];
      if(x2_mean[ilayer] - x_mean[ilayer]*x_mean[ilayer] > 0){

        mylshower.rms_x[ilayer] = sqrt(x2_mean[ilayer] - x_mean[ilayer]*x_mean[ilayer]);
//        cout<<x2_mean[ilayer]<<" "<<x_mean[ilayer]<<" "
//	    <<x_mean[ilayer]*x_mean[ilayer]<<" " 
//	    <<x2_mean[ilayer] - x_mean[ilayer]*x_mean[ilayer]<<" "<<mylshower.rms_x[ilayer]<<endl;
      }
      if(y2_mean[ilayer] - y_mean[ilayer]*y_mean[ilayer] > 0){
        mylshower.rms_y[ilayer] = sqrt(y2_mean[ilayer] - y_mean[ilayer]*y_mean[ilayer]);
//        cout<<y2_mean[ilayer]<<" "<<y_mean[ilayer]<<" "
//	    <<y_mean[ilayer]*y_mean[ilayer]<<" "
//	    <<y2_mean[ilayer] - y_mean[ilayer]*y_mean[ilayer]<<" "<<mylshower.rms_y[ilayer]<<endl;
      }
    }
//    cout <<"1.3"<<endl;
    lshower_list.push_back(mylshower);
  }
//  cout <<"2"<<endl;


  unsigned int NShowers = _mpcex_shower_container->size();
//  cout<<"Number of showers: "<<NShowers<<endl;
  shower_list.reserve(NShowers);
  for(unsigned int i = 0;i < NShowers;i++){
    TMpcExShower* shower = _mpcex_shower_container->getShower(i);
    MyShower myshower;

    myshower.arm = shower->get_arm();
    myshower.nlayers = shower->get_nlayers();
    myshower.first_layer = shower->get_first_layer();
    myshower.mpc_e3x3 = shower->get_mpcE3x3();
    myshower.mpc_e5x5 = shower->get_mpcE5x5();
    myshower.mpcN3x3 = shower->get_mpcN3x3();
    myshower.mpcN5x5 = shower->get_mpcN5x5();
    myshower.mpcPeakix = shower->get_mpcPeakix();
    myshower.mpcPeakiy = shower->get_mpcPeakiy();
    myshower.CalibEInRange = shower->get_CalibEInRange();
    myshower.roughTotE = shower->get_roughTotE();
    myshower.mpcCentTwrE = shower->get_mpcCentTwrE();
    myshower.ClosestMPCClusterDistanceX = shower->get_ClosestMPCClusterDistanceX();
    myshower.ClosestMPCClusterDistanceY = shower->get_ClosestMPCClusterDistanceY();
    myshower.mpcECorr = shower->get_mpcECorr();
    myshower.esum = shower->get_esum();
    myshower.raw_esum = shower->get_raw_esum();
    myshower.hsx = shower->get_hsx();
    myshower.hsy = shower->get_hsy();
    myshower.mpc33hsx = shower->get_mpc33hsx();
    myshower.mpc33hsy = shower->get_mpc33hsy();
    
    PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
    BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
    if(!bbcout && !phglobal){
      cout <<"No BbcOut or PHGlobal !!!"<<endl;
      exit(1);
    }
    float vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
    myshower.vertex = vertex;
   
    
    int Nhits = shower->sizeHits();
    std::vector<MyMpcExHit>* p_hits = &(myshower.hits);
    p_hits->clear();
    p_hits->reserve(Nhits);

    double x_mean[8] = {0.};
    double y_mean[8] = {0.};
    double x2_mean[8] = {0.};
    double y2_mean[8] = {0.};
    double norm[8] = {0.};

    for(int ihit = 0;ihit < Nhits;ihit++){
      int hit_key = shower->getHit(ihit);
      TMpcExHit* hit = _mpcex_hit_container->get_hit_by_key(hit_key);
      double hit_q = hit->combined();
      
      MyMpcExHit myhit;
      myhit.key = hit_key;
      myhit.q = hit_q;
//      myhit.shower = i;
//      mpcex_shower_hits.push_back(myhit);
      p_hits->push_back(myhit);

      double x = hit->x();
      double y = hit->y();
      int layer = hit->layer();
      norm[layer] += hit_q;
      x_mean[layer] += x*hit_q;
      x2_mean[layer] += x*x*hit_q;
      y_mean[layer] += y*hit_q;
      y2_mean[layer] += y*y*hit_q;          
    }
    for(int ilayer = 0;ilayer < 8;ilayer++){
      double layer_e = shower->get_e_layer(ilayer);
      myshower.rms_x[ilayer] = 0;
      myshower.rms_y[ilayer] = 0;
      myshower.e_layer[ilayer] = 0;
      if(layer_e < 0)continue;
      myshower.e_layer[ilayer] = layer_e;
      x_mean[ilayer] = x_mean[ilayer]/norm[ilayer];
      y_mean[ilayer] = y_mean[ilayer]/norm[ilayer];
      x2_mean[ilayer] = x2_mean[ilayer]/norm[ilayer];
      y2_mean[ilayer] = y2_mean[ilayer]/norm[ilayer];
      if(x2_mean[ilayer] - x_mean[ilayer]*x_mean[ilayer] > 0){
        myshower.rms_x[ilayer] = sqrt(x2_mean[ilayer] - x_mean[ilayer]*x_mean[ilayer]);
      }
      if(y2_mean[ilayer] - y_mean[ilayer]*y_mean[ilayer] > 0){
        myshower.rms_y[ilayer] = sqrt(y2_mean[ilayer] - y_mean[ilayer]*y_mean[ilayer]);
      }
    }
    shower_list.push_back(myshower);
  }
 
 
//  cout <<"3"<<endl;
//  cout <<"4"<<endl;
  
  //Mpc Stuff
  mpc_cluster_list.clear();
  mpc_tower_list.clear();


  int fNMpcClusters = _mpc_cluster_container->size();
  mpc_cluster_list.reserve(fNMpcClusters);
  for(int iMPCClus=0 ; iMPCClus<fNMpcClusters ; iMPCClus++){
    mpcClusterContent *clus =  _mpc_cluster_container->getCluster(iMPCClus);
    MyMpcCluster mycluster;
    mycluster.arm = clus->arm();
    mycluster.x = clus->x();
    mycluster.y = clus->y();
    mycluster.e = clus->e();
    mycluster.z = clus->z();
    mycluster.chi2 = clus->chi2();
    
    
    PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
    BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
    if(!bbcout && !phglobal){
      cout <<"No BbcOut or PHGlobal !!!"<<endl;
      exit(1);
    }
    float vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
    mycluster.vertex = vertex;
    
    
    int ntowers = clus->multiplicity();
    std::vector<int>* p_towers = &(mycluster.towers);
    p_towers->clear();
    p_towers->reserve(ntowers);
    for(int ictower = 0;ictower < ntowers;ictower++){
      int tower_ch = clus->towerid(ictower);
      p_towers->push_back(tower_ch);
//      mytower.cluster = iMPCClus;
//      mpc_tower_list.push_back(mytower);
    }
    mpc_cluster_list.push_back(mycluster);
  }

  int NMpcTowers = _mpc_tower_container->size();
  mpc_tower_list.reserve(NMpcTowers);
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    float e_tower = ctwr->get_energy();
    float tof = ctwr->get_tof();
    MyMpcTower mytower;
    mytower.e = e_tower;
    mytower.ch = tow_ch;
    mytower.tof = tof;
    mpc_tower_list.push_back(mytower);
  }
 
  mytree->Fill();
  
  return EVENT_OK; 
}

int mMpcExCreateTree::End(PHCompositeNode* topNode){
  omyfile->cd();
  mytree->Write();
  omyfile->Close();
  return EVENT_OK;
}

