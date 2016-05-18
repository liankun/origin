#include<string>

void Fun4All_MpcExDataAna(char* input_file_mpcex ="",char* input_file_mpc="",char* input_file_eve){
  gSystem->Load("libfun4all");
  gSystem->Load("libuspin.so");
  gSystem->Load("libMyMpcEx.so");
  gSystem->Load("libfun4allfuncs.so");
  gSystem->Load("libbbc");
  gSystem->Load("libt0");
  gSystem->Load("libmpc.so");
  gSystem->Setenv("ODBCINI","/opt/phenix/etc/odbc.ini.mirror");
  
  recoConsts* rc = recoConsts::instance();
  Fun4AllServer* se = Fun4AllServer::instance();
  
  char ifile[5000];
  strcpy(ifile, input_file_mpcex);
  strtok(ifile, "-");
  int runnumber = atoi(strtok(0, "-"));
  int segment = atoi(strtok(strtok(0, "-"), "."));

  //mpc reco part
  rc->set_IntFlag("MPC_RECO_MODE",0x16);
  rc->set_IntFlag("MPCEXCALIBMODE",0x0);//0:only ped subtract,1: + CMN sub,2: compelete
  rc->set_IntFlag("MPCEXCALIBAPPLYSTACK",1);//single buffer
//  rc->set_IntFlag("MPCEX_CMN_SUBTRACT",1);

  SubsysReco *mpcreco = new MpcReco("MPCRECO");
  se->registerSubsystem(mpcreco);
  
  SubsysReco* createNodeTree = new mMpcExCreateNodeTree();
  se->registerSubsystem(createNodeTree);

  SubsysReco* makeEvtQuality = new mMpcExMakeEventQuality();
  se->registerSubsystem(makeEvtQuality);

  SubsysReco* mpcexLoadCal = new mMpcExLoadCalibrations();
  se->registerSubsystem(mpcexLoadCal);

  SubsysReco* mpcexApplyCal = new mMpcExApplyCalibrations();
  se->registerSubsystem(mpcexApplyCal);


  SubsysReco* AuAuAna = new mMpcExAuAuAna();
  se->registerSubsystem(AuAuAna);



  Fun4AllInputManager* mpcex_dst_eve = new Fun4AllDstInputManager("DST_EVE","DST","TOP");
  se->registerInputManager(mpcex_dst_eve);

  Fun4AllDstInputManager* mpcex_dst_mpc = new Fun4AllDstInputManager("DST_MPC","DST","TOP");
  se->registerInputManager(mpcex_dst_mpc);
  
  Fun4AllInputManager* mpcex_dst_mpcex = new Fun4AllDstInputManager("DST_MPCEX","DST","TOP");
  se->registerInputManager(mpcex_dst_mpcex);

  rc->set_IntFlag("RUNNUMBER",runnumber);
  cout << "run number "<<runnumber<<endl;
 
  se->fileopen(mpcex_dst_eve->Name(),input_file_eve);
  cout <<"open "<<input_file_eve<<endl;
  se->fileopen(mpcex_dst_mpc->Name(),input_file_mpc);
  cout <<"open "<<input_file_mpc<<endl;
  se->fileopen(mpcex_dst_mpcex->Name(),input_file_mpcex);
  cout <<"open "<<input_file_mpcex<<endl;

  se->run(0);

  se->End();
  char output[100];
  sprintf(output,"Run16Ana_MinBias_NoCMN_Sub-%d-%d.root",runnumber,segment);
  Fun4AllHistoManager* hm = se->getHistoManager("AuAuAna");
  if(hm) hm->dumpHistos(output);

  delete se;

  cout << "Completed reconstruction." <<endl;
}

