#include "mMpcExMyShower.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "MpcExRawHit.h"
#include "MpcExEventQuality.h"
#include "Exogram.h"

#include "mpcClusterContainer.h"
#include "mpcTowerContainer.h"
#include "MpcMap.h"

#include "PHIODataNode.h"
#include "getClass.h"
#include "BbcOut.h"
#include "PHGlobal.h"

#include "Fun4AllReturnCodes.h"

#include "iostream"
#include "stdlib.h"

#include <algorithm>
#include <functional>
#include <TCanvas.h>

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
  }
}

int mMpcExMyShower::Init(PHCompositeNode* topNode){
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

  find_showers2();
  return EVENT_OK;
}

int mMpcExMyShower::find_showers2(){
  cout<<"start showr2 "<<endl;   
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
	//already 
	if(hit_to_peak_map.find(index) != hit_to_peak_map.end()) continue;
	
	double hit_q = get_good_q(hit);
//        if(hit_q > 300) cout<<"strange hit q : "<<hit_q<<endl;
        //the q is less than 0,then associate to peak 0
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
	  max_q = get_good_q(pre_hit);
//	  print_surround(pre_hit,mpcex_hitmap,1);
          
	  double q_surround[4] = {-9999,-9999,-9999,-9999};
	  unsigned int index_surround[4] = {0,0,0,0};
	  for(int i = 0;i < 4;i++){
	    //if nx or ny is out of range will return 0
            unsigned int index0 = mpcex_hitmap.get_index(arm,layer,nx+suround[i][0],ny+suround[i][1]);
	    //if index0 is invalid, hit0 will be Null;
	    TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
	    //if hit0 is Null, q will -9999.9;
	    double hit0_q = get_good_q(hit0);
//	    cout<<"hit0_q "<<hit0_q<<endl;
	    if(hit0_q > max_q){
              max_q = hit0_q;
	      max_index = index0;
	      q_surround[i] = hit0_q;
              index_surround[i] = index0;
	    }
	  }//i

          //prefer closer direction
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
	      mpcex_peak_hits->peak_q = get_good_q(peak_hit);
	      mpcex_peak_hits->peak_z = peak_hit->z();
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
//	  cout<<"push back the hit "<<hits_in_peak[ii]<<endl;
	  unsigned int find_peak_index = find_peak->peak_index;
//	  cout<<"peak index: "<<find_peak_index<<endl;
	  hit_to_peak_map.insert(pair<unsigned int,unsigned int>(hits_in_peak[ii],find_peak_index));
	} 
      }//iter
    }//layer
  }//arm

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
  typedef vector<MpcExPeakHits*> MyShower;
  vector<MyShower> shower_list[2];
  
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
	MpcExPeakHits* pk_hits = pk_map_iter->second;
	if(used_peak_list.find(pk_hits->peak_index) != used_peak_list.end()) continue;
	MyShower shower;
        shower.push_back(pk_hits);
	used_peak_list.insert(pk_hits->peak_index);
	double hsx = pk_hits->peak_hsx;
	double hsy = pk_hits->peak_hsy;
	for(unsigned ilayer = layer+1 ;ilayer<8;ilayer++){
          if(_peaks_map[arm][ilayer].size() == 0) continue;
	  map<unsigned int,MpcExPeakHits*>::iterator ipk_map_iter = _peaks_map[arm][ilayer].begin();  
	  map<unsigned int,MpcExPeakHits*>::const_iterator ipk_map_end = _peaks_map[arm][ilayer].end();
	  double good_dsxy = 9999;
	  MpcExPeakHits* good_pk_hits = NULL;
	  //find the closest 
	  for(;ipk_map_iter!=ipk_map_end;ipk_map_iter++){
            MpcExPeakHits* ipk_hits = ipk_map_iter->second;
	    if(used_peak_list.find(ipk_hits->peak_index) != used_peak_list.end()) continue;
	    double ihsx = ipk_hits->peak_hsx;
	    double ihsy = ipk_hits->peak_hsy;
	    double dhsx = fabs(ihsx-hsx);
	    double dhsy = fabs(ihsy-hsy);
	    double z = ipk_hits->peak_z;
	    //x type more accurate on x direction
	    if((layer%2 == 0) && (ilayer%2 == 0) && (dhsy < 1.62/fabs(z-_vertex)) && (dhsx < 0.31/fabs(z-_vertex))){
              if(good_dsxy > dhsx){
                good_dsxy = dhsx;
		good_pk_hits = ipk_hits;
	      }
	    }
            //y type more accurate on y direction
	    if((layer%2 == 1) && (ilayer%2 == 1) && (dhsy < 0.31/fabs(z-_vertex)) && (dhsx < 1.62/fabs(z-_vertex))){
              if(good_dsxy > dhsy){
                good_dsxy = dhsy;
		good_pk_hits = ipk_hits;
	      }
	    }
            
	    //x-y type, both are not accurate
	    if(((ilayer - layer)%2 == 1) && (dhsy < 0.96/fabs(z-_vertex)) && (dhsx < 0.96/fabs(z-_vertex))){
               double dr = sqrt(dhsx*dhsx+dhsy*dhsy);
	       if(good_dsxy > dr){
                 good_dsxy = dr;
		 good_pk_hits = ipk_hits;
	       }
	    }
	  }
	
	//find the required peak hit
	if(good_pk_hits){
          shower.push_back(good_pk_hits);
	  used_peak_list.insert(good_pk_hits->peak_index); 
	}

	}//ilayer
        shower_list[arm].push_back(shower);
      } 
    }//layer
  }//arm


//debugging code

  for(int arm = 0;arm < 2;arm++){
    if(grammy[arm]) delete grammy[arm];
    char name[50];
    sprintf(name,"grammy_Arm_%d",arm);
    grammy[arm] = new Exogram(name,name,900,-24,24,900,-24,24,8,-0.5,7.5);
    cout<<"Number of showers in arm "<<arm<<" "<<shower_list[arm].size()<<endl;
    int size = shower_list[arm].size();
    double step = 100./size;
    for(unsigned int i = 0;i < shower_list[arm].size();i++){
      MyShower shower = shower_list[arm][i];
      if(shower.size() <= 1 ) continue;
      cout <<"number of peaks: "<<shower.size()<<endl;
//      cout <<"Number of peaks "<<shower.size()<<endl;
      for(unsigned int j = 0;j < shower.size();j++){
        MpcExPeakHits* pk_hits = shower[j];
	if(pk_hits->size() > 0) {
	  for(unsigned int k = 0;k < pk_hits->size();k++){
            unsigned int index = (*pk_hits)[k];
	    TMpcExHit* hit = mpcex_hitmap.get_hit(index);
	    grammy[arm]->FillEx(hit->key(),step*i);
	  }
	}
      }
    }
  }

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

void mMpcExMyShower::print_mpcex_peak_hits(const MpcExPeakHits* pk_hits,const MpcExHitMap& mpcex_hitmap) const{
  if(!pk_hits) return;
  unsigned int index = pk_hits->peak_index;
  TMpcExHit* hit = mpcex_hitmap.get_hit(index);
  cout<<"----------------------"<<endl;
  cout<<"Number of hits: "<<pk_hits->size()<<endl;
  for(unsigned int i = 0;i < pk_hits->size();i++){
    unsigned int hits_index = (*pk_hits)[i];
    TMpcExHit* hit0 = mpcex_hitmap.get_hit(hits_index);
    double q = get_good_q(hit0);
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
  cout <<"peak hit : "<<get_good_q(hit) <<" nx "<<nx<<" ny "<<ny<<" "<<arm<<" "<<layer<<" "<<index<<endl;
  for(int i = ny+range;i >= ny-range;i--){
    for(int j = nx-range;j<= nx+range;j++){
//      cout <<i<<" "<<j<<endl;
      unsigned int index0 = mpcex_hitmap.get_index(arm,layer,j,i);
//      cout<<index0<<endl;
      TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
      double hit_q = get_good_q(hit0);
      if(hit_q > 0) cout<<setfill(' ')<<setw(6)<<setprecision(5)<<hit_q<<" ";
      else cout<<"000.00 ";
    }
    cout <<endl;
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
  double hit_q = get_good_q(hit);
  //bad q
  if(hit_q <=0) return false;
 
  //loop over the surround minipad 
  int index_list[4] ={-1,1,-198,198}; 
  for(int i = 0;i < 4;i++){
    int index2 = index+index_list[i];
    if((index2 > index_down) && (index2 < index_up)){
      TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2);
      if(hit2 && (get_good_q(hit2) > hit_q)) return false;
    }
  }
 
  return true;
}

double mMpcExMyShower::get_good_q(const TMpcExHit* hit) const{
  //the status can be changed due to the changing of software
  if(!hit) return -9999.9;
  if(hit->status()!= TMpcExHit::PEDESTAL_SUBTRACTED) return -9999.9;
  
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
