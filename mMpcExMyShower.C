#include "mMpcExMyShower.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "MpcExRawHit.h"
#include "MpcExEventQuality.h"
#include "Exogram.h"

#include "MpcMap.h"
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

#include "PHIODataNode.h"
#include "getClass.h"
#include "BbcOut.h"
#include "PHGlobal.h"

#include "Fun4AllReturnCodes.h"
#include "Fun4AllServer.h"
#include "Fun4AllHistoManager.h"

#include "iostream"
#include <fstream>
#include "stdlib.h"

#include <algorithm>
#include <functional>
#include <TCanvas.h>
#include <TH2D.h>

using namespace std;
using namespace findNode;



mMpcExMyShower::mMpcExMyShower(const char* name):
  SubsysReco(name)
{
  _vertex = -9999.0; 
  grammy[0] = NULL;
  grammy[1] = NULL;
}

mMpcExMyShower::~mMpcExMyShower(){
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      while(!_peaks_map[arm][layer].empty()){
        map<unsigned int,MpcExPeakHits*>::iterator iter = _peaks_map[arm][layer].begin();
	delete iter->second;
	_peaks_map[arm][layer].erase(iter);
      }
    }
  
    for(unsigned int i = 0;i < _track_list[arm].size();i++){
      if(_track_list[arm][i]) delete _track_list[arm][i];
    }
    _track_list[arm].clear();
  }
}

int mMpcExMyShower::Init(PHCompositeNode* topNode){
  htracks_vs_towers[0] = new TH2D("htracks_vs_towers_arm0","tracks vs fired towers",100,-0.5,99.5,100,-0.5,99.5);
  htracks_vs_towers[0]->GetXaxis()->SetTitle("fired towers");
  htracks_vs_towers[0]->GetYaxis()->SetTitle("mpcex tracks");

  htracks_vs_towers[1] = new TH2D("htracks_vs_towers_arm1","tracks vs fired towers",100,-0.5,99.5,100,-0.5,99.5);
  htracks_vs_towers[1]->GetXaxis()->SetTitle("fired towers");
  htracks_vs_towers[1]->GetYaxis()->SetTitle("mpcex tracks");


  htracks_towers[0] = new TH3D("htracks_towers_arm0","different between tracks and towers arm 0",288,-0.5,287.5,100,-10,10,100,-10,10);
  htracks_towers[0]->GetXaxis()->SetTitle("towers");
  htracks_towers[0]->GetYaxis()->SetTitle("X difference");
  htracks_towers[0]->GetZaxis()->SetTitle("Y difference");

  htracks_towers[1] = new TH3D("htracks_towers_arm1","different between tracks and towers arm 1",288,-0.5,287.5,100,-10,10,100,-10,10);
  htracks_towers[1]->GetXaxis()->SetTitle("towers");
  htracks_towers[1]->GetYaxis()->SetTitle("X difference");
  htracks_towers[1]->GetZaxis()->SetTitle("Y difference");

  htracks_angle[0] = new TH2D("htracks_angle_arm0","track angle distribution arm 0",50,-3.15,3.15,200,0,0.2);
  htracks_angle[0]->GetXaxis()->SetTitle("phi");
  htracks_angle[0]->GetYaxis()->SetTitle("theta");
  
  htracks_angle[1] = new TH2D("htracks_angle_arm1","track angle distribution arm 1",50,-3.15,3.15,200,0,0.2);
  htracks_angle[1]->GetXaxis()->SetTitle("phi");
  htracks_angle[1]->GetYaxis()->SetTitle("theta");



  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(!hm){
    hm = new Fun4AllHistoManager("DataAna");
    se->registerHistoManager(hm);
  }

  
  hm->registerHisto(htracks_vs_towers[0]);
  hm->registerHisto(htracks_vs_towers[1]);
  hm->registerHisto(htracks_towers[0]);
  hm->registerHisto(htracks_towers[1]);
  hm->registerHisto(htracks_angle[0]);
  hm->registerHisto(htracks_angle[1]);
  
  for(unsigned int arm = 0;arm < 2;arm++){
    char name[100];
    for(unsigned int layer = 0;layer < 8;layer++){
      sprintf(name,"hmpcex_nxy_freq_layer%d_arm%d",layer,arm);
      int Nx = 198*2;
      float ScaleX = 1;
      int Ny = 24*2;
      float ScaleY = 8;
      if(layer%2 == 1){
        Nx = 24*2;
	ScaleX = 8;
	Ny = 198*2;
	ScaleY = 1;
      }
      hmpcex_nxy_freq[arm][layer] = new TH2D(name,name,Nx,-0.5*ScaleX,(Nx-0.5)*ScaleX,Ny,-0.5*ScaleY,(Ny-0.5)*ScaleY);
      sprintf(name,"hmpcex_nxy_q_layer%d_arm%d",layer,arm);
      hmpcex_nxy_q[arm][layer] = new TH2D(name,name,Nx,-0.5*ScaleX,(Nx-0.5)*ScaleX,Ny,-0.5*ScaleY,(Ny-0.5)*ScaleY);
      hm->registerHisto(hmpcex_nxy_freq[arm][layer]);
      hm->registerHisto(hmpcex_nxy_q[arm][layer]);

      sprintf(name,"hhits_angle_arm%d_layer%d",arm,layer);
      hhits_angle[arm][layer] = new TH2D(name,name,50,-3.15,3.15,200,0,0.2);
      hhits_angle[arm][layer]->GetXaxis()->SetTitle("phi");
      hhits_angle[arm][layer]->GetYaxis()->SetTitle("theta");
      hm->registerHisto(hhits_angle[arm][layer]);
    }
  }

  
  hshower_hits[0] = new TH3D("hshower_hits_arm0","shower hits in arm 0",100,0,2,100,0,2,100,2,4);
  hshower_hits[0]->GetYaxis()->SetTitle("Energy ratio");
  hshower_hits[0]->GetXaxis()->SetTitle("Hits ratio");
  hshower_hits[0]->GetZaxis()->SetTitle("Energy/GeV");
  hm->registerHisto(hshower_hits[0]);

  hshower_hits[1] = new TH3D("hshower_hits_arm1","shower hits in arm 1",100,0,2,100,0,2,100,2,4);
  hshower_hits[1]->GetYaxis()->SetTitle("Energy ratio");
  hshower_hits[1]->GetXaxis()->SetTitle("Hits ratio");
  hshower_hits[1]->GetZaxis()->SetTitle("eta");
  hm->registerHisto(hshower_hits[1]);


  hshower_hxy_match[0] = new TH3D("hmyshower_hxy_match_arm0","shower match in hough space arm 0",100,-1,1,100,-1,1,100,0,1);
  hshower_hxy_match[0]->GetXaxis()->SetTitle("hsx match");
  hshower_hxy_match[0]->GetYaxis()->SetTitle("hsy match");
  hshower_hxy_match[0]->GetZaxis()->SetTitle("hsr match");
  hm->registerHisto(hshower_hxy_match[0]);

  hshower_hxy_match[1] = new TH3D("hmyshower_hxy_match_arm1","shower match in hough space arm 1",100,-1,1,100,-1,1,100,0,1);
  hshower_hxy_match[1]->GetXaxis()->SetTitle("hsx match");
  hshower_hxy_match[1]->GetYaxis()->SetTitle("hsy match");
  hshower_hxy_match[1]->GetZaxis()->SetTitle("hsr match");
  hm->registerHisto(hshower_hxy_match[1]);


//read the hot channel
  ifstream myfile ("hot_channel.txt");
  if(myfile.is_open()){
    while(!myfile.eof()){
      unsigned int arm;
      unsigned int layer;
      unsigned int nx;
      unsigned int ny;
      myfile >> arm >> layer >> nx >> ny;
      unsigned int index =  nx+198*ny+198*6*4*layer+198*4*6*8*arm;
      if(layer%2 == 1){ 
        index = ny+198*nx+198*6*4*layer+198*4*6*8*arm;
      }
      _hot_channel.insert(index);
      if(myfile.eof())break;
    }
  }
  else cout<<"open hot channel file failed"<<endl;
  return EVENT_OK;
}

int mMpcExMyShower::InitRun(PHCompositeNode* topNode){
  set_interface_ptrs(topNode);
  return EVENT_OK;
}


//sort function
bool sort_by_q(pair<TMpcExHit*,double> hit1,pair<TMpcExHit*,double> hit2){
  double q1 = hit1.second;
  double q2 = hit2.second;
  return (q2 < q1);
}

int mMpcExMyShower::process_event(PHCompositeNode* topNode){
  if(!_evt_quality->IsEventWanted()) return EVENT_OK;

//vertex
  PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout && !phglobal){
    cout <<"No BbcOut or PHGlobal !!!"<<endl;
    exit(1);
  }
  _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();

  find_tracks2();
  make_shower();
//  shower_display();
//  shower_test();
//  dead_hot_channel();
  myshower_test(topNode);
  return EVENT_OK;
}

int mMpcExMyShower::find_tracks2(){
//  cout<<"start showr2 "<<endl;   
//reset important

  peak_map_reset();

  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  //each hits associate with a peak;
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      std::map<unsigned int,unsigned int> hit_to_peak_map;
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = mpcex_hitmap.get_layer_first(arm,layer+1);
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
        unsigned int index = mpcex_hitmap.get_index(hit);
	//already used or useless 
	if(hit_to_peak_map.find(index) != hit_to_peak_map.end()) continue;
	
	double hit_q = get_good_q(hit,mpcex_hitmap);
//        if(hit_q > 300) cout<<"strange hit q : "<<hit_q<<endl;
        //the q is less than 0,then associate to peak 0,useless just ignore
	if(hit_q <= 0) {
	  hit_to_peak_map.insert(pair<unsigned int,unsigned int>(index,0));
	  continue;
	}

	double max_q = hit_q;
	unsigned int max_index = index;
        int suround[4][2] = {{1,0},{-1,0},{0,1},{0,-1}}; 

	vector<unsigned int> hits_in_peak;

	hits_in_peak.push_back(index);
	//loop until find peaks
	//counts the time of loops used
	int count = 0;
	MpcExPeakHits* find_peak = NULL;
	while(!find_peak){
          count++;
          
	  TMpcExHit* pre_hit = mpcex_hitmap.get_hit(index);
	  int nx = mpcex_hitmap.get_nx(pre_hit);
	  int ny = mpcex_hitmap.get_ny(pre_hit);
	  max_q = get_good_q(pre_hit,mpcex_hitmap);
//	  print_surround(pre_hit,mpcex_hitmap,1);
          
	  double q_surround[4] = {-9999,-9999,-9999,-9999};
	  unsigned int index_surround[4] = {0,0,0,0};
	  for(int i = 0;i < 4;i++){
	    //if nx or ny is out of range will return 0
            unsigned int index0 = mpcex_hitmap.get_index(arm,layer,nx+suround[i][0],ny+suround[i][1]);
	    //if index0 is invalid, hit0 will be Null;
	    TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
	    //if hit0 is Null, q will -9999.9;
	    double hit0_q = get_good_q(hit0,mpcex_hitmap);
//	    cout<<"hit0_q "<<hit0_q<<endl;
	    if(hit0_q > max_q){
              max_q = hit0_q;
	      max_index = index0;
	      q_surround[i] = hit0_q;
              index_surround[i] = index0;
	    }
	  }//i

          //prefer closer direction,the layer is the x type and x axis is the closer direction 
          if(layer%2 == 0 && ((q_surround[0] > 0) || (q_surround[1] > 0))){
            if(q_surround[0] > q_surround[1]){
              max_q = q_surround[0];
	      max_index = index_surround[0];
	    }
	    else {
              max_q = q_surround[1];
	      max_index = index_surround[1];
	    }
	  }

          //the layer is the y type and y axis is the closer direction 
          if(layer%2 == 1 && ((q_surround[2] > 0) || (q_surround[3] > 0))){
            if(q_surround[2] > q_surround[3]){
              max_q = q_surround[2];
	      max_index = index_surround[2];
	    }
	    else {
              max_q = q_surround[3];
	      max_index = index_surround[3];
	    }
	  }

//          cout<<"max q: "<<max_q<<endl;
	  //find the peak
	  if(max_index == index){ 
//	  cout<<"find the peak"<<endl;
          //check if the peak already exists
	    map<unsigned int,unsigned int>:: iterator hit_to_peak_map_iter = hit_to_peak_map.find(max_index); 
          //not exists create the peak
	    if(hit_to_peak_map_iter == hit_to_peak_map.end()){ 
//              hit_to_peak_map.insert(pair<unsigned int,unsigned int>(index,index));
	      MpcExPeakHits* mpcex_peak_hits = new MpcExPeakHits();
	      mpcex_peak_hits->arm = arm;
	      mpcex_peak_hits->layer = layer;
	      mpcex_peak_hits->peak_index = max_index;
	      TMpcExHit* peak_hit = mpcex_hitmap.get_hit(max_index);
              mpcex_peak_hits->peak_hsx = peak_hit->hsx(_vertex);
	      mpcex_peak_hits->peak_hsy = peak_hit->hsy(_vertex);
	      mpcex_peak_hits->peak_nx = mpcex_hitmap.get_nx(peak_hit);
	      mpcex_peak_hits->peak_ny = mpcex_hitmap.get_ny(peak_hit);
	      mpcex_peak_hits->peak_q = get_good_q(peak_hit,mpcex_hitmap);
	      mpcex_peak_hits->peak_z = peak_hit->z();
	      mpcex_peak_hits->tot_q = 0;
//              mpcex_peak_hits->push_back(max_index);    
	      _peaks_map[arm][layer].insert(pair<unsigned int,MpcExPeakHits*>(index,mpcex_peak_hits));
	     
//	      cout <<"new peak: q "<<  get_good_q(peak_hit)<<endl;
	      find_peak = mpcex_peak_hits;
	    }
	    else{//peak already exist
//	      cout <<" peak already created !!!"<<endl;
              unsigned int peak_index = hit_to_peak_map_iter->second;
//	      cout <<"the created peak index: "<<peak_index<<endl;
	      map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].find(peak_index);
	      // the peak should exsit
	      if(pk_map_iter == _peaks_map[arm][layer].end()){
                cout <<"something strange happened "<<endl;
		return EVENT_OK;
	      }
//	      cout <<"peak already created: max q "<<max_q<<endl; 
	      find_peak = pk_map_iter->second;
	    }
	  }
	  else {//peak not find
	    //search if the max_hit has alread has a peak belong to 
//	    cout<<"the hit is not peak "<<endl;
	    map<unsigned int,unsigned int>:: iterator hit_to_peak_map_iter = hit_to_peak_map.find(max_index); 
	    if(hit_to_peak_map_iter == hit_to_peak_map.end()){//no peak belong to 
//              cout<<"No belonging peak found for this hit:  max_q "<<max_q<<endl;
	      hits_in_peak.push_back(max_index);
	      index = max_index;
	    }
	    else{//the peak has already has a peak belong to
  //            cout <<"found the peak belonging to"<<endl;
	      unsigned int peak_index = hit_to_peak_map_iter->second;
//	      cout<<"the found peak index: "<<peak_index<<endl;
              map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].find(peak_index);
//	      cout<<"the hit q:  "<<max_q<<endl;
	      if(pk_map_iter == _peaks_map[arm][layer].end()){
                cout<<"something strange happened"<<endl;
		return EVENT_OK;
	      }
              find_peak = pk_map_iter->second;
	    }
	  }
          if(count > 200){
            cout <<"the nubmer of loop is bigger than 200,  something strange may happen"<<endl;
	    return EVENT_OK;
	  }
	}//while
        
	//insert the hits
	//size = 1, the hit has already inserted in creating process
//	cout<<"there are "<<hits_in_peak.size()<<" in hits in peak"<<endl;
	for(unsigned int ii = 0;ii < hits_in_peak.size();ii++){
          find_peak->push_back(hits_in_peak[ii]);
	  TMpcExHit* ii_pk_hit = mpcex_hitmap.get_hit(hits_in_peak[ii]);
	  double god_q = get_good_q(ii_pk_hit,mpcex_hitmap);
	  double ii_hit_hsx = ii_pk_hit->hsx(_vertex);
	  double ii_hit_hsy = ii_pk_hit->hsy(_vertex);
	  //average hsx,hsy
	  find_peak->avg_hsx += god_q*ii_hit_hsx; 
	  find_peak->avg_hsy += god_q*ii_hit_hsy;
	  //average hsx^2,hsy^2
	  find_peak->avg_hsx2 += god_q*ii_hit_hsx*ii_hit_hsx;
	  find_peak->avg_hsy2 += god_q*ii_hit_hsy*ii_hit_hsy;
	  
	  find_peak->tot_q += god_q;
	  

//	  cout<<"push back the hit "<<hits_in_peak[ii]<<endl;
	  unsigned int find_peak_index = find_peak->peak_index;
//	  cout<<"peak index: "<<find_peak_index<<endl;
	  hit_to_peak_map.insert(pair<unsigned int,unsigned int>(hits_in_peak[ii],find_peak_index));
	} 
      }//iter
    }//layer
  }//arm


//set hsx,hsy,hsx^2,hsy^2
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
	MpcExPeakHits* pk_hits = pk_map_iter->second;
	
	pk_hits->avg_hsx = (pk_hits->avg_hsx)/(pk_hits->tot_q);
      	pk_hits->avg_hsy = (pk_hits->avg_hsy)/(pk_hits->tot_q);

	pk_hits->avg_hsx2 = (pk_hits->avg_hsx2)/(pk_hits->tot_q);
      	pk_hits->avg_hsy2 = (pk_hits->avg_hsy2)/(pk_hits->tot_q);

      }
    }
  }

  //debuging code
 /* 
  cout<<"print the peak result: "<<endl;
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      cout<<"arm "<<arm<<" layer "<<layer<<endl;
      //ignore north, for database issue
//      if(arm == 1) continue;
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
        MpcExPeakHits* pk_hits = pk_map_iter->second;
	if(pk_hits->size() > 1){
	  print_mpcex_peak_hits(pk_hits,mpcex_hitmap);
	}
      }
    }
  }
  */
  //debugging code

 //associate this peaks according to hsx,hsy
  set<unsigned int>used_peak_list;
  track_list_reset();  
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
	MpcExPeakHits* pk_hits = pk_map_iter->second;
	if(used_peak_list.find(pk_hits->peak_index) != used_peak_list.end()) continue;
        MpcExMyTrack* mytrack = new MpcExMyTrack();
	mytrack->arm = arm;
	mytrack->tot_q = pk_hits->tot_q;
	//push back the hit 
	for(unsigned int ihit = 0;ihit < pk_hits->size();ihit++){
	  mytrack->push_back((*pk_hits)[ihit]);
	}
	mytrack->fired_layers = 0;
	mytrack->fired_layers++;

        double weight_x = 8*pk_hits->tot_q;
	double weight_y = pk_hits->tot_q;
	if(layer%2 == 1){
          weight_x = pk_hits->tot_q;
	  weight_y = 8*pk_hits->tot_q;
	}

	mytrack->hsx = (pk_hits->avg_hsx)*weight_x;
	mytrack->hsy = (pk_hits->avg_hsy)*weight_y;
	mytrack->hsx2 = (pk_hits->avg_hsx2)*weight_x;
	mytrack->hsy2 = (pk_hits->avg_hsy2)*weight_y;

	mytrack->norm_x = weight_x;
	mytrack->norm_y = weight_y;
	
	//check the same type layer first
	for(unsigned ilayer = layer+2 ;ilayer<8;ilayer += 2){
          if(_peaks_map[arm][ilayer].size() == 0) continue;
	  map<unsigned int,MpcExPeakHits*>::iterator ipk_map_iter = _peaks_map[arm][ilayer].begin();  
	  map<unsigned int,MpcExPeakHits*>::const_iterator ipk_map_end = _peaks_map[arm][ilayer].end();
	  double good_dsxy = 9999;
	  MpcExPeakHits* good_pk_hits = NULL;
	  //find the closest 
	  for(;ipk_map_iter!=ipk_map_end;ipk_map_iter++){
	    MpcExPeakHits* ipk_hits = ipk_map_iter->second;
	    if(used_peak_list.find(ipk_hits->peak_index) != used_peak_list.end()) continue;
	    
	    double z = ipk_hits->peak_z;
	    double ihsx = ipk_hits->avg_hsx;
	    double ihsy = ipk_hits->avg_hsy;
	    double hsx = (mytrack->hsx)/(mytrack->norm_x);
	    double hsy = (mytrack->hsy)/(mytrack->norm_y);
	    double dhsx = fabs(ihsx-hsx);
	    double dhsy = fabs(ihsy-hsy);

	    //x direction
	    if(layer%2 == 0 && (dhsy < 1.50/fabs(z-_vertex)) && (dhsx < 0.31/fabs(z-_vertex))){
	      if(good_dsxy > dhsx){
                good_dsxy = dhsx;
		good_pk_hits = ipk_hits;
	      }
	    }
	    if(layer%2 == 1 && (dhsx < 1.50/fabs(z-_vertex)) && (dhsy < 0.31/fabs(z-_vertex))){
	      if(good_dsxy > dhsy){
                good_dsxy = dhsy;
		good_pk_hits = ipk_hits;
	      }
	    }
	  }
          
	  if(good_pk_hits){
            used_peak_list.insert(good_pk_hits->peak_index);
	    for(unsigned int ihit = 0;ihit < good_pk_hits->size();ihit++){
	      mytrack->push_back((*good_pk_hits)[ihit]);
	    }
	    mytrack->tot_q += good_pk_hits->tot_q;
	    mytrack->fired_layers++;
	    
	    
	    weight_x = 8*good_pk_hits->tot_q;
	    weight_y = good_pk_hits->tot_q;
	    
	    if(layer%2 == 1){
              weight_x = good_pk_hits->tot_q;
	      weight_y = 8*good_pk_hits->tot_q;
	    }
	      
	    mytrack->hsx += weight_x*(good_pk_hits->avg_hsx);
	    mytrack->hsy += weight_y*(good_pk_hits->avg_hsy);

	    mytrack->hsx2 += weight_x*(good_pk_hits->avg_hsx2);
	    mytrack->hsy2 += weight_y*(good_pk_hits->avg_hsy2);
	      
	    mytrack->norm_x += weight_x;
	    mytrack->norm_y += weight_y;
	  }
	}//ilayer
        //check different type layer 
    	for(unsigned ilayer = layer+1 ;ilayer<8;ilayer += 2){
          if(_peaks_map[arm][ilayer].size() == 0) continue;
	  map<unsigned int,MpcExPeakHits*>::iterator ipk_map_iter = _peaks_map[arm][ilayer].begin();  
	  map<unsigned int,MpcExPeakHits*>::const_iterator ipk_map_end = _peaks_map[arm][ilayer].end();
	  double good_dsxy = 9999;
	  MpcExPeakHits* good_pk_hits = NULL;
	  for(;ipk_map_iter!=ipk_map_end;ipk_map_iter++){
	    MpcExPeakHits* ipk_hits = ipk_map_iter->second;
	    if(used_peak_list.find(ipk_hits->peak_index) != used_peak_list.end()) continue;
	    
	    double z = ipk_hits->peak_z;
	    double ihsx = ipk_hits->avg_hsx;
	    double ihsy = ipk_hits->avg_hsy;
	    double hsx = (mytrack->hsx)/(mytrack->norm_x);
	    double hsy = (mytrack->hsy)/(mytrack->norm_y);
	    double dhsx = fabs(ihsx-hsx);
	    double dhsy = fabs(ihsy-hsy);

	    //x direction
	    if((dhsy < 0.745/fabs(z-_vertex)) && (dhsx < 0.745/fabs(z-_vertex))){
	      if(good_dsxy > sqrt(dhsx*dhsx+dhsy*dhsy)){
                good_dsxy = sqrt(dhsx*dhsx+dhsy*dhsy);
		good_pk_hits = ipk_hits;
	      }
	    }
	  }
	  if(good_pk_hits){
            used_peak_list.insert(good_pk_hits->peak_index);
	    for(unsigned int ihit = 0;ihit < good_pk_hits->size();ihit++){
	      mytrack->push_back((*good_pk_hits)[ihit]);
	    }
	    mytrack->tot_q += good_pk_hits->tot_q;
	    mytrack->fired_layers++; 
            
	    weight_x = good_pk_hits->tot_q;
	    weight_y = 8*(good_pk_hits->tot_q);

	    if(layer%2 == 1){
              weight_x = 8*(good_pk_hits->tot_q);
	      weight_y = good_pk_hits->tot_q;
	    }
	    
            mytrack->hsx += weight_x*(good_pk_hits->avg_hsx);
	    mytrack->hsy += weight_y*(good_pk_hits->avg_hsy);

            mytrack->hsx2 += weight_x*(good_pk_hits->avg_hsx2);
	    mytrack->hsy2 += weight_y*(good_pk_hits->avg_hsy2);

	    mytrack->norm_x += weight_x;
	    mytrack->norm_y += weight_y;

	  }
        }//ilayer
       _track_list[arm].push_back(mytrack);
      }//pk_map_iter
    }//layer
  }//arm


//set the hsx,hsx^2,hsy,hsy^2
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int i = 0; i < _track_list[arm].size();i++){
      MpcExMyTrack* mytrack = _track_list[arm][i];
      mytrack->hsx = (mytrack->hsx)/(mytrack->norm_x);
      mytrack->hsy = (mytrack->hsy)/(mytrack->norm_y);
      
      mytrack->hsx2 = (mytrack->hsx2)/(mytrack->norm_x);
      mytrack->hsy2 = (mytrack->hsy2)/(mytrack->norm_y);
     
      //rms
      double hsx = mytrack->hsx;
      double hsy = mytrack->hsy;
      double hsx2 = mytrack->hsx2;
      double hsy2 = mytrack->hsy2;

      if(hsx2 > hsx*hsx) mytrack->hsx_rms = sqrt(hsx2-hsx*hsx);
      else mytrack->hsx_rms = 0;

      if(hsy2 > hsy*hsy) mytrack->hsy_rms = sqrt(hsy2-hsy*hsy);
      else mytrack->hsy_rms = 0;
    }
  }


//debugging code
/*
cout<<"draw the graph"<<endl;
  for(int arm = 0;arm < 2;arm++){
    if(grammy[arm]) delete grammy[arm];
    char name[50];
    sprintf(name,"grammy_Arm_%d",arm);
    grammy[arm] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
    cout<<"Number of tracks in arm "<<arm<<" "<<_track_list[arm].size()<<endl;
    int size = _track_list[arm].size();
    double step = 100./size;
    for(unsigned int i = 0;i < _track_list[arm].size();i++){
      MpcExMyTrack* mytrack = _track_list[arm][i];
      if(mytrack->fired_layers <= 2 ) continue;
//      cout <<"number of peaks: "<<track.size()<<endl;
//      cout <<"Number of peaks "<<track.size()<<endl;
      for(unsigned int j = 0;j < mytrack->size();j++){
	TMpcExHit* hit = mpcex_hitmap.get_hit((*mytrack)[j]);
	double weight = step*i;
	weight = get_good_q(hit,mpcex_hitmap);
	grammy[arm]->FillEx(hit->key(),weight);
      }
    }
  }//arm

  TCanvas* cg0 = new TCanvas("cgrammy_arm0","cgrammy_arm0",1500,800);
  cg0->Divide(4,2);
  TCanvas* cg1 = new TCanvas("cgrammy_arm1","cgrammy_arm1",1500,800);
  cg1->Divide(4,2);

  TCanvas* cga = new TCanvas("cga","cga",1500,800);
  cga->Divide(2,1);
  cga->cd(1);
  grammy[0]->SetAxisRange(0,7,"z");
  grammy[0]->Project3D("yx")->DrawCopy("colz");
  cga->cd(2);
  grammy[1]->SetAxisRange(0,7,"z");
  grammy[1]->Project3D("yx")->DrawCopy("colz");


  for(int i = 0;i < 8;i++){
    cg0->cd(i+1);
    grammy[0]->DrawLayer(i,"colz");
    cg1->cd(i+1);
    grammy[1]->DrawLayer(i,"colz");
  }
*/
  return EVENT_OK;
}

bool sort_track_by_q(MpcExMyTrack* track1,MpcExMyTrack* track2){
  double q1 = track1->tot_q;
  double q2 = track2->tot_q;
  return (q1 > q2);
}

int mMpcExMyShower::make_shower(){
  _shower_list[0].clear();
  _shower_list[1].clear();

//order the track by q
  sort(_track_list[0].begin(),_track_list[0].end(),sort_track_by_q);
  sort(_track_list[1].begin(),_track_list[1].end(),sort_track_by_q);
  for(unsigned int arm = 0 ;arm < 2;arm++){
    set<unsigned int> used_tracks;
    for(unsigned int i = 0;i < _track_list[arm].size();i++){     
      if(used_tracks.find(i) != used_tracks.end()) continue;
      
      MpcExMyTrack* mytrack = _track_list[arm][i];
      unsigned int fired_layers = _track_list[arm][i]->fired_layers;
      unsigned int hits_num = _track_list[arm][i]->size();
//      cout<<" fired layers: "<<fired_layers<<" number of hits: "<<hits_num<<endl;
      if((fired_layers <=1) || (fired_layers == hits_num)) {
	continue;
      }
      
      //create the shower
      MpcExMyShower myshower;
 //     cout<<"make a shower "<<endl;
      myshower.arm = arm;
      myshower.tot_q = mytrack->tot_q;
      myshower.add_track(mytrack);
      used_tracks.insert(i);

      double hsx = mytrack->hsx;
      double hsy = mytrack->hsy;
      double hsx2 = mytrack->hsx2;
      double hsy2 = mytrack->hsy2;

      double hsx_rms = mytrack->hsx_rms;
      double hsy_rms = mytrack->hsy_rms;
      double norm_x = mytrack->norm_x;
      double norm_y = mytrack->norm_y;

//      cout<<"hsx_rms: "<<hsx_rms<<" hsy_rms: "<<hsy_rms<<endl;

      bool track_added = true; 
      while(track_added){
        int track_counts = 0;
        for(unsigned int j = 0;j < _track_list[arm].size();j++){
          if(used_tracks.find(j) != used_tracks.end()) continue;
	  MpcExMyTrack* track = _track_list[arm][j];
	  double hsx1 = track->hsx;
	  double hsy1 = track->hsy;
	  double norm_x1 = track->norm_x;
	  double norm_y1 = track->norm_y;
	  if((fabs(hsx-hsx1) < 3*hsx_rms) && (fabs(hsy-hsy1) < 3*hsy_rms)){
            hsx = hsx*norm_x + hsx1*norm_x1;
	    hsy = hsy*norm_y + hsy1*norm_y1;

	    hsx2 = hsx2*norm_x + norm_x1*(track->hsx2);
	    hsy2 = hsy2*norm_y + norm_y1*(track->hsy2);
	    norm_x += norm_x1;
	    norm_y += norm_y1;
	    hsx = hsx/norm_x;
	    hsy = hsy/norm_y;
            hsx2 = hsx2/norm_x;
	    hsy2 = hsy2/norm_y;
            
            hsx_rms = sqrt(hsx2-hsx*hsx);
	    hsy_rms = sqrt(hsy2-hsy*hsy);

  //          cout<<"insert tracks and new rms: "
  //              <<"hsx_rms: "<<hsx_rms<<" hsy_rms: "<<hsy_rms<<endl;


	    myshower.add_track(track);
	    myshower.tot_q += track->tot_q;
	    used_tracks.insert(j);
	    track_counts++;
	  }
	}
	if(track_counts == 0) track_added = false;
      }
      
      myshower.hsx_rms = hsx_rms;
      myshower.hsy_rms = hsy_rms;
      myshower.hsx = hsx;
      myshower.hsy = hsy;

      _shower_list[arm].push_back(myshower);

    }//track_list
  }//arm
  
  return EVENT_OK;  
}


int mMpcExMyShower::myshower_test(PHCompositeNode* topNode){
//get information of single photon  
  primaryWrapper* primary = getClass<primaryWrapper> (topNode, "primary");
  double px_prim = 0;
  double py_prim = 0;
  double pz_prim = 0;
  double p_prim = 0;
  double hsx_prim = 0;
  double hsy_prim = 0;
  double eta = 0;
  size_t nprim = 2;  
  if(primary!=NULL){
    for (size_t iprim=0; iprim<nprim; iprim++){
      int idparticle = primary->get_idpart(iprim);
      if(idparticle==1){  //photon
        px_prim = primary->get_px_momentum(iprim);
	py_prim = primary->get_py_momentum(iprim);
	pz_prim = primary->get_pz_momentum(iprim);
      }
    }
    p_prim = sqrt(px_prim*px_prim + py_prim*py_prim + pz_prim*pz_prim);
    eta = 1/2.*log((p_prim + pz_prim)/(p_prim - pz_prim));
    hsx_prim = px_prim/pz_prim*(-1);
    hsy_prim = py_prim/pz_prim;
  }


//mpc information
  int NMpcTowers = _mpc_tower_container->size();
  double mpc_hsx[2] = {0};
  double mpc_hsy[2] = {0};
  double mpc_norm[2] = {0};
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower < 0.0) continue;
    double tower_x = _mpc_map->getX(tow_ch);
    double tower_y = _mpc_map->getY(tow_ch);
    double tower_z = _mpc_map->getZ(tow_ch);
    mpc_hsx[arm] += tower_x/(tower_z - _vertex)*e_tower;
    mpc_hsy[arm] += tower_y/(tower_z - _vertex)*e_tower;
    mpc_norm[arm] += e_tower;
  }

  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  for(int arm = 0;arm < 2;arm++){
    double max_q = -1;
    unsigned int max_shower = 0;
    for(unsigned int i = 0;i < _shower_list[arm].size();i++){
      MpcExMyShower myshower = _shower_list[arm][i];
      double shower_q = myshower.tot_q;
      if(shower_q > max_q){
        max_q = shower_q;
	max_shower = i;
      }
    }
  
    if(max_q <0) continue;
    MpcExMyShower myshower = _shower_list[arm][max_shower];
    double Nshower_hits = 0;
    double Ntotal_hits = 0;
    double Eshower_mpcex = max_q;
    double Etotal_mpcex = 0;
    double hsx = myshower.hsx;
    double hsy = myshower.hsy;

    for(unsigned int j = 0; j < myshower.track_num();j++){
      MpcExMyTrack* mytrack = myshower.get_track(j);
      Nshower_hits += (double)mytrack->size();
    }
    
    double hsx_t = 0;
    double hsy_t = 0;
    double normx_t = 0;
    double normy_t = 0;
    MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,0);
    MpcExHitMap::const_iterator iter_end = mpcex_hitmap.get_layer_first(arm+1,0);
    for(;iter != iter_end;iter++){
      TMpcExHit* hit = iter->second;
      double hit_q = get_good_q(hit,mpcex_hitmap); 
      if(hit_q < 0) continue;
      int layer = hit->layer();
      if(layer%2 == 0){
        hsx_t += hit->hsx(_vertex)*hit_q*8;
	hsy_t += hit->hsy(_vertex)*hit_q;
	normx_t += hit_q*8;
	normy_t += hit_q;
      }
      else{
        hsx_t += hit->hsx(_vertex)*hit_q;
	hsy_t += hit->hsy(_vertex)*hit_q*8;
	normx_t += hit_q;
	normy_t += hit_q*8;
      }
      Ntotal_hits += 1;
      Etotal_mpcex += hit_q;
    }
    
    hsx_t = hsx_t/normx_t;
    hsy_t = hsy_t/normy_t;

    double hsr = sqrt((hsx-hsx_t)*(hsx-hsx_t)+(hsy-hsy_t)*(hsy-hsy_t));
    mpc_hsx[arm] = mpc_hsx[arm]/mpc_norm[arm];
    mpc_hsy[arm] = mpc_hsy[arm]/mpc_norm[arm];


    hshower_hits[arm]->Fill(Nshower_hits/Ntotal_hits,Eshower_mpcex/Etotal_mpcex,fabs(eta));
    hshower_hxy_match[arm]->Fill(hsx - hsx_prim,hsy - hsy_prim,hsr);
  }//arm
 
  return EVENT_OK;
}

void mMpcExMyShower::shower_display(){
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  for(int arm = 0;arm < 2;arm++){
    if(grammy[arm]) delete grammy[arm];
    char name[50];
    sprintf(name,"grammy_Arm_%d",arm);
    grammy[arm] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
    cout<<"Number of showers in arm "<<arm<<" "<<_shower_list[arm].size()<<endl;
    int size = _shower_list[arm].size();
    double step = 100./size;
    for(unsigned int i = 0;i < _shower_list[arm].size();i++){
      MpcExMyShower myshower = _shower_list[arm][i]; 
      cout<<"Number of tracks in shower "<<i<<" : "<<myshower.track_num()<<endl;
      for(unsigned int j = 0; j < myshower.track_num();j++){
        MpcExMyTrack* mytrack = myshower.get_track(j);;
        for(unsigned int j = 0;j < mytrack->size();j++){
	  TMpcExHit* hit = mpcex_hitmap.get_hit((*mytrack)[j]);
	  double weight = step*(i+1);
//	  weight = get_good_q(hit,mpcex_hitmap);
	  grammy[arm]->FillEx(hit->key(),weight);
        }
      }
    }
  }//arm

  TCanvas* cg0 = new TCanvas("cgrammy_arm0","cgrammy_arm0",1500,800);
  cg0->Divide(4,2);
  TCanvas* cg1 = new TCanvas("cgrammy_arm1","cgrammy_arm1",1500,800);
  cg1->Divide(4,2);

  TCanvas* cga = new TCanvas("cga","cga",1500,800);
  cga->Divide(2,1);
  cga->cd(1);
  grammy[0]->SetAxisRange(0,7,"z");
  grammy[0]->Project3D("yx")->DrawCopy("colz");
  cga->cd(2);
  grammy[1]->SetAxisRange(0,7,"z");
  grammy[1]->Project3D("yx")->DrawCopy("colz");

  for(int i = 0;i < 8;i++){
    cg0->cd(i+1);
    grammy[0]->DrawLayer(i,"colz");
    cg1->cd(i+1);
    grammy[1]->DrawLayer(i,"colz");
  }

}

int mMpcExMyShower::shower_test(){
  int NMpcTowers = _mpc_tower_container->size();
  int fired_towers[2] = {0,0};
  for(int itower = 0; itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    int arm = _mpc_map->getArm(tow_ch);
    double e_tower = ctwr->get_energy();
    if(e_tower > 0.3) fired_towers[arm]++;
  }

  int good_tracks[2] = {0,0};
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int i = 0;i < _track_list[arm].size();i++){
      MpcExMyTrack* mytrack = _track_list[arm][i];
      if(mytrack->fired_layers <= 1) continue;
      good_tracks[arm]++;
      int good_tow_ch = -1;
      double dr = 9999;
      double dx = 9999;
      double dy = 9999;
      double zz = 9999;
      double hsx = mytrack->hsx;
      double hsy = mytrack->hsy;
      double phi = atan2(hsy,hsx);
      double theta = atan2(sqrt(hsx*hsx+hsy*hsy),1);

      htracks_angle[arm]->Fill(phi,theta);

      for(int itower = 0; itower < NMpcTowers;itower++){
        mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
        int tow_ch = ctwr->get_ch();
	unsigned int tarm = _mpc_map->getArm(tow_ch);
	if(tarm != arm) continue;
        double x = _mpc_map->getX(tow_ch);
	double y = _mpc_map->getY(tow_ch);
	double z = _mpc_map->getZ(tow_ch);
	zz = z;
	double thsx = x/(zz-_vertex);
	double thsy = y/(zz-_vertex);
	double dhsx = -(thsx - hsx);
	double dhsy = -(thsy - hsy);
	double tr = sqrt(dhsx*dhsx+dhsy*dhsy);
	if(dr > tr){
          dr = tr;
	  good_tow_ch = tow_ch;
	  dx = dhsx;
	  dy = dhsy;
	}
//	cout<<thsx<<" "<<thsy<<" "<<hsx<<" "<<hsy<<endl;
      }
      htracks_towers[arm]->Fill(good_tow_ch,dx*(zz-_vertex),dy*(zz-_vertex)); 
//      cout <<dx*zz<<" "<<dy*zz<<endl;
    }
  }//arm

  htracks_vs_towers[0]->Fill(fired_towers[0],good_tracks[0]);
  htracks_vs_towers[1]->Fill(fired_towers[1],good_tracks[1]);

  return EVENT_OK;  
}

int mMpcExMyShower::dead_hot_channel(){
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      std::map<unsigned int,unsigned int> hit_to_peak_map;
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = mpcex_hitmap.get_layer_first(arm,layer+1);
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
	double hit_q = get_good_q(hit,mpcex_hitmap);
	unsigned int nx = mpcex_hitmap.get_nx(hit);
	unsigned int ny = mpcex_hitmap.get_ny(hit);
        unsigned int index = mpcex_hitmap.get_index(hit);
	double hsx = hit->hsx(_vertex);
	double hsy = hit->hsy(_vertex);
	if(hit_q > 0 && (_hot_channel.find(index) == _hot_channel.end())){
	  double hit_freq = hmpcex_nxy_freq[arm][layer]->GetBinContent(2*nx+1+1,2*ny+1+1);
          double tot_q =  hmpcex_nxy_q[arm][layer]->GetBinContent(2*nx+1+1,2*ny+1+1);
	  hmpcex_nxy_freq[arm][layer]->SetBinContent(2*nx+1+1,2*ny+1+1,1+hit_freq);
	  hmpcex_nxy_q[arm][layer]->SetBinContent(2*nx+1+1,2*ny+1+1,tot_q+hit_q);
	  double phi = atan2(hsy,hsx);
	  double theta = atan2(sqrt(hsx*hsx+hsy*hsy),1);
	  hhits_angle[arm][layer]->Fill(phi,theta);
	}
      }
    }
  }
  return EVENT_OK;
}

void mMpcExMyShower::peak_map_reset(){
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      while(!_peaks_map[arm][layer].empty()){
        map<unsigned int,MpcExPeakHits*>::iterator iter = _peaks_map[arm][layer].begin();
	delete iter->second;
	_peaks_map[arm][layer].erase(iter);
      }
    }
  }
}

void mMpcExMyShower::track_list_reset(){
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int i = 0;i < _track_list[arm].size();i++){
      if(_track_list[arm][i]) delete _track_list[arm][i];
    }
    _track_list[arm].clear();
  }
}

void mMpcExMyShower::print_mpcex_peak_hits(const MpcExPeakHits* pk_hits,const MpcExHitMap& mpcex_hitmap) const{
  if(!pk_hits) return;
  unsigned int index = pk_hits->peak_index;
  TMpcExHit* hit = mpcex_hitmap.get_hit(index);
  cout<<"----------------------"<<endl;
  cout<<"Number of hits: "<<pk_hits->size()<<endl;
  for(unsigned int i = 0;i < pk_hits->size();i++){
    unsigned int hits_index = (*pk_hits)[i];
    TMpcExHit* hit0 = mpcex_hitmap.get_hit(hits_index);
    double q = get_good_q(hit0,mpcex_hitmap);
    cout<<q<<endl;
  }
  print_surround(hit,mpcex_hitmap,4);
  cout<<"----------------------"<<endl;
}

void mMpcExMyShower::print_surround(const TMpcExHit* hit,const MpcExHitMap& mpcex_hitmap,int range ) const{
  if(!hit) {
    cout <<"invalid hit "<<endl;
    return;
  }
  int nx = mpcex_hitmap.get_nx(hit);
  int ny = mpcex_hitmap.get_ny(hit);
  unsigned int arm = hit->arm();
  unsigned int layer = hit->layer();
  unsigned index = mpcex_hitmap.get_index(hit);
  cout <<"peak hit : "<<get_good_q(hit,mpcex_hitmap) <<" nx "<<nx<<" ny "<<ny<<" "<<arm<<" "<<layer<<" "<<index<<endl;
  for(int ilayer = 0;ilayer < 8;ilayer++){
  cout<<"layer: "<<ilayer<<endl;
    for(int i = ny+range;i >= ny-range;i--){
      for(int j = nx-range;j<= nx+range;j++){
//      cout <<i<<" "<<j<<endl;
        unsigned int index0 = mpcex_hitmap.get_index(arm,ilayer,j,i);
//      cout<<index0<<endl;
        TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
        double hit_q = get_good_q(hit0,mpcex_hitmap);
        if(hit_q > 0) cout<<setfill(' ')<<setw(6)<<setprecision(5)<<hit_q<<" ";
        else cout<<"000.00 ";
      }
      cout <<endl;
    }
  }
}


//calculate the distance in hough space between different Minipad;
double mMpcExMyShower::get_hough_dist(const TMpcExHit* hit1,const TMpcExHit* hit2) const{
  double hsx1 = hit1->hsx(_vertex);
  double hsy1 = hit1->hsy(_vertex);
  double hsx2 = hit2->hsx(_vertex);
  double hsy2 = hit2->hsy(_vertex);
  double rx2 = (hsx1-hsx2)*(hsx1-hsx2);
  double ry2 = (hsy1-hsy2)*(hsy1-hsy2);
  return sqrt(rx2+ry2);
}


//if the hit is the maximum or not
bool mMpcExMyShower::is_peak(const TMpcExHit* hit,const MpcExHitMap& mpcex_hitmap) const{
  if(!hit) return false;
  int index = mpcex_hitmap.get_index(hit);
  unsigned int layer = hit->layer();
  unsigned int arm = hit->arm();
  int index_down = 198*6*4*layer+198*4*6*8*arm;
  int index_up =  198*6*4*(layer+1)+198*4*6*8*arm;
  double hit_q = get_good_q(hit,mpcex_hitmap);
  //bad q
  if(hit_q <=0) return false;
 
  //loop over the surround minipad 
  int index_list[4] ={-1,1,-198,198}; 
  for(int i = 0;i < 4;i++){
    int index2 = index+index_list[i];
    if((index2 > index_down) && (index2 < index_up)){
      TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2);
      if(hit2 && (get_good_q(hit2,mpcex_hitmap) > hit_q)) return false;
    }
  }
 
  return true;
}

double mMpcExMyShower::get_good_q(const TMpcExHit* hit,const MpcExHitMap& mpcex_hitmap) const{
  //the status can be changed due to the changing of software
  if(!hit) return -9999.9;
  if(hit->state_high()!= TMpcExHit::PEDESTAL_SUBTRACTED) return -9999.9;
  unsigned int index = mpcex_hitmap.get_index(hit);
  if((_hot_channel.find(index) != _hot_channel.end())) return -9999.9;
  
  unsigned int key = hit->key();
  double high_q = hit->high();
  double low_q = hit->low();

  TMpcExCalib* calib = _mpcex_calibs->get(key);
  double high_ped = calib->high_pedestal();
  double low_ped = calib->low_pedestal();

  double hit_q = -9999.9;
  if(high_q > 1.5 && (high_q+high_ped) < 230 ) hit_q = high_q;
  else if(low_q > 1.5 && (low_q+low_ped) < 254) hit_q = low_q*4.5; //assume high low ration is 4.5
 
  return hit_q;
}



int mMpcExMyShower::End(PHCompositeNode* topNode){

  return EVENT_OK;
}

void mMpcExMyShower::set_interface_ptrs(PHCompositeNode* topNode){
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
  _mpc_map = MpcMap::instance();
  if(!_mpc_map){
    cout <<"No MpcMap!!!"<<endl;
    exit(1);
  }

}
