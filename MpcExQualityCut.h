#ifndef __MPCEXQUALITYCUT_H__
#define __MPCEXQUALITYCUT_H__

#include "MpcExEventHeader.h"
#include "Fun4AllServer.h"
#include "PHIODataNode.h"
#include "getClass.h"
#include "TriggerHelper.h"
#include "iostream"
#include "BbcOut.h"
#include "PHGlobal.h"

namespace MpcExQualityFunction{

inline bool IsSingleBuffered(){
    Fun4AllServer* se = Fun4AllServer::instance();
    PHCompositeNode* topNode = se->topNode();
    MpcExEventHeader* _mpcex_eventheader = findNode::getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");    
    if(!_mpcex_eventheader){
      std::cout <<"Get MpcExEventHeader failed !!!"<<std::endl;
      return -1;
    }
    int multibuffer = _mpcex_eventheader->getStack();
    if(multibuffer == 1) return true;
    else return false;
  }

inline bool IsTriggerWanted(){
    Fun4AllServer* se = Fun4AllServer::instance();
    PHCompositeNode* topNode = se->topNode();
    TriggerHelper* myTH = new TriggerHelper(topNode);
    int fire_minbias = myTH->trigScaled("BBCLL1(>0 tubes)");
    int fire_mpcb_N = myTH->trigScaled("MPC_N_B");
    int fire_mpcb_S = myTH->trigScaled("MPC_S_B");

    int fire_mpca_N = myTH->trigScaled("MPC_N_A");
    int fire_mpca_S = myTH->trigScaled("MPC_S_A");

    int fire_mpcpi0_N = myTH->trigScaled("MPC_N_C&MPC_N_C");
    int fire_mpcpi0_S = myTH->trigScaled("MPC_S_C$MPC_S_C");
    

    delete myTH;

    if(!fire_minbias && !(fire_mpcb_N || fire_mpcb_S) && !(fire_mpcpi0_N || fire_mpcpi0_S) && !(fire_mpca_N || fire_mpca_S)) return false;
    else return true;

  }

inline bool IsEventWanted(){
    Fun4AllServer* se = Fun4AllServer::instance();
    PHCompositeNode* topNode = se->topNode();
    PHGlobal* phglobal = findNode::getClass<PHGlobal>(topNode,"PHGlobal");
    BbcOut* bbcout = findNode::getClass<BbcOut>(topNode,"BbcOut");
    if(!bbcout && !phglobal){
      std::cout <<"No BbcOut or PHGlobal !!!"<<std::endl;
      return -1;
    }
    double _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
    if(fabs(_vertex) > 30.) return false;
    if(!IsSingleBuffered()) return false;
    if(!IsTriggerWanted()) return false;
    
    return true;
  }

}

#endif /*__MPCEXQUALITYCUT_H__*/
