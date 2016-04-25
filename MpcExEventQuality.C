#include "MpcExEventQuality.h"
#include <iostream>
using namespace std;

MpcExEventQuality::MpcExEventQuality(){
  _trigger_wanted = false;
  _vertex_wanted = false;
  _single_bufferred = false;
  _event_wanted = false;
  for(int arm = 0;arm < 2;arm++){
    for(int packet = 0;packet < 8;packet++){
      for(int chip = 0;chip < 48;chip++){
        _cell_id_dominated[arm][packet][chip] = false;
	_cell_id_locked[arm][packet][chip] = false;
	_cell_id_good[arm][packet][chip] = false;
      }
    }
  }
}

MpcExEventQuality::~MpcExEventQuality(){

}

bool MpcExEventQuality::IsCellIDDominated(int arm ,int packet,int index){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return false;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return false;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return false;
  }
  return _cell_id_dominated[arm][packet][index];
}

bool MpcExEventQuality::IsCellIDUnLocked(int arm,int packet,int index){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return false;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return false;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return false;
  }
  return _cell_id_locked[arm][packet][index]; 
}

bool MpcExEventQuality::IsCellIDGood(int arm,int packet,int index){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return false;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return false;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return false;
  }
  return _cell_id_good[arm][packet][index]; 
}

void MpcExEventQuality::setCellIDDominated(int arm,int packet,int index, bool tof){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return ;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return ;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return ;
  }
  _cell_id_dominated[arm][packet][index] = tof;
}

void MpcExEventQuality::setCellIDUnLocked(int arm, int packet,int index,bool tof){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return;
  }
  _cell_id_locked[arm][packet][index] = tof;
}

void MpcExEventQuality::setCellIDGood(int arm,int packet,int index,bool tof){
  if(arm < 0 || arm > 1) {
    cout <<"Arm should be 0 or 1 !!!"<<endl;
    return;
  }
  if(packet < 0 || packet > 7){
    cout <<"Packet should be between 0 and 7 !!!"<<endl;
    return;
  }
  if(index < 0 || index > 47){
    cout <<"Index should be between 0 and 47 !!!"<<endl;
    return;
  }
  _cell_id_good[arm][packet][index] = tof; 
}

void MpcExEventQuality::Reset(){
  _trigger_wanted = false;
  _vertex_wanted = false;
  _single_bufferred = false;
  _event_wanted = false;
  for(int arm = 0;arm < 2;arm++){
    for(int packet = 0;packet < 8;packet++){
      for(int chip = 0;chip < 48;chip++){
        _cell_id_dominated[arm][packet][chip] = false;
	_cell_id_locked[arm][packet][chip] = false;
	_cell_id_good[arm][packet][chip] = false;
      }
    }
  }
}
