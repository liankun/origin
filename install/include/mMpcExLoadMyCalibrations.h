#ifndef __MMPCEXLOADMYCALIBRATIONS_H__
#define __MMPCEXLOADMYCALIBRATIONS_H__

/**
 * @class  mMpcExLoadCalibration 
 * @author ngrau@augie.edu
 * @date   July 2015
 * @brief  This SubsysReco reads calibration data and fills the in-memory TMpcExCalibContainer
 */

#include <SubsysReco.h>
class PHCompositeNode;
class TMpcExCalibContainer;

class mMpcExLoadMyCalibrations : public SubsysReco {

 public:

  //! Constructor
  mMpcExLoadMyCalibrations();

  //! Destructor
  virtual ~mMpcExLoadMyCalibrations();

  //! Read the calibration data for a given run 
  int InitRun(PHCompositeNode *topNode);

  //! Because the calibrations persist through all events in a run, clear them at the end
  int End(PHCompositeNode *topNode);

};

#endif /* __MMPCEXLOADCALIBRATIONS_H__ */
