#ifndef __MMPCEXDATAANA_HH__
#define __MMPCEXDATAANA_HH__

#ifndef __CINT__
#include <SubsysReco.h>
#endif

#include <vector>

class SubsysReco;
class PHCompositeNode;
class MpcMap;
class mpcClusterContainer;
class mpcTowerContainer;
class TMpcExHitContainer;
class TMpcExShowerContainer;
class TMpcExCalibContainer;
class MpcExRawHit;
class TMpcExHit;
class TH3F;
class TH2F;
class TH2D;
class TH3D;
class TH1F;
class TH1D;
class MpcExEventHeader;
class MpcExSpin;
class TrigLvl1;
class Exogram;
class MpcExEventQuality;
class mpcSampleContainer;
class MpcCalib;
class mpcRawContainer;
class TMpcExLShowerContainer;
class TCanvas;
class TH1;


#define SAMPLING_FRACTION 0.02161

class mMpcExDataAna: public SubsysReco{
  public:
    mMpcExDataAna(const char* name = "MMPCEXDATAANA");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExDataAna();
    virtual int End(PHCompositeNode*);
    int mpcexhit_init();
    int mpcexhit_study();
    int mpcex_eventheader_init();
    int mpcex_eventheader_study();
    int mpcex_spin_study();
    int mpcex_shower_init();
    int mpcex_shower_study();
    bool isSingleBuffered(); 
    void event_display();
    int mpcex_spin_init();
    int mpcex_hitmap_test();
    int mpcex_myshower();

    void new_shower_module_display();

    int mpcex_new_shower_module_init();
    int mpcex_new_shower_module_study();

    //the Mpc tower signal shape
    int mpc_tower_shape_init();
    int mpc_tower_shape_study();

    int mpc_tower_init();
    int mpc_tower_study();

    int lshower_init();
    int lshower_study();

    int lshower_mip_init();
    int lshower_mip_study();

    // study the hit in the suroundings
    int mpcex_hit2_init();
    int mpcex_hit2_study();

    //study the mip on mpc
    int lshower_mip_mpc_init();
    int lshower_mip_mpc_study();

    //study the track in shower
    int lshower_track_init();
    int lshower_track_study();

    double get_good_q(TMpcExHit*);
    
    int bbc_init();
    int bbc_study(PHCompositeNode*);

    int pair_channel_init();
    int pair_channel_study();

    int zero_surpress_init();
    int zero_surpress_study();

    int sim_mpc_init();
    int sim_mpc_study(PHCompositeNode*);

    int HW_init();
    int HW_study();

    int mpc_cluster_study();

    void lshower_evt_display();
   

    void set_dead_map_path(const char* path){_dead_map_path = path;}

    void set_mpc_tower_ped_path(const char* path){_mpc_tower_ped_path = path;}

    int primary_particle(PHCompositeNode*);

    int run16_mpcex_hit_init();
    int run16_mpcex_hit_study();

    int run16_MpcExHit_calib(); //own module simply calibrate MpcEx

  private:
    void set_interface_ptrs(PHCompositeNode*);
    TMpcExHitContainer* _mpcex_hit_container;
    MpcExRawHit* _mpcex_raw_hits;
    TMpcExShowerContainer* _mpcex_shower_container;
    TMpcExLShowerContainer* _mpcex_lshower_container;
    TMpcExCalibContainer* _mpcex_calibs;
    mpcClusterContainer* _mpc_cluster_container;
    mpcTowerContainer* _mpc_tower_container;
    MpcMap* _mpc_map;
    MpcExSpin* _mpcex_spin;
    TrigLvl1* _triglvl1;
    MpcExEventQuality* _evt_quality;    
    mpcSampleContainer* _mpc_sample_container;
    MpcCalib* _mpc_calib;
    mpcRawContainer* _mpc_raw_container;
    int _triglvl1_clock_cross;

//mpcex hit 
    TH2F* hmpcex_high_hits[2];
    TH2F* hmpcex_low_hits[2];
    TH2F* hmpcex_high_vs_low[2];
    TH1D* hmpcex_hitfreq[2];
    TH1F* hvertex_z;
    TH2F* hraw_cal;
    TH2F* hoddq;
    TH2F* hoddq_sub;
    TH2D* hmpcex_good_q[2];//good q study
    TH3D* hmpcex_layer_adc[2];

    //event header study
    TH1F* hStack;
    TH2F* hCellID[2];
    TH2F* hCellIDdiff[2];
    
    TH2F* hStatePhase[2];
    MpcExEventHeader* _mpcex_eventheader;

    int eventNum;

    //event display
    Exogram* grammyl[2];
    Exogram* grammyh[2];
    TH2F* hmpc_gridxy[2];

    TH2F* hmpc_gridxy2[2];
    Exogram* grammy2[2];
   //globle nx ny study
    TH2F* hmpcex_nxy[2][8];    

    //shower study part
    TH3F* hnshower[2];
    TH3F* hshower_e[2];

    //spin information
    TH1F* han;
    TH1F* hphi[2];
   

    //tower study
    TH2D* hmpc_fired_tower[2];
    TH3D* hmpc_fired_tower_e[2];
    TH3D* hmpc_peak_tower[2];
    TH2D* hmpc_tower_e[2];
  
  //tower adc signal shape
    TH3D* hmpc_tower_shape[2];
    TH2D* hmpc_peak_sample[2];
    TH3D* hmpc_tower_chi2[2];

  //new shower module study
    std::vector<TCanvas*> _c_list;
    std::vector<TH1*> _hist_list;
  //new shower module check
    TH3D* hshower_hxy_match[2];
    TH3D* hshower_hxy_match2[2];
    TH3D* hshower_hits_match[2];

  //Lshower module study 
    TH3D* hdr[2];//eta, phi,dependence
    TH3D* hdxy[2];//mpcex energy dependence
    TH3D* hdxy2[2];//mpcex first layer e /mpcex e dependence
    TH3D* hdxy3[2];//rms, e dependence
    TH3D* hlayer_e[2];//layer e, total e
    TH3D* hlayer_e_rms[2];//layer,rms
    TH2D* hrms_e[2];//mpcex e vs rms
    TH3D* hkey_e[2];//key mpcex e
    TH2D* hhigh_low[2];
    TH3D* hfired[2];//fired towers, fired layers
    TH2D* he_hits[2];//number of energy, hits
    TH3D* hlsxy[2];//shower hits distribution
    Exogram* hgrammy[2];

    TH2D* hkey_high[2];//key-high
    TH2D* hkey_low[2];//key-low

    TH3D* hlayer_adc[2];//instead of energy , use adc

    TH3D* hmpcex_hit2[2];//mpcex_hit2
    
    TH3D* hmpcex_mip_bbc[2];//number of mips vs bbc charge
    TH2D* hmpcex_mip_bbc2[2];//number of mips vs bbc charge, whole arm

    TH2D* hlshower_bbc[2];//number of shower vs bbc charge
    TH2D* hltrack_bbc[2];//number of ltrack vs bbc charge
    TH2D* hltrack_shower[2];//numbr of ltrack in shower

    TH2D* hpair_channel[2];//the paired channel in DCM high ADC
    TH2D* hpair_channel2[2];//the paired channel in DCM low ADC

    //bbc study
    TH2D* hcharge_vs_pmt[2];
    TH2D* hNpmt;
    //mip mpc
    TH3D* hmip_mpc[2];
    TH3D* hmip_tower[2];

    //pair channel
    TH1D* hbad_pair_channel[2];//the no existed pair channel key
    //the pair channel should both be zero suppressed,but no
    TH1D* hodd_pair_channel_high[2];   
    TH1D* hodd_pair_channel_low[2];

    TH2D* hneg_adc_ratio[2];
    TH2D* hneg_total[2];
    TH3D* hneg_total_adc_high[2];
    TH3D* hneg_total_adc_low[2];
    TH1D* hratio_evt[2];
    TH2D* hratio_mpc[2];


    //simulation mpc
    TH3D* hsim_mpc_cluster[2];
    TH3D* hsim_mpc_tower[2];
    
    TH2D* hprim_photon[2];
    TH1D* hphoton_theta;

    //HW
    TH1D* hMpc_tot_e[2];
    TH1D* hMpc_twr_e[2];

    TH1D* hMpcEx_tot_e[2];
    TH1D* hMpcEx_mpad_e[2];

    //run 16 stuff
    TH2D* hkey_adc_high;
    TH2D* hkey_adc_low;
    
    TH2D* hbbc_Nhits[2];
    TH2D* hbbc_Nhits_anti[2];
    TH2D* hbbc_adc[2];
    TH2D* hbbc_adc_anti[2];


    double _px_prim;
    double _py_prim;
    double _pz_prim;
    double _p_prim;
    double _hsx_prim;
    double _hsy_prim;
    double _eta;
    double _bbc_charge[2];

    const char* _dead_map_path;
    const char* _mpc_tower_ped_path;
    int _dead_map[49152][2];
    double _mpc_tower_ped[2][288];
  protected:
    double _vertex;
};

#endif /*__MMPCEXDATAANA_HH__*/
