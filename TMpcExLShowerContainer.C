#include "TMpcExLShowerContainer.h"
#include "TMpcExLShower.h"

TMpcExLShowerContainer::~TMpcExLShowerContainer(){
  reset();
}

void TMpcExLShowerContainer::Reset(){
  for(unsigned int i = 0;i < _showers_list.size();i++){
    delete _showers_list[i];
  }
  _showers_list.clear();
}

void TMpcExLShowerContainer::reset(){
  Reset();
}
