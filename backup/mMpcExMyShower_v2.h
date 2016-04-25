#ifndef __MMPCEXMYSHOWER_HH__
#define __MMPCEXMYSHOWER_HH__

#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include "MpcExHitMap.h"
#include <vector>
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

struct MpcExPeakHits{
  MpcExPeakHits():status(Unset),arm(0),layer(0),peak_index(0),hsx(0),hsy(0),peak_hit(NULL),hits(),hits_index(){}
  enum Status {Good = 0,Isolate,Mixed,Unset};
  Status status; 
  unsigned int arm;
  unsigned int layer;
  unsigned int peak_index;
  double hsx;
  double hsy;
  double peak_q;
  TMpcExHit peak_hit;
  std::vector<TMpcExHit> hits;
  std::set<unsigned int> hits_index;
//  ~MpcExPeakHits(){
//    if(peak_hit) delete peak_hit;
//    for(unsigned int i = 0;i < hits.size();i++){
//      if(hits[i])delete hits[i];
//    }
//    hits.clear();
//  }
  void push_back(TMpcExHit hit){hits.push_back(hit);}
  TMpcExHit& operator[](unsigned int i){return hits[i];}
  const TMpcExHit& operator[](unsigned int i) const {return hits[i];}
  unsigned int size(){return hits.size();};
  bool exist(unsigned int index){
    if(hits_index.find(index) == hits_index.end()) return false;
    else return true;
  }
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

    int find_showers0(); //first algebra to find showers
    int find_showers1(); //second algebra to find showers 
    int find_showers2(); //third algebra to find showers

  private:
    void set_interface_ptrs(PHCompositeNode*);
    TMpcExHitContainer* _mpcex_hit_container;
    MpcExRawHit* _mpcex_raw_hits;
    TMpcExCalibContainer* _mpcex_calibs;
    mpcClusterContainer* _mpc_cluster_container;
    mpcTowerContainer* _mpc_tower_container;
    MpcMap* _mpc_map;
    MpcExEventQuality* _evt_quality;
    
    std::vector<MpcExPeakHits> _peaks_list[2][8];
    std::map<unsigned int, MpcExPeakHits> _peaks_map[2][8];

    double _vertex;
};

#endif /*__MMPCEXMYSHOWER_HH__*/
