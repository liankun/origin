#ifndef __MPCEXSPIN_H__
#define __MPCEXSPIN_H__

/* class MpcExSpin
 * Liankun Zou
 *July 20 2015
 *read the spin informantion from the database
 */

#include "SpinDBContent.hh"
#include "SpinDBOutput.hh"

#define NCROSS 120

class MpcExSpin{
  
  public:
    virtual ~MpcExSpin();
  //greb the spin information from the database
  static MpcExSpin* instance(){
    if(_instance == NULL){
      _instance = new MpcExSpin();
    }
    return _instance;
  }
 
  void Print();
  int GetRunNumber(){return runnumber;}
  int GetFillNumber(){return fillnumber;}
  int GetQALevel(){return qalevel;}
  int GetCrossingShift(){return cross_shift;}
  float GetPolBlue(int bunch){return bpol[(bunch+cross_shift)%120];}
  float GetPolYellow(int bunch){return ypol[(bunch+cross_shift)%120];}
  float GetPolErrorBlue(int bunch){return bpolerr[(bunch+cross_shift)%120];}
  float GetPolErrorYellow(int bunch){return ypolerr[(bunch+cross_shift)%120];}
  int GetSpinPatternBlue(int bunch){return bpat[(bunch+cross_shift)%120];}
  int GetSpinPatternYellow(int bunch){return ypat[(bunch+cross_shift)%120];}
  int GetSpinPattern(int bunch){return pattern[(bunch+cross_shift)%120];}
  
  long long GetScalerBbcVertexCut(int bunch){return scaler_bbc_vtxcut[(bunch+cross_shift)%120];}
  long long GetScalerBbcNoCut(int bunch){return scaler_bbc_nocut[(bunch+cross_shift)%120];}
  long long GetScalerZdcWide(int bunch){return scaler_zdc_wide[(bunch+cross_shift)%120];}
  long long GetScalerZdcNarrow(int bunch){return scaler_zdc_narrow[(bunch+cross_shift)%120];}


  protected:  
    MpcExSpin();

    static MpcExSpin* _instance;
    int runnumber;
    int cross_shift;
    int fillnumber;
    int qalevel;
    float bpol[120];
    float bpolerr[120];
    float bpolsys[120];
    float ypol[120];
    float ypolerr[120];
    float ypolsys[120];
    int bpat[120];
    int ypat[120];
    int pattern[120];
    long long scaler_bbc_vtxcut[120];
    long long scaler_bbc_nocut[120];
    long long scaler_zdc_wide[120];
    long long scaler_zdc_narrow[120];

    SpinDBContent spin_cont;
    SpinDBOutput spin_out;

};

#endif /* __MPCEXSPIN_H__*/

