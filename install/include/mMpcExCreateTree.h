#ifndef __MMPCEXCREATETREE_HH__
#define __MMPCEXCREATETREE_HH__

#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include <vector>

class SubsysReco;
class PHCompositeNode;
class TMpcExShowerContainer;
class TMpcExLShowerContainer;
class TTree;
class TFile;
class MpcExEventQuality;
class TMpcExHitContainer;
class mpcClusterContainer;
class mpcTowerContainer;
class MpcExSpin;
class TrigLvl1;

  struct MyMpcExHit{
//    int shower;
    int key;
    float q;
  };

  struct MyShower{
    int arm;
    int nlayers;
    int first_layer;
    int mpcN3x3;
    int mpcN5x5;
    int mpcPeakix;
    int mpcPeakiy;
    int CalibEInRange;
    float vertex;
    float hsx;
    float hsy;
    float mpc33hsx;
    float mpc33hsy;
    float mpc_e3x3;
    float mpc_e5x5;
    float roughTotE;
    float mpcCentTwrE;
    float ClosestMPCClusterDistanceX;
    float ClosestMPCClusterDistanceY;
    float mpcECorr;
    float esum;
    float raw_esum;
    float rms_x[8];
    float rms_y[8];
    float e_layer[8];
    std::vector<MyMpcExHit> hits;
  };

  struct MyLShower{
    int arm;
    int nlayers;
    int first_layer;
    int mpcN3x3;
    int mpcN5x5;
    int mpc_towers_ch[5][5];
    float vertex;
    float hsx;
    float hsy;
    float mpc_towers_e[5][5];
    float mpc_e3x3;
    float mpc_e5x5;
    float ClosestMPCClusterDistanceX;
    float ClosestMPCClusterDistanceY;
    float rms_x[8];
    float rms_y[8];
    float e_layer[8];
//    std::vector<MyMpcExHit> hits;
  };

  struct MyMpcTower{
//    int cluster;
    int ch;
    float e;
    float tof;
  };


  struct MyMpcCluster{
    int arm;
    float x;
    float y;
    float z;
    float e;
    float chi2;
    float vertex;
    std::vector<int> towers;
  };

  struct EventInfo{
    EventInfo():run_number(0),fill_number(0),lvl1_clock_cross(-9999),gl1_cross_id(-9999),cross_shift(-9999),ypat(-9999),bpat(-9999),ypol(-9999),bpol(-9999),ypolerr(-9999),bpolerr(-9999){}
    int run_number;
    int fill_number;
    int lvl1_clock_cross;
    int gl1_cross_id;
    int cross_shift;
    int ypat;
    int bpat;
    float ypol;
    float bpol;
    float ypolerr;
    float bpolerr;
  };



class mMpcExCreateTree: public SubsysReco{
  public:
    mMpcExCreateTree(const char* name = "MMPCEXCREATETREE");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExCreateTree();
    virtual int End(PHCompositeNode*);
    
    void set_file_name(const char* name){file_name = name;}
  private:
    void set_interface_ptrs(PHCompositeNode*);
    TMpcExShowerContainer* _mpcex_shower_container;
    TMpcExLShowerContainer* _mpcex_lshower_container;
    MpcExEventQuality* _evt_quality;
    TMpcExHitContainer* _mpcex_hit_container;
    mpcTowerContainer* _mpc_tower_container;
    mpcClusterContainer* _mpc_cluster_container;
    MpcExSpin* _mpcex_spin;
    TrigLvl1* _triglvl1;


    int _triglvl1_clock_cross;
    TTree* mytree;
    TFile* omyfile;
    const char* file_name;
    std::vector<MyShower> shower_list;
    std::vector<MyLShower> lshower_list;
    std::vector<MyMpcCluster> mpc_cluster_list;
    std::vector<MyMpcTower> mpc_tower_list;
    std::vector<MyMpcExHit> mpcex_shower_hits;
    std::vector<MyMpcExHit> mpcex_lshower_hits;
    EventInfo evtinfo;
};
#endif /*__MMPCEXCREATETREE_HH__*/
