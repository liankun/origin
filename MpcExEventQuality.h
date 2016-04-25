#ifndef __MPCEXEVENTQUALITY_H__
#define __MPCEXEVENTQUALITY_H__
/**
 * class Event Quality
 * Liankun Zou @2015 July 28
 */
#include "PHObject.h"


class MpcExEventQuality : public PHObject {
  public:
    MpcExEventQuality();
    virtual ~MpcExEventQuality();
    void setTriggerWanted(bool tof){_trigger_wanted = tof;}
    void setVertexWanted(bool tof){_vertex_wanted = tof;}
    void setSingleBufferred(bool tof){_single_bufferred = tof;}
    void setCellIDUnLocked(int arm,int packet,int index,bool tof);
    void setCellIDDominated(int arm,int packet,int index,bool tof);
    void setCellIDGood(int arm,int packet,int index,bool tof);
    void setEventWanted(bool tof){_event_wanted = tof;}

    bool IsEventWanted(){return _event_wanted;}
    bool IsTriggerWanted(){return _trigger_wanted;}
    bool IsVertexWanted(){return _vertex_wanted;}
    bool IsSingleBufferred(){return _single_bufferred;}
    bool IsCellIDUnLocked(int arm,int packet,int index);
    bool IsCellIDDominated(int arm,int packet,int index);
    bool IsCellIDGood(int arm,int packet,int index);

    void Reset();
 
   private:
     bool _trigger_wanted;
     bool _vertex_wanted;
     bool _single_bufferred;
     bool _cell_id_locked[2][8][48];
     bool _cell_id_dominated[2][8][48];
     bool _cell_id_good[2][8][48];
     bool _event_wanted;
    
};

#endif /*__MPCEXEVENTQUALITY_H__*/
