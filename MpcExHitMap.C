#include "MpcExHitMap.h"
#include "MpcExMapper.h"
#include "MpcExRawHit.h"
#include "TMpcExHit.h"
#include "TMpcExHitContainer.h"
#include "MpcExConstants.h"
#include "iostream"

MpcExHitMap::MpcExHitMap(const MpcExRawHit* raw): _raw_hits(raw),_calib_hits(NULL),_hit_map(){
  for ( unsigned int i = 0; i < _raw_hits->getnhits() ; i++)
    {
     TMpcExHit *hit = new TMpcExHit(_raw_hits->getOnlineKey(i));
     hit->set_low(_raw_hits->getladc(i));
     hit->set_high(_raw_hits->gethadc(i));
     hit->set_state_high(TMpcExHit::ADC);
     hit->set_state_low(TMpcExHit::ADC);
     unsigned int index = get_index(hit);
     _hit_map.insert(std::pair<unsigned int,TMpcExHit*>(index,hit));
  }
}

MpcExHitMap::MpcExHitMap(const TMpcExHitContainer* hits):_raw_hits(NULL),_calib_hits(hits),_hit_map(){
  for ( unsigned int i = 0; i < _calib_hits->size() ; i++){
    TMpcExHit *hit = _calib_hits->getHit(i)->clone();
    unsigned int index = get_index(hit);
    _hit_map.insert(std::pair<unsigned int,TMpcExHit*>(index,hit));
  }
}

MpcExHitMap::MpcExHitMap():_hit_map(){
  std::cout <<"MpcExHitMap will be initialized for test !!! "<<std::endl;
  for(unsigned int arm = 0;arm < MpcExConstants::NARMS;arm++){
    for(unsigned int packet =0;packet < MpcExConstants::NPACKETS_PER_ARM;packet++){
      for(unsigned int chipmap = 0;chipmap < MpcExConstants::NMINIPADS_PER_PACKET;chipmap++){
        MpcExMapper* mpcex_mapper = MpcExMapper::instance();
	unsigned int key =  mpcex_mapper->generate_key(arm,packet,chipmap);
        TMpcExHit *hit = new TMpcExHit(key);
        unsigned int nx = get_nx(hit);
	unsigned int ny = get_ny(hit);
	unsigned layer = hit->layer();
//	unsigned new_layer = layer + arm*8;
	int weight = nx + 198*ny;
	if(layer%2 == 1) weight = ny + 198*nx; 
	hit->set_low(weight);
        hit->set_high(weight);
        hit->set_state_high(TMpcExHit::ADC);
	hit->set_state_low(TMpcExHit::ADC);
        unsigned int index = get_index(hit);
//	const_iterator finder =_hit_map.find(index);
//	if(finder != _hit_map.end()) std::cout<<"already exist!!!"<<std::endl;
        _hit_map.insert(std::pair<unsigned int,TMpcExHit*>(index,hit));
      }
    }
  } 
}

MpcExHitMap::~MpcExHitMap(){
  while(!_hit_map.empty()){
    container::const_iterator iter = _hit_map.begin();
    delete iter->second;
    _hit_map.erase(_hit_map.begin());
  }
}



MpcExHitMap::const_iterator MpcExHitMap::get_begin() const {
  return _hit_map.begin();
}

MpcExHitMap::const_iterator MpcExHitMap::get_end() const {
  return _hit_map.end();
}

unsigned int MpcExHitMap::get_index(const TMpcExHit* hit) const{
//ny from 0 to 24 and nx from 0 to 198 (or reverse) 
  unsigned int layer = hit->layer();
  unsigned int arm = hit->arm();
  unsigned int nx = get_nx(hit);
  unsigned int ny = get_ny(hit);
  unsigned index = nx+198*ny+198*6*4*layer+198*4*6*8*arm;
  if(layer%2 == 1){
    index = ny+198*nx+198*6*4*layer+198*4*6*8*arm;
  }
  return index; 
}

unsigned int MpcExHitMap::get_index(unsigned int arm,unsigned int layer,unsigned int nx,unsigned int ny) const{
  unsigned index = 0;
  if(arm > 1 || layer > 7){
   std::cout <<"bad arm or layer value "<<std::endl;
     return 0;
  }
  if(layer%2 == 0){
    if(nx > 197 || ny > 23){
//      std::cout <<"bad nx or ny value"<<std::endl;
      return 0;
    }
    index = nx+198*ny+198*6*4*layer+198*4*6*8*arm;
  }
  if(layer%2 == 1){
    if(nx > 23 || ny > 197){
//      std::cout <<"bad nx or ny value"<<std::endl;
      return 0;
    }
    index = ny+198*nx+198*6*4*layer+198*4*6*8*arm;
  }
  return index;
}

MpcExHitMap::const_iterator MpcExHitMap::get_layer_first(unsigned int arm,unsigned int layer) const {
  if(arm == 0 && layer > 7 ) {
    arm = 1;
    layer = 0;
  }
  if(arm > 1 || (arm == 1 && layer > 7)) return _hit_map.end();
  unsigned int index = 198*6*4*layer+198*4*6*8*arm;
  return _hit_map.lower_bound(index);
}

TMpcExHit* MpcExHitMap::get_hit(unsigned int index) const {
  const_iterator iter = _hit_map.find(index);
  if(iter != _hit_map.end()){
    return iter->second;
  }
  else return NULL;
}

unsigned int MpcExHitMap::get_nx(const TMpcExHit* hit) const {
  if(!hit) {
    std::cout<<"MpcExHitMap::bad hit"<<std::endl;
    return 0;
  }
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  unsigned int key = hit->key();
  unsigned int nx = mpcex_mapper->get_nx(key);
  return nx;
}

unsigned int MpcExHitMap::get_ny(const TMpcExHit* hit) const {
  if(!hit) {
    std::cout<<"MpcExHitMap::bad hit"<<std::endl;
    return 0;
  }
  MpcExMapper* mpcex_mapper = MpcExMapper::instance();
  unsigned int key = hit->key();
  unsigned int arm = hit->arm();
  unsigned int layer = hit->layer();
  unsigned int ny = mpcex_mapper->get_ny(key);;
  if(arm == 1){
    if(layer%2 == 0) ny = 23 - ny;
    else ny = 197 - ny;
  }    
  return ny;
}

void MpcExHitMap::remove(unsigned int index) {
  iterator iter = _hit_map.find(index);
  if(iter != _hit_map.end()){
    delete iter->second;
    _hit_map.erase(iter);
  }
}

void MpcExHitMap::clear(){


}
