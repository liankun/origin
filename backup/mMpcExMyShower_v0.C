#include "mMpcExMyShower.h"
#include "TMpcExCalibContainer.h"
#include "TMpcExHitContainer.h"
#include "TMpcExHit.h"
#include "MpcExRawHit.h"
#include "MpcExEventQuality.h"
#include "MpcExHitMap.h"

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


  
  MpcExHitMap mpcex_hitmap(_mpcex_hit_container);
  typedef vector<TMpcExHit> myshower;
  typedef vector<unsigned int> peaks;
  vector<peaks> peaks_list;
  vector<unsigned int> used_list;
  for(unsigned int arm = 0;arm < 2;arm++){
    MpcExHitMap::iterator iter_begin = mpcex_hitmap.get_layer_first(arm,0);
    MpcExHitMap::iterator iter_end = mpcex_hitmap.get_layer_first(arm+1,0);
    for(;iter_begin != iter_end;iter_begin++){
      TMpcExHit* hit = iter_begin->second;
      //ignore none peaks
      if(!is_peak(hit)) continue;
      unsigned int nx = mpcex_hitmap.get_nx(hit);
      unsigned int ny = mpcex_hitmap.get_ny(hit);
      unsigned int layer = hit->layer();
      unsigned int index = mpcex_hitmap.get_index(hit);
      double hsx = hit->hsx();
      double hsy = hit->hsy();
      //search the peaks in the following layers
      
      for(unsigned int ilayer = layer+1;ilayer < 8;ilayer++){
        int delta_nx = 9;
	int detta_ny = 1;
	if(ilayer%2 == 1){
	  delta_nx = 1;
	  delta_ny = 9;
	}
	  //loop over the following to find peaks
	unsigned int good_peak = 0;
	//the distances for x-y,x-x,y-y, type layer are different
	double dist = 9999;
	for(int inx = nx-delta_nx;inx <= nx+delta_nx;inx++){
          for(int iny = ny-delta_ny;iny <= ny+delta_ny;iny++){
            unsigned int index2 = mpcex_hitmap.get_index(arm,ilayer,inx,iny);
	    TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2);
            if(is_peak(hit2)){
              double hsx2 = hit2->hsx();
              double hsy2 = hit2->hsy();
	      double scale_x = 8;
	      double scale_y = 8;
	      //x-x type
	      if((layer%2 == 0 && ilayer%2 ==0)) scale_x = 1;
	      //y-y type
              if((layer%2 == 1 && ilayer%2 ==1)) scale_y = 1;

              double dist2 = sqrt((hsx2-hsx)*(hsx2-hsx)/scale_x+(hsy2-hsy)*(hsy2-hsy)/scale_y);
	      if(dist > dist2){
                dist = dist2;
		good_peak = index2;
	      }
	    }
	  }//iny
	}//inx
	if(good_peak != 0 && dist < 0.0045) peak_list.push_back(good_peak);
      }//ilayer
      if(peak_list.size() > 1){
         
      }
    }//iiter
  }//arm
 
  return EVENT_OK;
}


//if the hit is the maximum or not
bool mMpcExMyShower::is_peak(const TMpcExHit* hit) const{
  unsigned int index = mpcex_hitmap.get_index(hit);
  unsigned int layer = hit->layer();
  unsigned int arm = hit->arm();
  unsigned int index_down = 198*6*4*layer+198*4*6*8*arm;
  unsigned int indiex_up =  198*6*4*(layer+1)+198*4*6*8*arm;
  double hit_q = get_good_q(hit);
 
  //loop over the surround minipad 
  unsigned index_list[4] ={-1,1,-198,198}; 
  for(unsigned int i = 0;i < 4;i++){
    unsigned int index2 = index+index_list[i];
    if((index2 > index_down) && (index2 < index_up)){
      TMpcExHit* hit2 = mpcex_hitmap.get_hit(index2);
      if(!hit2 && (get_good_q(hit2) > hit_q)) return false;
    }
  }
 
  return true;
}

double mMpcExMyShower::get_good_q(const TMpcExHit* hit) const{
  //the status can be changed due to the changing of software
  if(hit->get_status()!= TMpcExHit::PEDESTAL_SUBTRACTED) return -9999;
  
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
