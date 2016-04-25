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

struct MpcExPeakHits{
  MpcExPeakHits():arm(0),layer(0),peak_index(0),peak_nx(0),peak_ny(0),peak_hsx(0),peak_hsy(0),peak_z(0),peak_q(0),avg_hsx(0),avg_hsy(0),tot_q(0),hits_index(){}
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
  double tot_q;
  std::vector<unsigned int> hits_index;
  void push_back(unsigned int index){hits_index.push_back(index);}
  unsigned int operator[](unsigned int index) const {return hits_index[index];}
  unsigned int size() const {return hits_index.size();}
};

struct MpcExMyShower{
  MpcExMyShower():arm(0),fired_layers(0),hsx(0),hsy(0),tot_q(0){}
  unsigned int arm;
  unsigned int fired_layers;
  double hsx;
  double hsy;
  double tot_q;
  std::vector<unsigned int> hits_index;
  void push_back(unsigned int index){hits_index.push_back(index);}
  unsigned int operator[](unsigned int index) const {return hits_index[index];}
  unsigned int size() const {return hits_index.size();}
};

class mMpcExMyShower : public SubsysReco{
  public:
    mMpcExMyShower(const char* name = "MMPCEXMYSHOWER");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExMyShower();
    virtual int End(PHCompositeNode*);

    double get_good_q(const TMpcExHit*) const;
    
    bool is_peak(const TMpcExHit*,const MpcExHitMap&) const;
    void print_surround(const TMpcExHit*,const MpcExHitMap&,int range = 2) const;
    double get_hough_dist(const TMpcExHit*,const TMpcExHit*) const;
    void print_mpcex_peak_hits(const MpcExPeakHits*,const MpcExHitMap&) const;
//    int find_showers0(); //first algebra to find showers
//    int find_showers1(); //second algebra to find showers 
    int find_showers2(); //third algebra to find showers

    int shower_test();

    void peak_map_reset();
    void shower_list_reset();



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
    std::vector<MpcExMyShower*> _shower_list[2]; 
    Exogram* grammy[2];

    double _vertex;

    //shower test
    TH2D* htracks_vs_towers[2];
};

#endif /*__MMPCEXMYSHOWER_HH__*/
