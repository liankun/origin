#ifndef __MMPCEXMYSHOWER_HH__
#define __MMPCEXMYSHOWER_HH__

#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include "MpcExHitMap.h"
#include <vector>
#include <set>
#include <map>
#include "TMpcExHit.h"

class PHCompositeNode;
class TMpcExCalibContainer;
class MpcExRawHit;
class TMpcExHitContainer;
class mpcClusterContainer;
class mpcTowerContainer;
class MpcMap;
class MpcExEventQuality;
class TMpcExHit;
class Exogram;
class TH2D;
class TH3D;

struct MpcExPeakHits{
  MpcExPeakHits():arm(0),layer(0),peak_index(0),peak_nx(0),peak_ny(0),peak_hsx(0),peak_hsy(0),peak_z(0),peak_q(0),avg_hsx(0),avg_hsy(0),avg_hsx2(0),avg_hsy2(0),tot_q(0),hits_index(){}
  unsigned int arm;
  unsigned int layer;
  unsigned int peak_index;
  unsigned int peak_nx;
  unsigned int peak_ny;
  double peak_hsx;
  double peak_hsy;
  double peak_z;
  double peak_q;
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

struct MpcExMyTrack{
  MpcExMyTrack():arm(0),fired_layers(0),hsx(0),hsy(0),hsx2(0),hsy2(0),hsx_rms(0),hsy_rms(0),tot_q(0),norm_x(0),norm_y(0){}
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

struct MpcExMyShower{
  unsigned int arm;
  unsigned int fired_layers;
  double hsx;
  double hsy;
  double hsx_rms;
  double hsy_rms;
  double tot_q;
  double mpc_e3x3;
  double mpc_e5x5;
  std::vector<MpcExMyTrack*> track_list;
  std::vector<int> tower_list;
  unsigned int track_num() const { return track_list.size();}
  void add_track(MpcExMyTrack* track){track_list.push_back(track);}
  unsigned int tower_num() const { return tower_list.size();}
  void add_tower(int itower) {tower_list.push_back(itower);}
  MpcExMyTrack* get_track(unsigned int i){return track_list[i];}
  int get_tower(int i){return tower_list[i];}
};

class mMpcExMyShower : public SubsysReco{
  public:
    mMpcExMyShower(const char* name = "MMPCEXMYSHOWER");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExMyShower();
    virtual int End(PHCompositeNode*);

    double get_good_q(const TMpcExHit*,const MpcExHitMap&) const;
    
    bool is_peak(const TMpcExHit*,const MpcExHitMap&) const;
    void print_surround(const TMpcExHit*,const MpcExHitMap&,int range = 2) const;
    double get_hough_dist(const TMpcExHit*,const TMpcExHit*) const;
    void print_mpcex_peak_hits(const MpcExPeakHits*,const MpcExHitMap&) const;
//    int find_showers0(); //first algebra to find showers
//    int find_showers1(); //second algebra to find showers 
    int find_tracks2(); //third algebra to find tracks
   
    int make_shower(); //merge track to find showers

    int shower_test();

    int myshower_test(PHCompositeNode* topNode);

    void shower_display();

    int dead_hot_channel();

    void peak_map_reset();
    void track_list_reset();



  private:
    void set_interface_ptrs(PHCompositeNode*);
    TMpcExHitContainer* _mpcex_hit_container;
    MpcExRawHit* _mpcex_raw_hits;
    TMpcExCalibContainer* _mpcex_calibs;
    mpcClusterContainer* _mpc_cluster_container;
    mpcTowerContainer* _mpc_tower_container;
    MpcMap* _mpc_map;
    MpcExEventQuality* _evt_quality;
    
    std::map<unsigned int, MpcExPeakHits*> _peaks_map[2][8];
    std::vector<MpcExMyTrack*> _track_list[2];
    std::vector<MpcExMyShower> _shower_list[2];
    std::set<unsigned int> _hot_channel;
    Exogram* grammy[2];

    double _vertex;

    //shower test
    TH2D* htracks_vs_towers[2];
    TH3D* htracks_towers[2];
    TH2D* htracks_angle[2];
    TH2D* hhits_angle[2][8];
    TH2D* hmpcex_nxy_freq[2][8];
    TH2D* hmpcex_nxy_q[2][8];

    TH3D* hshower_hits[2];
    TH3D* hshower_hxy_match[2];

};

#endif /*__MMPCEXMYSHOWER_HH__*/
