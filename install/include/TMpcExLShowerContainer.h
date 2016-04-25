#ifndef __TMPCEXLSHOWERCONTAINER_H__
#define __TMPCEXLSHOWERCONTAINER_H__
/*
 *class TMpcExLShowerContainer
 *by Liankun Zou @2015/09/23
 *
 */
#include "PHObject.h"
#include "vector"

class TMpcExLShower;

class TMpcExLShowerContainer : public PHObject{
  public:
    TMpcExLShowerContainer():_showers_list(){}
    virtual ~TMpcExLShowerContainer();

    TMpcExLShower* get_shower(unsigned int i) const {
      if(i < _showers_list.size()) return _showers_list[i];
      else return NULL;
    }

    unsigned int size() const {return _showers_list.size();}
    void add_shower(TMpcExLShower* shower) {_showers_list.push_back(shower); }
    void Reset();
    void reset();
    
  private:
    std::vector<TMpcExLShower*> _showers_list;   
};


#endif /*__TMPCEXLSHOWERCONTAINER_H__*/
