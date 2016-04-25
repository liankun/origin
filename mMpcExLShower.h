#ifndef __MMPCEXLSHOWER_H__
#define __MMPCEXLSHOWER_H__
/*
 *class mMpcExLShower 
 * by Liankun Zou @2015/09/23
 * the algorithem to find 
 * the shower
 * */

#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include "vector"
#include "map"

class PHCompositeNode;
class MpcMap;
class TMpcExHitContainer;
class mpcTowerContainer;
class mpcClusterContainer;
class MpcExEventQuality;
class TMpcExLShowerContainer;
class MpcExHitMap;
class TMpcExCalibContainer;
class TMpcExLTrack;
class TMpcExLShower;
class TMpcExHit;


//parameters in shower algorithem
//require two hit in different layer < 1.7*Minipad short length 
#define HIT_DST_SHORT 1.7
//require two hit in different layer < 1*Minipad long length 
#define HIT_DST_LONG 1.
//require two hit in different layer < 1*Minipad long length, for different layer type
#define HIT_DST 0.6
//weight for x,y direction when calculate average hsx and average hsy
#define WEIGHT 8.
//shower radias in rms, merge the tracks to make showers
#define SHOWER_RAD 3.
//sample fraction
#define SAMPLING_FRACTION 0.02161

struct MpcExPeakHits2{
  unsigned int arm;
  unsigned int layer;
  unsigned int peak_index;
  double peak_z;
  double avg_hsx;
  double avg_hsy;
  double avg_hsx2;
  double avg_hsy2;
  double tot_q;
  std::vector<unsigned int> hits_index;
  void push_back(unsigned int index){hits_index.push_back(index);}
  unsigned int operator[](unsigned int index) const {return hits_index[index];}
  unsigned int size() const {return hits_index.size();}
};

struct MpcExMyTrack2{
  unsigned int arm;
  unsigned int fired_layers;
  double hsx;
  double hsy;
  double hsx2;
  double hsy2;
  double hsx_rms;
  double hsy_rms;
  double tot_q;
  double norm_x;
  double norm_y;
  std::vector<unsigned int> hits_index;
  void push_back(unsigned int index){hits_index.push_back(index);}
  unsigned int operator[](unsigned int index) const {return hits_index[index];}
  unsigned int size() const {return hits_index.size();}
};


class mMpcExLShower : public SubsysReco{
  public:
    mMpcExLShower(const char* name = "MMPCEXLSHOWER");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExLShower();
    virtual int End(PHCompositeNode*);

    double get_good_q(const TMpcExHit*) const;

    int find_track();
    int make_shower();
    void make_shower_parameters(TMpcExLShower*);

    TMpcExLTrack* create_ltrack(const MpcExMyTrack2*);

    void peak_map_reset();
    void track_list_reset();

    void set_dead_map_path(const char* path){_dead_map_path = path;}

  private:
    void set_interface_ptrs(PHCompositeNode*);
    TMpcExHitContainer* _mpcex_hit_container;
    MpcMap* _mpc_map;
    TMpcExCalibContainer* _mpcex_calibs;
    mpcTowerContainer* _mpc_tower_container;
    mpcClusterContainer* _mpc_cluster_container;
    TMpcExLShowerContainer* _showers_container;
    MpcExEventQuality* _evt_quality;
    MpcExHitMap* _mpcex_hit_map;
    
    short _dead_map[49152][2];
    std::map<unsigned int, MpcExPeakHits2*> _peaks_map[2][8];
    std::vector<MpcExMyTrack2*> _track_list[2];
    const char* _dead_map_path;

    double _vertex;
};

#endif /*__MMPCEXLSHOWER_H__*/
