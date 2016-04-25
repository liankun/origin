#ifndef __TMPCEXLSHOWER_H__
#define __TMPCEXLSHOWER_H__


/*
 *Class TMpcExLShower 
 *by Liankun Zou @2015/09
 *
 *
 */

#include "MpcExConstants.h"
#include "vector"

class TMpcExLTrack{
  public:
    TMpcExLTrack();
    virtual ~TMpcExLTrack();
    void add_hit(unsigned int key){_hits.push_back(key);} 
    unsigned int get_hit(unsigned int i) const {
      if(i < _hits.size()){
        return _hits[i];
      }
      else return NULL;
    } 
    unsigned int get_hits_num() const {return _hits.size();}
    
    short  get_arm()               const {return _arm;}
    double get_hsx()               const {return _hsx;}
    double get_hsy()               const {return _hsy;}
    double get_rms_hsx()           const {return _rms_hsx;}
    double get_rms_hsy()           const {return _rms_hsy;}
    double get_mpcex_e()           const {return _mpcex_e;}
    int    get_n_fired_layers()    const {return _n_fired_layers;}
    int    get_first_fired_layer() const {return _first_fired_layer;}
    double get_e_layer(int l)      const {
      if((l>=0) && (l<MpcExConstants::NLAYERS)) return _layer_e[l];
      else return 0.0;
    }

    void set_arm(short arm)           {_arm = arm;}                 
    void set_hsx(double hsx)          {_hsx = hsx;}             
    void set_hsy(double hsy)          {_hsy = hsy;}              
    void set_rms_hsx(double rms_hsx)  {_rms_hsx = rms_hsx;}          
    void set_rms_hsy(double rms_hsy)  {_rms_hsy = rms_hsy;}         
    void set_mpcex_e(double mpcex_e)  {_mpcex_e = mpcex_e;}          
    void set_n_fired_layers(int n)    {_n_fired_layers = n;}   
    void set_first_fired_layer(int n) {_first_fired_layer = n;}
    void set_e_layer(int l,double e)  {if((l>=0) && (l<MpcExConstants::NLAYERS)) _layer_e[l] = e;}     

  private:
    short _arm;
    double _hsx;
    double _hsy;
    double _rms_hsx;
    double _rms_hsy;
    double _mpcex_e;
    int _n_fired_layers;
    int _first_fired_layer;
    double _layer_e[MpcExConstants::NLAYERS];
    std::vector<unsigned int> _hits;
};


class TMpcExLShower{
  public:
    TMpcExLShower();
    virtual ~TMpcExLShower();
    short  get_arm()                 const {return _arm;}
    double get_hsx()                 const {return _hsx;}
    double get_hsy()                 const {return _hsy;}
    double get_rms_hsx()             const {return _rms_hsx;}
    double get_rms_hsy()             const {return _rms_hsy;}
    double get_disp_hsx()            const {return _disp_hsx;}
    double get_disp_hsy()            const {return _disp_hsy;}
    double get_mpcex_e()             const {return _mpcex_e;}
    double get_mpc_e3x3()            const {return _mpc_e3x3;}
    double get_mpc_e5x5()            const {return _mpc_e5x5;}
    double get_closest_tower_dx()    const {return _closest_tower_dx;}
    double get_closest_tower_dy()    const {return _closest_tower_dy;}
    double get_closest_cluster_dx()  const {return _closest_cluster_dx;}
    double get_closest_cluster_dy()  const {return _closest_cluster_dy;}
  
    int    get_closest_mpc_cluster() const {return _closest_mpc_cluster;} 
    int    get_n_fired_layers()      const {return _n_fired_layers;}
    int    get_first_fired_layer()   const {return _first_fired_layer;}
    int    get_n_fired_towers3x3()   const {return _n_fired_towers3x3;}
    int    get_n_fired_towers5x5()   const {return _n_fired_towers5x5;}
    int    get_mpc_towers_ch(int i,int j) const {
             if( (i>=0) && (i<5) && (j>=0) && (j<5) ) return _mpc_towers_ch[i][j];
             else return -1;
    }
    double get_mpc_towers_e(int i,int j) const {
             if( (i>=0) && (i<5) && (j>=0) && (j<5) ) return _mpc_towers_e[i][j];
	     else return 0.0;
    }
    double get_e_layer(int l) const {
      if((l>=0) && (l<MpcExConstants::NLAYERS)) return _layer_e[l];
      else return 0.0;
    }
  
    unsigned int get_tracks_num() const {return _tracks_list.size();}
    unsigned int get_hits_num();

    void add_track(TMpcExLTrack* track){
       if(track) _tracks_list.push_back(track);
    }

    TMpcExLTrack* get_track(unsigned int i) const {
      if(i < _tracks_list.size()) return _tracks_list[i];
      else return NULL;
    }

    unsigned int get_hit(unsigned int i); 

    void set_arm(short arm)             {_arm = arm;}                 
    void set_hsx(double hsx)            {_hsx = hsx;}             
    void set_hsy(double hsy)            {_hsy = hsy;}              
    void set_rms_hsx(double rms_hsx)    {_rms_hsx = rms_hsx;}          
    void set_rms_hsy(double rms_hsy)    {_rms_hsy = rms_hsy;}      
    void set_disp_hsx(double disp_hsx)  {_disp_hsx = disp_hsx;}
    void set_disp_hsy(double disp_hsy)  {_disp_hsy = disp_hsy;}
    void set_mpcex_e(double mpcex_e)    {_mpcex_e = mpcex_e;}
    void set_mpc_e3x3(double e3x3)      {_mpc_e3x3 = e3x3;}
    void set_mpc_e5x5(double e5x5)      {_mpc_e5x5 = e5x5;}
    void set_closest_tower_dx(double v) {_closest_tower_dx = v;}  
    void set_closest_tower_dy(double v) {_closest_tower_dy = v;}  
    void set_closest_cluster_dx(double v){_closest_cluster_dx = v;} 
    void set_closest_cluster_dy(double v){_closest_cluster_dy = v;} 
    void set_closest_mpc_cluster(int i) {_closest_mpc_cluster = i;}
    void set_n_fired_layers(int n)      {_n_fired_layers = n;}   
    void set_first_fired_layer(int n)   {_first_fired_layer = n;}
    void set_n_fired_towers3x3(int n)   {_n_fired_towers3x3 = n;}
    void set_n_fired_towers5x5(int n)   {_n_fired_towers5x5 = n;}
    void set_e_layer(int l,double e)    {if((l>=0) && (l<MpcExConstants::NLAYERS)) _layer_e[l] = e;}     
    void set_mpc_towers_ch(int i,int j,int ch){
           if( (i>=0) && (i<5) && (j>=0) && (j<5) ) _mpc_towers_ch[i][j] = ch;
    }
    void set_mpc_towers_e(int i,int j,double e){
           if( (i>=0) && (i<5) && (j>=0) && (j<5) ) _mpc_towers_e[i][j] = e;
    }
  
  private:
    short _arm;
    double _hsx;
    double _hsy;
    double _rms_hsx;
    double _rms_hsy;
    double _disp_hsx;
    double _disp_hsy;
    //use the good q: choose a better q in low and high q
    double _mpcex_e;
    double _mpc_e3x3;
    double _mpc_e5x5;

    int _closest_mpc_cluster;
    int _n_fired_layers;
    double _layer_e[8];
    int _first_fired_layer;
    //record the fired towers around 3x3 towers
    int _n_fired_towers3x3;
    //record the fired towers around 5x5 towers
    int _n_fired_towers5x5;
    //record the towers (channel number) around the closest tower 
    int _mpc_towers_ch[5][5];
    //record the towers energy around the closest tower
    double _mpc_towers_e[5][5];
    //record the different to the tower
    double _closest_tower_dx;
    double _closest_tower_dy;
    double _closest_cluster_dx;
    double _closest_cluster_dy;

    bool fill_hits_done;
    void fill_hits();

    std::vector<TMpcExLTrack*> _tracks_list;
    std::vector<unsigned int> _hits_list;
};
#endif
