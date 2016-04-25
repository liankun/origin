void Fun4All_MpcExDataAna(char* input_file_mpc ="",char* input_file_eve="",int runnumber = 0,int segment = 0){
  gSystem->Load("libfun4all");
  gSystem->Load("libuspin.so");
//  gSystem->Load("libmpcex_base.so");
//  gSystem->Load("libmpcex_interface.so");
//  gSystem->Load("libmpcex_modules.so");
  gSystem->Load("libMyMpcEx.so");
  gSystem->Load("libfun4allfuncs.so");
  gSystem->Load("libbbc");
  gSystem->Load("libt0");
  gSystem->Load("libmpc.so");
  gSystem->Setenv("ODBCINI","/opt/phenix/etc/odbc.ini.mirror");
  
  recoConsts* rc = recoConsts::instance();
  Fun4AllServer* se = Fun4AllServer::instance();

  //mpc reco part
  rc->set_IntFlag("MPC_RECO_MODE",0x16);
  rc->set_IntFlag("MPCEXCALIBMODE",0x1);
  rc->set_IntFlag("MPCEXCALIBAPPLYSTACK",1);//single buffer

//  SubsysReco *mpcreco = new MpcReco("MPCRECO");
//  se->registerSubsystem(mpcreco);
  
  SubsysReco* createNodeTree = new mMpcExCreateNodeTree();
  se->registerSubsystem(createNodeTree);

  SubsysReco* makeEvtQuality = new mMpcExMakeEventQuality();
  se->registerSubsystem(makeEvtQuality);

  SubsysReco* mpcexLoadCal = new mMpcExLoadMyCalibrations();
  se->registerSubsystem(mpcexLoadCal);

  SubsysReco* mpcexApplyCal = new mMpcExApplyMyCalibrations();
  se->registerSubsystem(mpcexApplyCal);

//  SubsysReco* mpcexShower = new mMpcExShower();
//  se->registerSubsystem(mpcexShower);

//  mMpcExLShower* mpcexLShower = new mMpcExLShower();
//  mpcexLShower->set_dead_map_path("BadChannelID.txt");
//  se->registerSubsystem(mpcexLShower);

  mMpcExDataAna* mpcexDataAna = new mMpcExDataAna();
//  mpcexDataAna->set_dead_map_path("BadChannelID.txt");
//  mpcexDataAna->set_mpc_tower_ped_path("work_2015_11_23/tower_pedestal.txt");
  se->registerSubsystem(mpcexDataAna);

//  SubsysReco* myshower = new mMpcExMyShower();
//  se->registerSubsystem(myshower);

//  Fun4AllInputManager* mpcex_dst_eve = new Fun4AllDstInputManager("DST_EVE","DST","TOP");
//  se->registerInputManager(mpcex_dst_eve);

//  Fun4AllDstInputManager* mpcex_dst_mpc = new Fun4AllDstInputManager("DST_MPC","DST","TOP");
//  se->registerInputManager(mpcex_dst_mpc);
  
  Fun4AllInputManager* mpcex_dst_mpcex = new Fun4AllDstInputManager("DST_MPCEX","DST","TOP");
  se->registerInputManager(mpcex_dst_mpcex);

  
  runnumber = 430013; //430013:3sigma,428713:5sigma
  
  rc->set_IntFlag("RUNNUMBER",runnumber);
  cout << "run number "<<runnumber<<endl;
 
//  MpcMap* map = MpcMap::instance();
//  map->Print();
  runnumber = 448911;
  for(int i = 0;i < 1;i++){
    
    char path_mpcex[200];    
    sprintf(path_mpcex,"/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/OwnProduction/run_0000448000_0000449000/DST_MPCEX_MB-0000%d-00%02d.root",runnumber,i);
    
    cout <<"open "<<path_mpcex<<endl;



//    se->fileopen(mpcex_dst_eve->Name(),path_eve);
//    se->fileopen(mpcex_dst_mpc->Name(),path_mpc);
    se->fileopen(mpcex_dst_mpcex->Name(),path_mpcex);


    se->run(0);
  }


  se->End();
  char output[100];
  sprintf(output,"DataAna_Mpc_triger-%d.root",runnumber);
  Fun4AllHistoManager* hm = se->getHistoManager("DataAna");
  if(hm) hm->dumpHistos(output);

  delete se;

  cout << "Completed reconstruction." <<endl;
}

