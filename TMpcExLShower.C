#include "TMpcExLShower.h"
TMpcExLTrack::TMpcExLTrack(){
  _arm = -1;
  _hsx = -1;
  _hsy = -1;
  _rms_hsx = -1;
  _rms_hsy = -1;
  _mpcex_e = -1;
  _n_fired_layers = -1;
  _first_fired_layer = -1;
  for(int i = 0;i < MpcExConstants::NLAYERS;i++){
    _layer_e[i] = 0;
  }
  _hits.clear();
}

TMpcExLTrack::~TMpcExLTrack(){
  _hits.clear();
}

TMpcExLShower::TMpcExLShower(){
  _arm = -1;
  _hsx = -1;
  _hsy = -1;
  _rms_hsx = -1;
  _rms_hsy = -1;
  _disp_hsx = -1;
  _disp_hsy = -1;
  _mpcex_e = 0;
  _mpc_e3x3 = 0;
  _mpc_e5x5 = 0;
  _closest_tower_dx = -1;
  _closest_tower_dy = -1;
  _closest_cluster_dx = -1;
  _closest_cluster_dy = -1;
  _closest_mpc_cluster = -1;
  _n_fired_layers = -1;
  _first_fired_layer = -1;
  _n_fired_towers3x3 = -1;
  _n_fired_towers5x5 = -1;
  fill_hits_done = false;
  for(int i = 0;i < MpcExConstants::NLAYERS;i++){
    _layer_e[i] = 0;
  }
  for(int i = 0;i < 5;i++){
    for(int j = 0;j < 5;j++){
      _mpc_towers_e[i][j] = 0;
      _mpc_towers_ch[i][j] = -1;
    }
  }
  _tracks_list.clear();
}

unsigned int TMpcExLShower::get_hits_num(){
  if(!fill_hits_done) fill_hits();
  return _hits_list.size();
}

unsigned int TMpcExLShower::get_hit(unsigned int i){
  if(!fill_hits_done) fill_hits();
  if(i < _hits_list.size()) return _hits_list[i];
  else return NULL;
}

void TMpcExLShower::fill_hits(){
  if(fill_hits_done) return;
  _hits_list.clear();
  for(unsigned int i = 0; i < get_tracks_num();i++){
    TMpcExLTrack* track = _tracks_list[i];
    for(unsigned int j = 0;j < track->get_hits_num();j++){
      _hits_list.push_back(track->get_hit(j));
    }
  }
  fill_hits_done = true;
}

TMpcExLShower::~TMpcExLShower(){
  for(unsigned int i = 0;i < _tracks_list.size();i++){
    delete _tracks_list[i];
  }
  _tracks_list.clear();
}
