#include "MpcExSpin.h"
#include "iostream"
#include "recoConsts.h"
#include <cstdlib>

using namespace std;

MpcExSpin* MpcExSpin::_instance = NULL;

MpcExSpin::MpcExSpin(){
  recoConsts* rc = recoConsts::instance();
  runnumber = rc->get_IntFlag("RUNNUMBER");
  if(runnumber == 0) {
    cout <<"runnumber  is not set in recoConsts !!!"<<endl;
    exit(1);
  }  
  spin_out.StoreDBContent(runnumber,runnumber);
  if(spin_out.CheckRunRowStore(runnumber)!=1){
    cout <<"read from data base failed !!!"<<endl;
    exit(1);
  }
  
  spin_out.GetDBContentStore(spin_cont,runnumber);
  
  cross_shift = spin_cont.GetCrossingShift();
  qalevel = spin_cont.GetQALevel();
  fillnumber = spin_cont.GetFillNumber();
  for(int i = 0;i < NCROSS;i++){
    spin_cont.GetPolarizationBlue(i,bpol[i],bpolerr[i],bpolsys[i]);
    spin_cont.GetPolarizationYellow(i,ypol[i],ypolerr[i],ypolsys[i]);
    bpat[i] = spin_cont.GetSpinPatternBlue(i);
    ypat[i] = spin_cont.GetSpinPatternYellow(i);
    scaler_bbc_vtxcut[i] = spin_cont.GetScalerBbcVertexCut(i);
    scaler_bbc_nocut[i] = spin_cont.GetScalerBbcNoCut(i);
    scaler_zdc_wide[i] = spin_cont.GetScalerZdcWide(i);
    scaler_zdc_narrow[i] = spin_cont.GetScalerZdcNarrow(i);

    //set pattern
     if(bpat[i] == 1 && ypat[i] == 1) {
        pattern[i]=0;
     } else if( bpat[i] == -1 && ypat[i] == 1) { 
        pattern[i]=1;
     } else if( bpat[i] ==  1 && ypat[i] ==-1) { 
        pattern[i]=2;
     } else if( bpat[i] == -1 && ypat[i] ==-1) { 
      pattern[i]=3;
     } else {                     
        pattern[i]=4;
     }
  }
}

MpcExSpin::~MpcExSpin(){

}

void MpcExSpin::Print(){
  printf("Run number = %d\n",runnumber);
  printf("QA Level = %d\n",qalevel);
  printf("Fill number = %d\n",fillnumber);
  printf("Crossing shift = %d\n",cross_shift);
  
  for(int i=0; i<NCROSS; i++){
    printf("%3d : %12lld %12lld %12lld %12lld : %3d %3d ",i,
    scaler_bbc_vtxcut[i],scaler_bbc_nocut[i],
    scaler_zdc_wide[i],scaler_zdc_narrow[i],
    bpat[i],ypat[i]);
    printf(" : %6.3f +- %6.3f +- %6.3f %6.3f +- %6.3f +- %6.3f\n",
    bpol[i],bpolerr[i],bpolsys[i],ypol[i],ypolerr[i],ypolsys[i]);
  }

  return;
}

