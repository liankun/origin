#ifndef __MMPCEXLOADCALIBRATIONS_H__
#define __MMPCEXLOADCALIBRATIONS_H__

/**
 * @class  mMpcExLoadCalibration 
 * @author ngrau@augie.edu
 * @date   July 2015
 * @brief  This SubsysReco reads calibration data and fills the in-memory TMpcExCalibContainer
 */

#include <SubsysReco.h>
class PHCompositeNode;
class TMpcExCalibContainer;

class mMpcExLoadCalibrations : public SubsysReco {

 public:

  //! Constructor
  mMpcExLoadCalibrations();

  //! Destructor
  virtual ~mMpcExLoadCalibrations();

  //! Read the calibration data for a given run 
  int InitRun(PHCompositeNode *topNode);

  //! Because the calibrations persist through all events in a run, clear them at the end
  int EndRun(PHCompositeNode *topNode);

};

#endif /* __MMPCEXLOADCALIBRATIONS_H__ */
