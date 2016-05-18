#ifndef __MMPCEXAUAUANA_HH__
#define __MMPCEXAUAUANA_HH__

#include <vector>

#ifndef __CINT__
#include <SubsysReco.h>
#endif

class SubsysReco;
class PHCompositeNode;
class MpcMap;
class mpcClusterContainer;
class mpcTowerContainer;
class TMpcExLShowerContainer;
class TMpcExShowerContainer;
class TMpcExHitContainer;
class TH2D;
class Exogram;
class MpcExRawHit;
class TMpcExCalibContainer;
class mpcRawContainer;

struct MyCluster{
  int arm;
  double x;
  double y;
};

//git test

class mMpcExAuAuAna:public SubsysReco{
  public:
    mMpcExAuAuAna(const char* name = "mMpcExAuAuAna");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExAuAuAna();
    virtual int End(PHCompositeNode*);

    int event_display();
    int general_data_check_init();
    int general_data_check_study();
    int lshower_init();
    int lshower_study();
    int lshower_random_match();

    int shower_init();
    int shower_study();


    private:
      void set_interface_ptrs(PHCompositeNode*);
      TMpcExHitContainer* _mpcex_hit_container;
      TMpcExShowerContainer* _mpcex_shower_container;
      TMpcExLShowerContainer* _mpcex_lshower_container;
      TMpcExCalibContainer* _mpcex_calib_container;

      mpcClusterContainer* _mpc_cluster_container;
      mpcTowerContainer* _mpc_tower_container;
      MpcExRawHit* _mpcex_raw_hits;
      MpcMap* _mpc_map;
      mpcRawContainer* _mpc_raw_container;

      TH2D* hkey_adc_high;
      TH2D* hkey_adc_low;
      TH2D* hkey_rawadc_high;
      TH2D* hkey_rawadc_low;

      TH2D* hadc_sensor_layer_high[2][8];
      TH2D* hadc_sensor_layer_low[2][8];

      TH2D* hHL_sensor[2][8][24];
      TH2D* hHL_sensor_raw[2][8][24];

      TH2D* hlayer_adc_high[2];
      TH2D* hlayer_adc_low[2];
      TH2D* htower_raw;
      TH2D* htower_e;
      TH2D* htower_tdc;

      TH2D* hbbc_nhits[2];

      TH2D* hbbc_adc[2];
      TH2D* hbbc_adc_low[2];
      Exogram* hgrammy_high[2];
      Exogram* hgrammy_low[2];
      
      //combine high and low q,use better one;
      Exogram* hgrammy_combine[2];

      //event display
      Exogram* grammyh[2];
      Exogram* grammyl[2];

      TH2D* hmpc_gridxy[2];

      TH2D* hadc_mpc_e[2];
    
      TH2D* hlshowr_layer_e[2];
      TH2D* hclosest_cluster[2];
      TH2D* hclosest_clus_dx_bbc[2];
      TH2D* hclosest_clus_dy_bbc[2];


      TH2D* hclosest_tower[2];

      double _bbc_charge[2];      

      std::vector<MyCluster> mycluster_list;     
      bool used;
      
    protected:
      double _vertex;
};

#endif
