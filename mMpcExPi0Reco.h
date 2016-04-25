#ifndef __MPCEXPI0RECO_HH__
#define __MPCEXPI0RECO_HH__

#ifndef __CINT__
#include <SubsysReco.h>
#endif

class SubsysReco;
class PHCompositeNode; 
class MpcMap;
class mpcClusterContainer;
class mpcTowerContainer;
class TMpcExHitContainer;
class TMpcExShowerContainer;
class TMpcExHit;

class mMpcExPi0Reco:public SubsysReco{
  public:
    mMpcExPi0Reco(const char* name = "MPCEXPI0RECO");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExPi0Reco();
    virtual int End(PHCompositeNode*);

    private:
      void set_interface_ptrs(PHCompositeNode*);
      TMpcExHitContainer* _mpcex_hit_container;
      TMpcExShowerContainer* _mpcex_shower_container;
      TMpcExLShowerContainer* _mpcex_lshower_container;
      mpcClusterContainer* _mpc_cluster_container;
      mpcTowerContainer* _mpc_tower_container;
      MpcMap* _mpc_map;

    protected:
      double _vertex;
};

#endif 
