#include "mMpcExLShower.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "TMpcExLShower.h"
#include "TMpcExLShowerContainer.h"
#include "MpcExRawHit.h"
#include "MpcExEventQuality.h"
#include "TMpcExHitContainer.h"
#include "MpcExConstants.h"
#include "MpcExHitMap.h"

#include "MpcMap.h"
#include "mpcClusterContent.h"
#include "mpcClusterContainer.h"
#include "mpcClusterContentV1.h"
#include "mpcTowerContainer.h"
#include "mpcTowerContent.h"
#include "mpcTowerContentV1.h"

#include "PHIODataNode.h"
#include "getClass.h"
#include "BbcOut.h"
#include "PHGlobal.h"

#include "Fun4AllReturnCodes.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include "stdlib.h"

using namespace std;
using namespace findNode;

mMpcExLShower::mMpcExLShower(const char* name): 
  SubsysReco(name)
{
  _vertex = -9999.9;
  _dead_map_path = NULL;
  for(unsigned int i = 0;i < 49152;i++){
    _dead_map[i][0] = 0;
    _dead_map[i][1] = 0;
  }
}

mMpcExLShower::~mMpcExLShower(){
  peak_map_reset();
  track_list_reset();
}

int mMpcExLShower::Init(PHCompositeNode* topNode){
//add the TMpcExLShowerContainer Node  
  PHNodeIterator nodeIter(topNode);
  PHCompositeNode *dstNode = static_cast<PHCompositeNode*>(nodeIter.findFirst("PHCompositeNode","DST"));
  _showers_container = new TMpcExLShowerContainer();
  PHIODataNode<TMpcExLShowerContainer>* LshowerNode = new PHIODataNode<TMpcExLShowerContainer>(_showers_container,"TMpcExLShowerContainer","PHObject");
  dstNode->addNode(LshowerNode);
 
//read the dead map
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

  return EVENT_OK; 
}

int mMpcExLShower::InitRun(PHCompositeNode* topNode){
  set_interface_ptrs(topNode);
  _showers_container->Reset();
  return EVENT_OK;
}

void mMpcExLShower::set_interface_ptrs(PHCompositeNode* topNode){
  _mpcex_hit_container = getClass<TMpcExHitContainer>(topNode,"TMpcExHitContainer");
  if(!_mpcex_hit_container){
    cout << PHWHERE <<":: No TMpcExHitContainer!!!"<<endl;
    exit(1);
  } 
 
  _mpcex_calibs = getClass<TMpcExCalibContainer>(topNode,"TMpcExCalibContainer");
  if(!_mpcex_calibs){
    cout << PHWHERE <<":: No TMpcExCalibContainer !!!"<<endl;
    exit(1);
  }
 
//  _evt_quality = getClass<MpcExEventQuality>(topNode,"MpcExEventQuality");
//  if(!_evt_quality){
//    cout << PHWHERE <<"No MpcExEventQuality !!!"<<endl;
//    exit(1);
//  }

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
    exit(1);
  }
}

int mMpcExLShower::End(PHCompositeNode* topNode){
  return EVENT_OK;
}

int mMpcExLShower::process_event(PHCompositeNode* topNode){
//  if(!_evt_quality->IsEventWanted()) return EVENT_OK;
  //vertex
  PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout && !phglobal){
    cout <<"No BbcOut or PHGlobal !!!"<<endl;
    exit(1);
  }
  _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
  
//  cout <<"process mMpcExLShower "<<endl;
  _mpcex_hit_map = new MpcExHitMap(_mpcex_hit_container);
//  cout <<"1"<<endl;
  if(find_track() == EVENT_OK) make_shower();
//  cout <<"2"<<endl;
//  make_shower();
  //need to be delete each event
  delete _mpcex_hit_map;
  
  return EVENT_OK;
}

int mMpcExLShower::find_track(){
  peak_map_reset();

  //each hits associate with a peak;
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int layer = 0;layer < MpcExConstants::NLAYERS;layer++){
      std::map<unsigned int,unsigned int> hit_to_peak_map;
      MpcExHitMap::const_iterator iter = _mpcex_hit_map->get_layer_first(arm,layer);
      MpcExHitMap::const_iterator iter_end = _mpcex_hit_map->get_layer_first(arm,layer+1);
      for(;iter != iter_end;iter++){
        TMpcExHit* hit = iter->second;
        unsigned int index = _mpcex_hit_map->get_index(hit);
	//already used or useless 
	if(hit_to_peak_map.find(index) != hit_to_peak_map.end()) continue;
	
	double hit_q = get_good_q(hit);
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
	MpcExPeakHits2* find_peak = NULL;
	while(!find_peak){
          count++;
          
	  TMpcExHit* pre_hit = _mpcex_hit_map->get_hit(index);
	  int nx = _mpcex_hit_map->get_nx(pre_hit);
	  int ny = _mpcex_hit_map->get_ny(pre_hit);
	  max_q = get_good_q(pre_hit);
          
	  double q_surround[4] = {-9999,-9999,-9999,-9999};
	  unsigned int index_surround[4] = {0,0,0,0};
	  for(int i = 0;i < 4;i++){
	    //if nx or ny is out of range will return 0
            unsigned int index0 = _mpcex_hit_map->get_index(arm,layer,nx+suround[i][0],ny+suround[i][1]);
	    //if index0 is invalid, hit0 will be Null;
	    TMpcExHit* hit0 = _mpcex_hit_map->get_hit(index0);
	    //if hit0 is Null, q will be -9999.9;
	    double hit0_q = get_good_q(hit0);
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

	  //find the peak
	  if(max_index == index){ 
          //check if the peak already exists
	    map<unsigned int,unsigned int>:: iterator hit_to_peak_map_iter = hit_to_peak_map.find(max_index); 
          //not exists create the peak
	    if(hit_to_peak_map_iter == hit_to_peak_map.end()){ 
//              hit_to_peak_map.insert(pair<unsigned int,unsigned int>(index,index));
	      MpcExPeakHits2* mpcex_peak_hits = new MpcExPeakHits2();
	      mpcex_peak_hits->arm = arm;
	      mpcex_peak_hits->layer = layer;
	      mpcex_peak_hits->peak_index = max_index;
	      TMpcExHit* peak_hit = _mpcex_hit_map->get_hit(max_index);
	      mpcex_peak_hits->peak_z = peak_hit->z();
	      mpcex_peak_hits->tot_q = 0;
	      _peaks_map[arm][layer].insert(pair<unsigned int,MpcExPeakHits2*>(index,mpcex_peak_hits));
	     
	      find_peak = mpcex_peak_hits;
	    }
	    else{//peak already exist
              unsigned int peak_index = hit_to_peak_map_iter->second;
	      map<unsigned int,MpcExPeakHits2*>::iterator pk_map_iter = _peaks_map[arm][layer].find(peak_index);
	      // the peak should exsit
	      if(pk_map_iter == _peaks_map[arm][layer].end()){
                cout <<"something strange happened "<<endl;
		return -1;
	      }
	      find_peak = pk_map_iter->second;
	    }
	  }
	  else {//peak not find
	    //search if the max_hit has alread has a peak belong to 
	    map<unsigned int,unsigned int>:: iterator hit_to_peak_map_iter = hit_to_peak_map.find(max_index); 
	    if(hit_to_peak_map_iter == hit_to_peak_map.end()){//no peak belong to 
	      hits_in_peak.push_back(max_index);
	      index = max_index;
	    }
	    else{//the peak has already has a peak belong to
	      unsigned int peak_index = hit_to_peak_map_iter->second;
              map<unsigned int,MpcExPeakHits2*>::iterator pk_map_iter = _peaks_map[arm][layer].find(peak_index);
	      if(pk_map_iter == _peaks_map[arm][layer].end()){
                cout<<PHWHERE<<"::something strange happened"<<endl;
		return -1;
	      }
              find_peak = pk_map_iter->second;
	    }
	  }
          if(count > 200){
            cout <<PHWHERE<<"::the nubmer of loop is bigger than 200,  something strange may happen"<<endl;
	    return -1;
	  }
	}//while
        
	//insert the hits
	//size = 1, the hit has already inserted in creating process
	for(unsigned int ii = 0;ii < hits_in_peak.size();ii++){
          find_peak->push_back(hits_in_peak[ii]);
	  TMpcExHit* ii_pk_hit = _mpcex_hit_map->get_hit(hits_in_peak[ii]);
	  double god_q = get_good_q(ii_pk_hit);
	  double ii_hit_hsx = ii_pk_hit->hsx(_vertex);
	  double ii_hit_hsy = ii_pk_hit->hsy(_vertex);
	  //average hsx,hsy
	  find_peak->avg_hsx += god_q*ii_hit_hsx; 
	  find_peak->avg_hsy += god_q*ii_hit_hsy;
	  //average hsx^2,hsy^2
	  find_peak->avg_hsx2 += god_q*ii_hit_hsx*ii_hit_hsx;
	  find_peak->avg_hsy2 += god_q*ii_hit_hsy*ii_hit_hsy;
	  
	  find_peak->tot_q += god_q;
	  
	  unsigned int find_peak_index = find_peak->peak_index;
	  hit_to_peak_map.insert(pair<unsigned int,unsigned int>(hits_in_peak[ii],find_peak_index));
	} 
      }//iter
    }//layer
  }//arm


//set hsx,hsy,hsx^2,hsy^2
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int layer = 0;layer < MpcExConstants::NLAYERS;layer++){
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits2*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits2*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
	MpcExPeakHits2* pk_hits = pk_map_iter->second;
	
	pk_hits->avg_hsx = (pk_hits->avg_hsx)/(pk_hits->tot_q);
      	pk_hits->avg_hsy = (pk_hits->avg_hsy)/(pk_hits->tot_q);

	pk_hits->avg_hsx2 = (pk_hits->avg_hsx2)/(pk_hits->tot_q);
      	pk_hits->avg_hsy2 = (pk_hits->avg_hsy2)/(pk_hits->tot_q);

      }
    }
  }


 //associate this peaks to make tracks according to hsx,hsy
  set<unsigned int>used_peak_list;
  track_list_reset();  
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int layer = 0;layer < MpcExConstants::NLAYERS;layer++){
      if(_peaks_map[arm][layer].size() == 0) continue;
      map<unsigned int,MpcExPeakHits2*>::iterator pk_map_iter = _peaks_map[arm][layer].begin();
      map<unsigned int,MpcExPeakHits2*>::const_iterator pk_map_end = _peaks_map[arm][layer].end();
      for(;pk_map_iter!=pk_map_end;pk_map_iter++){
	MpcExPeakHits2* pk_hits = pk_map_iter->second;
	if(used_peak_list.find(pk_hits->peak_index) != used_peak_list.end()) continue;
        MpcExMyTrack2* mytrack = new MpcExMyTrack2();
	mytrack->arm = arm;
	mytrack->tot_q = pk_hits->tot_q;
	//push back the hit 
	for(unsigned int ihit = 0;ihit < pk_hits->size();ihit++){
	  mytrack->push_back((*pk_hits)[ihit]);
	}
	mytrack->fired_layers = 0;
	mytrack->fired_layers++;

        double weight_x = WEIGHT*pk_hits->tot_q;
	double weight_y = pk_hits->tot_q;
	if(layer%2 == 1){
          weight_x = pk_hits->tot_q;
	  weight_y = WEIGHT*pk_hits->tot_q;
	}

	mytrack->hsx = (pk_hits->avg_hsx)*weight_x;
	mytrack->hsy = (pk_hits->avg_hsy)*weight_y;
	mytrack->hsx2 = (pk_hits->avg_hsx2)*weight_x;
	mytrack->hsy2 = (pk_hits->avg_hsy2)*weight_y;

	mytrack->norm_x = weight_x;
	mytrack->norm_y = weight_y;
	
	//check the same type layer first
	for(unsigned ilayer = layer+2 ;ilayer< MpcExConstants::NLAYERS;ilayer += 2){
          if(_peaks_map[arm][ilayer].size() == 0) continue;
	  map<unsigned int,MpcExPeakHits2*>::iterator ipk_map_iter = _peaks_map[arm][ilayer].begin();  
	  map<unsigned int,MpcExPeakHits2*>::const_iterator ipk_map_end = _peaks_map[arm][ilayer].end();
	  double good_dsxy = 9999;
	  MpcExPeakHits2* good_pk_hits = NULL;
	  //find the closest 
	  for(;ipk_map_iter!=ipk_map_end;ipk_map_iter++){
	    MpcExPeakHits2* ipk_hits = ipk_map_iter->second;
	    if(used_peak_list.find(ipk_hits->peak_index) != used_peak_list.end()) continue;
	    
	    double z = ipk_hits->peak_z;
	    double ihsx = ipk_hits->avg_hsx;
	    double ihsy = ipk_hits->avg_hsy;
	    double hsx = (mytrack->hsx)/(mytrack->norm_x);
	    double hsy = (mytrack->hsy)/(mytrack->norm_y);
	    double dhsx = fabs(ihsx-hsx);
	    double dhsy = fabs(ihsy-hsy);

	    //x direction
	    double short_length = HIT_DST_SHORT*MpcExConstants::MINIPAD_SHORT_LENGTH;
	    double long_length = HIT_DST_LONG*MpcExConstants::MINIPAD_LONG_LENGTH;
//            cout <<"short length "<<short_length<<" long_length"<<long_length<<endl;
	    if(layer%2 == 0 && (dhsy < short_length/fabs(z-_vertex)) && (dhsx < long_length/fabs(z-_vertex))){
	      if(good_dsxy > dhsx){
                good_dsxy = dhsx;
		good_pk_hits = ipk_hits;
	      }
	    }
	    if(layer%2 == 1 && (dhsx < long_length/fabs(z-_vertex)) && (dhsy < short_length/fabs(z-_vertex))){
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
	    
	    
	    weight_x = WEIGHT*good_pk_hits->tot_q;
	    weight_y = good_pk_hits->tot_q;
	    
	    if(layer%2 == 1){
              weight_x = good_pk_hits->tot_q;
	      weight_y = WEIGHT*good_pk_hits->tot_q;
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
    	for(unsigned ilayer = layer+1 ;ilayer<MpcExConstants::NLAYERS;ilayer += 2){
          if(_peaks_map[arm][ilayer].size() == 0) continue;
	  map<unsigned int,MpcExPeakHits2*>::iterator ipk_map_iter = _peaks_map[arm][ilayer].begin();  
	  map<unsigned int,MpcExPeakHits2*>::const_iterator ipk_map_end = _peaks_map[arm][ilayer].end();
	  double good_dsxy = 9999;
	  MpcExPeakHits2* good_pk_hits = NULL;
	  for(;ipk_map_iter!=ipk_map_end;ipk_map_iter++){
	    MpcExPeakHits2* ipk_hits = ipk_map_iter->second;
	    if(used_peak_list.find(ipk_hits->peak_index) != used_peak_list.end()) continue;
	    
	    double z = ipk_hits->peak_z;
	    double ihsx = ipk_hits->avg_hsx;
	    double ihsy = ipk_hits->avg_hsy;
	    double hsx = (mytrack->hsx)/(mytrack->norm_x);
	    double hsy = (mytrack->hsy)/(mytrack->norm_y);
	    double dhsx = fabs(ihsx-hsx);
	    double dhsy = fabs(ihsy-hsy);

	    //x direction
	    double length = HIT_DST*MpcExConstants::MINIPAD_LONG_LENGTH;
//	    cout<<"length "<<length<<endl;
	    if((dhsy < length/fabs(z-_vertex)) && (dhsx < length/fabs(z-_vertex))){
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
	    weight_y = WEIGHT*(good_pk_hits->tot_q);

	    if(layer%2 == 1){
              weight_x = WEIGHT*(good_pk_hits->tot_q);
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
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int i = 0; i < _track_list[arm].size();i++){
      MpcExMyTrack2* mytrack = _track_list[arm][i];
      mytrack->hsx = (mytrack->hsx)/(mytrack->norm_x);
      mytrack->hsy = (mytrack->hsy)/(mytrack->norm_y);
      
      mytrack->hsx2 = (mytrack->hsx2)/(mytrack->norm_x);
      mytrack->hsy2 = (mytrack->hsy2)/(mytrack->norm_y);
     
      //rms
      double hsx = mytrack->hsx;
      double hsy = mytrack->hsy;
      double hsx2 = mytrack->hsx2;
      double hsy2 = mytrack->hsy2;

      double ps_z = MpcExConstants::PS_REFERENCE_Z_S;
      if(arm == 1) ps_z = MpcExConstants::PS_REFERENCE_Z_N;

      if(hsx2 > hsx*hsx) mytrack->hsx_rms = sqrt(hsx2-hsx*hsx);
      else {
//        mytrack->hsx_rms = 0;
	mytrack->hsx_rms = MpcExConstants::MINIPAD_SHORT_LENGTH/sqrt(12.)/fabs(ps_z - _vertex);

//        cout <<"rms hsx is not usable"<<endl;
      }

      if(hsy2 > hsy*hsy) mytrack->hsy_rms = sqrt(hsy2-hsy*hsy);
      else {
//        mytrack->hsy_rms = 0;
	mytrack->hsy_rms = MpcExConstants::MINIPAD_SHORT_LENGTH/sqrt(12.)/fabs(ps_z - _vertex);
//        cout <<"rms hsx is not usable"<<endl;
      }

      mytrack->tot_q = mytrack->tot_q/SAMPLING_FRACTION/1.0e6;
    }
  }
  return EVENT_OK;
}

void mMpcExLShower::peak_map_reset(){
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int layer = 0;layer < MpcExConstants::NLAYERS;layer++){
      while(!_peaks_map[arm][layer].empty()){
        map<unsigned int,MpcExPeakHits2*>::iterator iter = _peaks_map[arm][layer].begin();
	delete iter->second;
	_peaks_map[arm][layer].erase(iter);
      }
    }
  }
}

void mMpcExLShower::track_list_reset(){
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int i = 0;i < _track_list[arm].size();i++){
      if(_track_list[arm][i]) delete _track_list[arm][i];
    }
    _track_list[arm].clear();
  }
}

bool sort_track_by_q(MpcExMyTrack2* track1,MpcExMyTrack2* track2){
  double q1 = track1->tot_q;
  double q2 = track2->tot_q;
  return (q1 > q2);
}

double mMpcExLShower::get_good_q(const TMpcExHit* hit) const{
//the status can be changed due to the changing of software
  if(!hit) return -9999.9;
//  cout <<"high "<<hit->high()<<" low "<<hit->low()<<" combined "<<hit->combined()<<endl;
  double hit_q = -9999.9;
  
  if(hit->state_combined()!=TMpcExHit::INVALID) hit_q = hit->combined();
  if(hit_q < 10) hit_q = -9999.9;

  
  //no calibrated data
//  if(hit->isGoodHighHit()){
//     hit_q = hit->high()*147.6/19.0;
//  }
//  if(hit_q > 220 && hit->isGoodLowHit()) hit_q = hit->low()*147.6/19.0*4.5;
  
  return hit_q;
}

TMpcExLTrack* mMpcExLShower::create_ltrack(const MpcExMyTrack2* mytrack){
  if(!mytrack) return NULL;

  TMpcExLTrack* ltrack = new TMpcExLTrack();
  ltrack->set_arm(mytrack->arm);
  ltrack->set_hsx(mytrack->hsx);
  ltrack->set_hsy(mytrack->hsy);
  ltrack->set_rms_hsx(mytrack->hsx_rms);
  ltrack->set_rms_hsy(mytrack->hsy_rms);
  ltrack->set_mpcex_e(mytrack->tot_q);
  ltrack->set_n_fired_layers(mytrack->fired_layers);
  
  int first_fired_layer = 9999;
//  cout <<"create the track, hit number: "<<mytrack->size()<<endl;
  
  for(unsigned int i = 0;i < mytrack->size();i++){
    
    unsigned int index = (*mytrack)[i];
    TMpcExHit* hit = _mpcex_hit_map->get_hit(index);
    unsigned int key = hit->key();
    int layer = hit->layer();
 //   cout <<"layer "<<layer<<endl;
    if(layer < first_fired_layer) first_fired_layer = layer;
    double q = ltrack->get_e_layer(layer);
    q += get_good_q(hit)/(SAMPLING_FRACTION*1.0e6);
    ltrack->set_e_layer(layer,q);
    ltrack->add_hit(key);
  }
  ltrack->set_first_fired_layer(first_fired_layer);

  return ltrack;
}

int mMpcExLShower::make_shower(){
  _showers_container->Reset();
//order the track by q
  sort(_track_list[0].begin(),_track_list[0].end(),sort_track_by_q);
  sort(_track_list[1].begin(),_track_list[1].end(),sort_track_by_q);
  for(unsigned int arm = 0 ;arm < MpcExConstants::NARMS;arm++){
    set<unsigned int> used_tracks;
//    cout <<"number of tracks "<<_track_list[arm].size()<<endl;
    for(unsigned int i = 0;i < _track_list[arm].size();i++){     
      if(used_tracks.find(i) != used_tracks.end()) continue;
      
      MpcExMyTrack2* mytrack = _track_list[arm][i];
      //create the track
      TMpcExLTrack* ltrack = create_ltrack(mytrack);
      //create the shower
      TMpcExLShower* lshower = new TMpcExLShower();
      lshower->set_arm(arm);

      lshower->add_track(ltrack);
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
	  MpcExMyTrack2* track = _track_list[arm][j];
	  double hsx1 = track->hsx;
	  double hsy1 = track->hsy;
	  double norm_x1 = track->norm_x;
	  double norm_y1 = track->norm_y;
             
	  double ps_z = MpcExConstants::PS_REFERENCE_Z_S;
	  if(arm == 1) ps_z = MpcExConstants::PS_REFERENCE_Z_N;
           
	  double hsr_rms = sqrt(hsx_rms*hsx_rms + hsy_rms*hsy_rms);
	  double shower_radius = hsr_rms*fabs(ps_z - _vertex);
	  //the size of the shower 
	  //radius is assume as 3 rms
	  if(shower_radius > 1) break;

	  if((fabs(hsx-hsx1) < SHOWER_RAD*hsx_rms) && (fabs(hsy-hsy1) < SHOWER_RAD*hsy_rms)){

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

            if(hsx2 < hsx*hsx){
//	      cout <<PHWHERE<<":: rms hsx may not be usable "<<endl;
	      hsx_rms = MpcExConstants::MINIPAD_SHORT_LENGTH/sqrt(12.)/fabs(ps_z - _vertex);
	    }
	    if(hsy2 < hsy*hsy){
//	      cout <<PHWHERE<<":: rms hsy may not be usable "<<endl;
	      hsy_rms = MpcExConstants::MINIPAD_SHORT_LENGTH/sqrt(12.)/fabs(ps_z - _vertex);
	    }
      
            TMpcExLTrack* ltrack2 = create_ltrack(track);
	    lshower->add_track(ltrack2);
	    used_tracks.insert(j);
	    track_counts++;
	  }
	}
	if(track_counts == 0) track_added = false;
      }
     
      lshower->set_hsx(hsx);
      lshower->set_hsy(hsy);
      lshower->set_rms_hsx(hsx_rms);
      lshower->set_rms_hsy(hsy_rms);
      make_shower_parameters(lshower);
      _showers_container->add_shower(lshower);
    }//track_list
  }//arm
 
  return EVENT_OK; 
}

void mMpcExLShower::make_shower_parameters(TMpcExLShower* lshower){
  if(!lshower) return;
  //set layer e, mpcex e
  int first_fired_layer = 9999;
  set<unsigned int> fired_layers;
  double layer_e[MpcExConstants::NLAYERS] = {0.0};
  double total_e = 0;
  double mpcex_e = 0;
  unsigned int Ntrack = lshower->get_tracks_num();
  for(unsigned int i = 0;i < Ntrack;i++){
    TMpcExLTrack* ltrack = lshower->get_track(i);
    int fired_layer = ltrack->get_first_fired_layer();
    if(first_fired_layer > fired_layer) first_fired_layer = fired_layer;
//    cout <<"track "<<i<<" first fired layer "<< fired_layer<<endl;    
    mpcex_e += ltrack->get_mpcex_e();
    for(unsigned int ilayer = fired_layer; ilayer < MpcExConstants::NLAYERS;ilayer++){
      double e = ltrack->get_e_layer(ilayer);
      if(e > 0){
       total_e += e;
       layer_e[ilayer] += e;
       fired_layers.insert(ilayer);
      }
    }//ilayer
  }//i
  
  //crossing check,should never happen
  if(fabs((mpcex_e - total_e)/total_e) > 0.0001) cout << PHWHERE<<":: touble in total energy: "<<mpcex_e<<" "<<total_e<<endl;
  lshower->set_mpcex_e(mpcex_e);
  lshower->set_first_fired_layer(first_fired_layer);
  lshower->set_n_fired_layers(fired_layers.size());
  for(unsigned int ilayer = first_fired_layer;ilayer < MpcExConstants::NLAYERS;ilayer++){
    lshower->set_e_layer(ilayer,layer_e[ilayer]); 
//    if(fired_layers.size() == 8){
//      cout <<ilayer<<" elayer in Lshower "<<layer_e[ilayer]<<endl;
//    }
  }

  //debug code:
  
  
  //assosiate with MPC
  if(!_mpc_cluster_container || !_mpc_tower_container) return;

  double hsx = lshower->get_hsx();
  double hsy = lshower->get_hsy();
  int arm = lshower->get_arm();
  double z = MpcExConstants::MPC_REFERENCE_Z;
  if(arm == 0) z = -z;
  double lz = z - _vertex;
  double x = hsx*lz;
  double y = hsy*lz;
  
  //mpc information
  //the estimated grid x,y distribution
  int ix = 0;
  int iy = 0;
  if(arm == 0){
    double step_x = (20.73+20.73)/18;
    double step_y = (20.49+20.67)/18;
    ix = (x + 20.82)/step_x;
    iy = (y + 20.93)/step_y;
  }
  else{
    double step_x = (21.08+21.12)/18;
    double step_y = (20.48+20.67)/18;
    ix = (21.3 - x)/step_x;
    iy = (y + 20.7)/step_y;
  }
  //search the closest tower, the tower may not be fired
  double closest_dr = 9999;
  double closest_dx = 9999;
  double closest_dy = 9999;

  double closest_dr2 = 9999;
  double closest_dx2 = 9999;
  double closest_dy2 = 9999;

  int closest_ch = -9999;
  for(int iix = ix-3;iix <= ix+3;iix++){
    for(int iiy = iy-3;iiy <= iy+3;iiy++){
      int tower_ch = _mpc_map->getFeeCh(iix,iiy,arm); 
      if(tower_ch < 0) continue;
      int tx = _mpc_map->getX(tower_ch);
      int ty = _mpc_map->getY(tower_ch);
      double dx = tx - x;
      double dy = ty - y;
      double dr = sqrt(dx*dx+dy*dy);
      if(closest_dr2 > dr){
        closest_dr2 = dr;
	closest_dx2 = dx;
	closest_dy2 = dy;
      }
      if(fabs(dx) < 1.2 && fabs(dy) < 1.2 ){
        if(closest_dr > dr) {
          closest_dr = dr;
	  closest_dx = dx;
	  closest_dy = dy;
	  closest_ch = tower_ch;
        }
      }
    }//iiy
  }//iix
  
  int closest_ix = ix;
  int closest_iy = iy;
 
  lshower->set_closest_tower_dx(closest_dx2);
  lshower->set_closest_tower_dy(closest_dy2);

  if(closest_ch > 0){
//    cout <<PHWHERE<<"::find the fired tower for the shower"
//         <<"closest_ch : "<<closest_ch<<" closest_dx: "<<closest_dx<<" closest_dy: "<<closest_dy
//	 <<endl; 
     closest_ix = _mpc_map->getGridX(closest_ch);
     closest_iy = _mpc_map->getGridY(closest_ch);
     lshower->set_closest_tower_dx(closest_dx);
     lshower->set_closest_tower_dy(closest_dy);

     //projection tower
     lshower->set_mpc_towers_ch(2,2,closest_ch);
  }


  int n_fired_tower3x3 = 0;
  int n_fired_tower5x5 = 0;
  double tower_e3x3 = 0;
  double tower_e5x5 = 0;
  int NMpcTowers = _mpc_tower_container->size();
  for(int itower = 0;itower < NMpcTowers;itower++){
    mpcTowerContent* ctwr = _mpc_tower_container->getTower(itower);
    int tow_ch = ctwr->get_ch();
    if(arm != _mpc_map->getArm(tow_ch)) continue;
    int tow_gridx = _mpc_map->getGridX(tow_ch);
    int tow_gridy = _mpc_map->getGridY(tow_ch);
    double tower_e = ctwr->get_energy();
    int dix = tow_gridx - closest_ix;
    int diy = tow_gridy - closest_iy;
    if((abs(dix) < 2) && (abs(diy) < 2) && (tower_e > 0)){
      n_fired_tower3x3++;
      tower_e3x3 += tower_e;
    }
    if((abs(dix) < 3) && (abs(diy) < 3) && (tower_e > 0)){
      n_fired_tower5x5++;
      tower_e5x5 += tower_e;
      if(arm == 0){
        lshower->set_mpc_towers_ch(2+dix,2+diy,tow_ch);
        lshower->set_mpc_towers_e(2+dix,2+diy,tower_e);
      }
      else{//the gridx,y will filp for the North arm
        lshower->set_mpc_towers_ch(2-dix,2+diy,tow_ch);
        lshower->set_mpc_towers_e(2-dix,2+diy,tower_e);
      }
    }
  }
  lshower->set_n_fired_towers3x3(n_fired_tower3x3);
  lshower->set_n_fired_towers5x5(n_fired_tower5x5);
  lshower->set_mpc_e3x3(tower_e3x3);
  lshower->set_mpc_e5x5(tower_e5x5);
//set the cluster
  int fNMpcClusters = _mpc_cluster_container->size();
  closest_dr = 9999;
  closest_dx = 9999;
  closest_dy = 9999;
  int closest_clus = -9999;
  for(int iMPCClus=0 ; iMPCClus<fNMpcClusters ; iMPCClus++){
    mpcClusterContent *clus =  _mpc_cluster_container->getCluster(iMPCClus);
    if(clus->arm()!=arm) continue;
    double clus_x = clus->x();
    double clus_y = clus->y();
    double clus_dx = clus_x - x;
    double clus_dy = clus_y - y;
    double clus_dr = sqrt(clus_dx*clus_dx + clus_dy*clus_dy);
    if(closest_dr > clus_dr){
      closest_clus = iMPCClus;
      closest_dr = clus_dr;
      closest_dx = clus_dx;
      closest_dy = clus_dy;
    }
  }
  lshower->set_closest_mpc_cluster(closest_clus);
  lshower->set_closest_cluster_dx(closest_dx);
  lshower->set_closest_cluster_dy(closest_dy);
}

