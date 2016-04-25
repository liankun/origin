#ifndef __MMPCEXMAKEEVENTQUALITY_H__
#define __MMPCEXMAKEEVENTQUALITY_H__

#ifndef __CINT__
#include <SubsysReco.h>
#endif
#include "vector"


class PHCompositeNode;


class mMpcExMakeEventQuality : public SubsysReco {
  public:
    mMpcExMakeEventQuality(const char* name = "MMPCEXMAKEEVENTQUALITY");
    virtual int Init(PHCompositeNode*);
    virtual int InitRun(PHCompositeNode*);
    virtual int process_event(PHCompositeNode*);
    virtual ~mMpcExMakeEventQuality();
    virtual int End(PHCompositeNode*);

  private:
    std::vector<int>_cell_id[2][8][48];

};

#endif /*__MMPCEXMAKEEVENTQUALITY_H__*/
