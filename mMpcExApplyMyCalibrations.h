#ifndef __MMPCEXAPPLYMYCALIBRATIONS_H__
#define __MMPCEXAPPLYMYCALIBRATIONS_H__

/**
 * @class  mMpcExApplyCalibrations
 * @author ngrau@augie.edu 
 * @date   July 2015
 * @brief  Apply the minipad-by-minipad calibrations
 */

#include <SubsysReco.h>
#include "MpcExConstants.h"
class PHCompositeNode;
class MpcExEventHeader;
class TRandom3; 
class TFile; 
class TH2D; 
class TH1D; 

class mMpcExApplyMyCalibrations : public SubsysReco {

 public:

  enum Mode {
    PEDESTAL_SUBTRACTED_ONLY = 0,
    COMPLETE = 1
  }; 

  //! Constructor
  mMpcExApplyMyCalibrations(float threshold_sigma=3.0);

  //! Constructor
  virtual ~mMpcExApplyMyCalibrations();

  //! Apply the minipad-by-minipad calibrations and fill a TMpcExCalibHitContainer
  int process_event(PHCompositeNode *topNode);

  int Init         (PHCompositeNode *topNode);
  int End          (PHCompositeNode *topNode);

 private:

  //
  int CellIDCheck(MpcExEventHeader *evt_head);

  //! internal array that determins if a chip locks up
  unsigned int _FailCellIDCheck[MpcExConstants::NARMS][MpcExConstants::NPACKETS_PER_ARM][MpcExConstants::NCHAINS_PER_PACKET][MpcExConstants::NCHIPS_PER_CHAIN];

  //! number of sigma above pedestal to make the threshold cut
  //! default is 3
  float _pedestal_threshold_sigma;

  // flag check for fixed calibrations
  int fixedCalibs; 

  // flag check for fixed calibrations
  Mode calibMode;

  // eliminate bad hits when calibrating
  int eliminateBad; 

  // flag to make histograms
  int makeHisto;

  // apply stack=1 cut
  int applyStackCut;

  TFile *outputfile; 
  TH2D *_histo_low; 
  TH2D *_histo_high; 
  TH2D *_histo_combined; 
  TH2D *_histo_lowADC; 
  TH2D *_histo_highADC; 
  TH1D *_histo_numHits_S; 
  TH1D *_histo_numHits_N; 

  // randome number generator
  TRandom3 *r3; 

};

#endif /* __MMPCEXCALIBRATEHITS_H__ */
