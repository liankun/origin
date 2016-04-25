#include "mMpcExMyShower.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "MpcExRawHit.h"
#include "MpcExEventQuality.h"

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

using namespace std;
using namespace findNode;



mMpcExMyShower::mMpcExMyShower(const char* name):
  SubsysReco(name)
{
  _vertex = -9999.0; 
}

mMpcExMyShower::~mMpcExMyShower(){

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

  find_showers1();

  return EVENT_OK;
}

int mMpcExMyShower::find_showers1(){
  cout<<"starting showers1"<<endl;
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      _peaks_list[arm][layer].clear();
    }
  }
  //find peaks 
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  std::set<unsigned int> used_list;
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int layer = 0;layer < 8;layer++){
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = mpcex_hitmap.get_layer_first(arm,layer+1);
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
        unsigned int index = mpcex_hitmap.get_index(hit);
//	cout <<"index: "<<index<<endl;
	if(used_list.find(index)!= used_list.end()) continue;
        if(is_peak(hit,mpcex_hitmap)){
	  // find the surround hits of the peak
//	  cout <<"insert in used_list: "<< index<<endl;
	  used_list.insert(index);
	  
//	  cout <<"print the hit matrix"<<endl;
//	  print_surround(hit,mpcex_hitmap);

	  int nx = mpcex_hitmap.get_nx(hit);
	  int ny = mpcex_hitmap.get_ny(hit);
	  unsigned int hit_arm = hit->arm();
	  if(hit_arm!=arm) cout<<"serious error: arm does not match"<<endl;
	  unsigned int hit_layer = hit->layer();
	  if(hit_layer!=layer) cout<<"serious error: layer does not match"<<endl;
	  //include the hits around the peak
	  MpcExPeakHits mpcex_peak_hits;
	  //0: x_pos,1:x_neg,2:y_pos,3:y_neg, 4 direction 
	  bool x_y[4] = {false,false,false,false};
	  int x_y_sign[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
	  int x_y_limit[4] = {0}; 
          MpcExPeakHits::Status stat= MpcExPeakHits::Mixed;
	  
	  mpcex_peak_hits.peak_hit = *hit;
	  mpcex_peak_hits.hsx = hit->hsx(_vertex);
	  mpcex_peak_hits.hsy = hit->hsy(_vertex);
	  mpcex_peak_hits.push_back(*hit);
	  mpcex_peak_hits.peak_q = get_good_q(hit);
	  
	  //100 should be enough
	  for(int i = 1;i < 100;i++ ){
            for(int j = 0;j < 4;j++){
	      if(!x_y[j]){
//	        cout <<"starting  "<<j<<endl;
                unsigned int index0 = mpcex_hitmap.get_index(arm,layer,nx+i*x_y_sign[j][0],ny+i*x_y_sign[j][1]);
	        TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
	        //the hit is Null, the hits peak should end here
		if(!hit0 || (get_good_q(hit0)<=0)) {
//	          cout <<"find NULL "<<j <<"direction "<<endl;
		  x_y[j] = true;
		  x_y_limit[j] = i-1;
		  if(used_list.find(index0) == used_list.end()){
		    used_list.insert(index0);
//		    cout <<"insert in used_list: "<<index0<<endl;
		  }
	        }
	        else{
		  //the hits is valid, check the next hits
                  unsigned int index1 = mpcex_hitmap.get_index(arm,layer,nx+(i+1)*x_y_sign[j][0],ny+(i+1)*x_y_sign[j][1]);  
		  TMpcExHit* hit1 = mpcex_hitmap.get_hit(index1);
                  //the hits is Null, the hits arround the peak should end at hit1
		  if(!hit1 || (hit1&& (get_good_q(hit1) <= get_good_q(hit0)))){                  
//		    cout<<"find good hit"<<endl;
//		    cout<<" q0  "<<get_good_q(hit0)<<" q1 "<<get_good_q(hit1)<<endl;
		    if(is_peak(hit0,mpcex_hitmap)) cout<<"this is a peak"<<endl;
		    if(used_list.find(index0) == used_list.end()){
//		      cout <<"not in used list "<<endl;
		      mpcex_peak_hits.push_back(*hit0);
		      used_list.insert(index0);
		    }     
		    else {
//		      cout <<"in used list"<<endl;
                      mpcex_peak_hits.status = stat;
		    //  x_y[j] = true;
      		    }
		  }
		  // the the q of hit2 is larger than q of hit1, the peak should end at hit1
		  else {
//                  cout << "the hit is smaller "<<endl;
		    x_y[j] = true;
		    x_y_limit[j] = i;
		    if(used_list.find(index0) == used_list.end()){
		      mpcex_peak_hits.push_back(*hit0);
		      used_list.insert(index0);
		    }
                    else mpcex_peak_hits.status = stat;
		  }
	        }
	      }
            }//j
	    if(x_y[0]&&x_y[1]&&x_y[2]&&x_y[3]) break;
	  }//i
          
	  
	  // search the surroundings not only in x or y direction
	  //0: x_pos,1:x_neg,2:y_pos,3:y_neg, 4 direction 
	  int x_y_pattern[4][2]={{x_y_limit[0],x_y_limit[2]},{x_y_limit[0],x_y_limit[3]},{x_y_limit[1],x_y_limit[3]},{x_y_limit[1],x_y_limit[2]}};   
	  int x_y_sign2[4][2] = {{1,1},{1,-1},{-1,-1},{-1,1}};
	  for(unsigned int ii = 0;ii < 4;ii++){
            for( int ix = 1;ix <= x_y_pattern[ii][0]+1;ix++){
              for( int iy = 1;iy <= x_y_pattern[ii][1]+1;iy++){
	        if( (ix == (x_y_pattern[ii][0]+1)) && (iy == (x_y_pattern[ii][1]+1)) ) continue;		
                unsigned int index0 = mpcex_hitmap.get_index(arm,layer,nx+ix*x_y_sign2[ii][0],ny+iy*x_y_sign2[ii][1]);
		TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
		if(!hit0 || (get_good_q(hit0) <=0)) continue;

		unsigned int index1 = mpcex_hitmap.get_index(arm,layer,nx+(ix-1)*x_y_sign2[ii][0],ny+iy*x_y_sign2[ii][1]);
                TMpcExHit* hit1 = mpcex_hitmap.get_hit(index1);
		unsigned int index2 = mpcex_hitmap.get_index(arm,layer,nx+ix*x_y_sign2[ii][0],ny+(iy-1)*x_y_sign2[ii][1]);
                TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2);


                if((hit0 && hit2 && (get_good_q(hit0) <= get_good_q(hit2))) || (hit0 && hit1 && (get_good_q(hit0) <= get_good_q(hit1))) ){
		  if(used_list.find(index0)==used_list.end()){
		      mpcex_peak_hits.push_back(*hit0);
		      used_list.insert(index0);
		  }
                  else mpcex_peak_hits.status = stat;
		}
	      }//iy
	    }//ix
	  }//ii

          if(mpcex_peak_hits.size() == 0) cout <<"some strange happend !!!"<<endl;
	  //set the status
	  if((mpcex_peak_hits.status == MpcExPeakHits::Unset) && (mpcex_peak_hits.size() == 1)) {
            mpcex_peak_hits.status = MpcExPeakHits::Isolate;
//	    cout <<"status: Isolate"<<endl;
	  }
          if((mpcex_peak_hits.status == MpcExPeakHits::Unset) && (mpcex_peak_hits.size() > 1)) {
            mpcex_peak_hits.status = MpcExPeakHits::Good;
//	    cout <<"status: Good"<<endl;
	  }
	  if(mpcex_peak_hits.size() > 1) {
	    int max_range = *max_element(x_y_limit,x_y_limit+4);
	    cout<<"-----------------------------------------"<<endl;
	    print_surround(hit,mpcex_hitmap,max_range+1);
	    cout <<mpcex_peak_hits.size()<<" hits in peak find in arm "<<arm<<" layer "<<layer<<" "<<mpcex_peak_hits.status<<endl;
	    for(int ibug = 0;ibug <4;ibug++){
              cout <<ibug<<" range "<<x_y_limit[ibug]<<" ";
	    }
	    cout <<endl;
	    cout<<"-----------------------------------------"<<endl;
	  }
	  _peaks_list[arm][layer].push_back(mpcex_peak_hits);
	}//ispeak
      }//iter
    }//layer
  }//arm


  return EVENT_OK;
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
  cout <<"peak hit : "<<get_good_q(hit) <<" nx "<<nx<<" ny "<<ny<<" "<<index<<endl;
  for(int i = ny+range;i >= ny-range;i--){
    for(int j = nx-range;j<= nx+range;j++){
//      cout <<i<<" "<<j<<endl;
      unsigned int index0 = mpcex_hitmap.get_index(arm,layer,j,i);
//      cout<<index0<<endl;
      TMpcExHit* hit0 = mpcex_hitmap.get_hit(index0);
      double hit_q = get_good_q(hit0);
      if(hit_q > 0) cout<<hit_q<<" ";
      else cout<<"0000.0 ";
    }
    cout <<endl;
  }
}

int mMpcExMyShower::find_showers0(){
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  vector<TMpcExHit*> peaks_list[2][8];
  typedef pair<TMpcExHit*,double> pair_type;
  vector<pair_type> peaks_of_arm[2];
  //find all peaks in peak layer
  for(unsigned int arm = 0;arm < 2;arm++){
//some issues with arm 1
//    if(arm == 1) continue;
    for(unsigned int layer = 0;layer < 8;layer++){
      MpcExHitMap::const_iterator iter = mpcex_hitmap.get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = mpcex_hitmap.get_layer_first(arm,layer+1);
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
        if(is_peak(hit,mpcex_hitmap)){
	  peaks_list[arm][layer].push_back(hit);
	  peaks_of_arm[arm].push_back(pair<TMpcExHit*,double>(hit,get_good_q(hit)));
	}
      }
    }//layer
  }//arm
 
  //associate these peaks in hough space
  //sort by q and starting at biggest q;
  std::sort(peaks_of_arm[0].begin(),peaks_of_arm[0].end(),sort_by_q);
  std::sort(peaks_of_arm[1].begin(),peaks_of_arm[1].end(),sort_by_q);
  
  typedef vector<TMpcExHit*> hit_list;
  vector<hit_list> track_candi_list;
  set<unsigned int> used_list;
  for(unsigned int arm = 0;arm < 2;arm++){
    for(unsigned int i = 0;i < peaks_of_arm[arm].size();i++){
      TMpcExHit* hit = peaks_of_arm[arm][i].first;
      unsigned test_arm = hit->arm();
      if(test_arm != arm){
       cout <<"seris error: arm does not match!!!"<<endl;
      }
      unsigned int index = mpcex_hitmap.get_index(hit);
      if(used_list.find(index) != used_list.end()) continue;
      unsigned int layer = hit->layer();
      vector<TMpcExHit*> track_candi;
      track_candi.push_back(hit);
      used_list.insert(index);
      for(unsigned int ilayer = 0;ilayer < 8;ilayer ++){
        //only search different layer
	if(layer == ilayer) continue;
	double best_dist = 999;
	TMpcExHit* best_hit = NULL;
	for(unsigned int j = 0;j < peaks_list[arm][ilayer].size();j++){
          TMpcExHit* hit2 = peaks_list[arm][ilayer][j];
	  unsigned int index2 = mpcex_hitmap.get_index(hit2);
          if(used_list.find(index2) != used_list.end()) continue;
	  unsigned int layer2 = hit2->layer();
	  if(layer2 != ilayer) cout <<"series error: layer does not match!!!"<<endl;
          double hsx = hit->hsx(_vertex);
	  double hsy = hit->hsx(_vertex);
	  double hsx2 = hit2->hsx(_vertex);
	  double hsy2 = hit2->hsy(_vertex);
	  double z = hit->z();
	 

	 //different type of layers should be different
          if(layer%2 == 0 && layer2%2 == 0 && (fabs(hsx-hsx2) < fabs(0.4/(z-_vertex))) && (fabs(hsy-hsy2)< fabs(1.6/(z-_vertex)))){
            if(best_dist > fabs(hsx-hsx2)) {
              best_dist = fabs(hsx - hsx2);
	      best_hit = hit2;
	    }
	  }          
	  else if(layer%2 == 1 && layer2%2 == 1 && (fabs(hsx-hsx2) < fabs(1.6/(z-_vertex))) && (fabs(hsy-hsy2)< fabs(0.4/(z-_vertex)))){
	    if(best_dist > fabs(hsy-hsy2)){
              best_dist = fabs(hsy-hsy2);
	      best_hit = hit2;
	    }
	  }
	  else if((fabs(hsx-hsx2) < fabs(0.8/(z-_vertex))) && (fabs(hsy-hsy2)< fabs(0.8/(z-_vertex)))){
            double dist = sqrt((hsx-hsx2)*(hsx-hsx2)+(hsy-hsy2)*(hsy-hsy2));
	    if(best_dist > dist){
              best_dist = dist;
	      best_hit = hit2;
	    }
	  }
	
	}//j
        //find the closet hit
	if(!best_hit) {
	  track_candi.push_back(best_hit);
	  unsigned int best_index = mpcex_hitmap.get_index(best_hit);
	  used_list.insert(best_index);
	}
      }//ilayer
      track_candi_list.push_back(track_candi);
    }//i
  }//arm

//associate the hits in different layers
  
  vector<hit_list> showers_list;
  used_list.clear();
  for(unsigned int i = 0;i < track_candi_list.size();i++){
    vector<TMpcExHit*> track_candi = track_candi_list[i];
    //now require at least two layers are fired
    if(track_candi.size() < 2) continue; 
    
    vector<TMpcExHit*> shower;
    for(unsigned int j = 0;j < track_candi.size();j++){
//      TMpcExHit* hit =  track_candi[j];
//      unsigned int arm = hit->arm();
//      unsigned int layer = hit->layer();
//      unsigned int nx = mpcex_hitmap.get_nx(hit);
//      unsigned int ny = mpcex_hitmap.get_ny(hit);

/*
      //nx+ direction
      for(unsigned int ii = 1;ii < 100;ii++){
        unsigned int index2 = mpcex_hitmap.get_index(arm,layer,nx+ii,ny);
        TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2); 
	if(!hit2) break;
        double hit2_q = get_good_q(hit2);
	if(hit2 <=0) break;



	unsigned int index3 = mpcex_hitmap.get_index(arm,layer,nx+ii,ny);
        TMpcExHit* hit3 = mpcex_hitmap.get_hit(index3);

      }
  */    
       
    }
  }
  return EVENT_OK;
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
