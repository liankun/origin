#include "mMpcExMakeEventQuality.h"
#include "MpcExEventHeader.h"
#include "recoConsts.h"
#include "PHIODataNode.h"
#include "getClass.h"
#include "PHGlobal.h"
#include "Fun4AllReturnCodes.h"
#include "MpcExEventQuality.h"
#include "iostream"
#include "cstdlib"
#include "BbcOut.h"
#include "TriggerHelper.h"

using namespace std;
using namespace findNode;

mMpcExMakeEventQuality::mMpcExMakeEventQuality(const char* name) :
  SubsysReco(name)
{
  for(int arm = 0;arm < 2;arm++){
    for(int packet = 0;packet < 8;packet++){
      for(int index = 0;index < 48;index++){
        _cell_id[arm][packet][index].clear();
      }
    }
  }
}

mMpcExMakeEventQuality::~mMpcExMakeEventQuality(){

}

int mMpcExMakeEventQuality::Init(PHCompositeNode* topNode){
  return EVENT_OK;
}

int mMpcExMakeEventQuality::InitRun(PHCompositeNode* topNode){
  PHNodeIterator nodeIter(topNode);
  PHCompositeNode* mpcexNode = static_cast<PHCompositeNode*>(nodeIter.findFirst("PHCompositeNode","MPCEX"));
  if(mpcexNode == NULL){
    cout <<"MPCEX Node does not exist!!!"<<endl;
    exit(1);
  }
  MpcExEventQuality* evtQuality = new MpcExEventQuality();
  PHIODataNode<MpcExEventQuality>* evtQualityNode = new PHIODataNode<MpcExEventQuality>(evtQuality,"MpcExEventQuality","PHObject");
  mpcexNode->addNode(evtQualityNode);
  return EVENT_OK;
}

int mMpcExMakeEventQuality::process_event(PHCompositeNode* topNode){  
  MpcExEventHeader* _mpcex_eventheader = findNode::getClass<MpcExEventHeader>(topNode,"MpcExEventHeader");
  if(!_mpcex_eventheader){
    cout << "No MpcExEventHeader !!!"<<endl;
    exit(1);
  }
  MpcExEventQuality* _evt_quality = findNode::getClass<MpcExEventQuality>(topNode,"MpcExEventQuality");
  if(!_evt_quality){
    cout << "Get MpcExEventQuality failed !!!"<<endl;
    exit(1);
  }
  _evt_quality->Reset();
//single buffer  
  int multibuffer = _mpcex_eventheader->getStack();
  
  if(multibuffer == 1) _evt_quality->setSingleBufferred(true);
  else _evt_quality->setSingleBufferred(false);

//trigger
   TriggerHelper* myTH = new TriggerHelper(topNode);
   int fire_minbias = myTH->trigScaled("BBCLL1(>0 tubes)");
   int fire_mpcb_N = myTH->trigScaled("MPC_N_B");
   int fire_mpcb_S = myTH->trigScaled("MPC_S_B");

   int fire_mpca_N = myTH->trigScaled("MPC_N_A");
   int fire_mpca_S = myTH->trigScaled("MPC_S_A");

   int fire_mpcpi0_N = myTH->trigScaled("MPC_N_C&MPC_N_C");
   int fire_mpcpi0_S = myTH->trigScaled("MPC_S_C$MPC_S_C");
    
   delete myTH;

   if(!fire_minbias && !(fire_mpcb_N || fire_mpcb_S) && !(fire_mpcpi0_N || fire_mpcpi0_S) && !(fire_mpca_N || fire_mpca_S)) _evt_quality->setTriggerWanted(false);
   else _evt_quality->setTriggerWanted(true);

//vertex
  PHGlobal* phglobal = getClass<PHGlobal>(topNode,"PHGlobal");
  BbcOut* bbcout = getClass<BbcOut>(topNode,"BbcOut");
  if(!bbcout && !phglobal){
    cout <<"No BbcOut or PHGlobal !!!"<<endl;
    exit(1);
  }
  double _vertex = (phglobal==0) ? phglobal->getBbcZVertex() : bbcout->get_VertexPoint();
  if(fabs(_vertex) > 30.) _evt_quality->setVertexWanted(false);
  else _evt_quality->setVertexWanted(true);

  bool sbuf = _evt_quality->IsSingleBufferred();
  bool trgw = _evt_quality->IsTriggerWanted();
  bool vtxw = _evt_quality->IsVertexWanted();
  if(sbuf && trgw && vtxw) _evt_quality->setEventWanted(true);

//cell ID cut
//dominate Cell ID
 
  int cell_id_size = _mpcex_eventheader->getCellIDsSize();
  vector<int> main_cellid;
  for(int i = 0;i < cell_id_size;i++){
    int arm = _mpcex_eventheader->getCellIDsArm(i);
//    cout <<"arm "<<arm<<endl;
    int packet = _mpcex_eventheader->getCellIDsPkt(i);
//    cout <<"packet "<<packet<<endl;
    int svx4_id = _mpcex_eventheader->getCellIDsSVXID(i);
//    cout <<"svx4_id "<<svx4_id <<endl;
    unsigned int cell_id_value = _mpcex_eventheader->getCellIDsValue(i);
//    cout <<arm<<" "<<packet<<" "<<svx4_id<<" Cell ID value: "<<cell_id_value<<endl;
    if(cell_id_value >0 && cell_id_value < 48) _evt_quality->setCellIDGood(arm,packet,svx4_id,true);
    else _evt_quality->setCellIDGood(arm,packet,svx4_id,false);
    _cell_id[arm][packet][svx4_id].push_back(cell_id_value);
    main_cellid.push_back(cell_id_value);
    if(main_cellid.size() == 12){
      int stat[12] = {0};
      for(unsigned int j = 0;j < main_cellid.size();j++){
        int value = main_cellid[j];
	for(unsigned int k = 0; k < main_cellid.size();k++){
          if(main_cellid[k] == value) stat[j]++;
	}
      }
      int max_index = 0;
      int max = 0;
      for(int n = 0;n < 12 ;n++){
	if(stat[n] > max){ 
	  max_index = n;
	  max = stat[n];
	} 
      }
      int max_value = main_cellid[max_index];
//      cout <<"Dominate Cell ID: "<<max_value<<endl;
      for(int ii = 0;ii < 12 ;ii++){
        if(main_cellid[ii] == max_value){ 
          _evt_quality->setCellIDDominated(arm,packet,svx4_id-11+ii,true);
//	  cout <<"SVX4 "<<svx4_id-11+ii<<" true"<<endl;
	}
	else{
	  _evt_quality->setCellIDDominated(arm,packet,svx4_id-11+ii,false);
//	  cout <<"SVX4 "<<svx4_id-11+ii<<" false"<<endl;
	}
      }
      main_cellid.clear();
    }
  }

 
//cell id cut
  for(int arm = 0;arm <2;arm ++){
    for(int packet = 0;packet < 8; packet++){
      for(int index = 0;index < 48; index++){
        if(_cell_id[arm][packet][index].size() > 4){
          int last_value = _cell_id[arm][packet][index].back();
	  unsigned int count = 0;
	  for(unsigned size_i = 0;size_i < _cell_id[arm][packet][index].size();size_i++){
            if(last_value == _cell_id[arm][packet][index][size_i]){
	      count++;
	    }
	  }
          if(count == _cell_id[arm][packet][index].size()){
	    _evt_quality->setCellIDUnLocked(arm,packet,index,false);
//	    cout <<arm<<" "<<packet<<" "<<index<<" locked"<<" "<<last_value<<endl;
	  }
	  else _evt_quality->setCellIDUnLocked(arm,packet,index,true);

          _cell_id[arm][packet][index].erase(_cell_id[arm][packet][index].begin()); 
	}
        else _evt_quality->setCellIDUnLocked(arm,packet,index,true);
      }
    }
  }
  return EVENT_OK;
}

int mMpcExMakeEventQuality::End(PHCompositeNode* topNode){
  return EVENT_OK;
}
