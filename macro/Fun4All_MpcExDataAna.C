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

//  mMpcExDataAna* mpcexDataAna = new mMpcExDataAna();
//  mpcexDataAna->set_dead_map_path("BadChannelID.txt");
//  mpcexDataAna->set_mpc_tower_ped_path("work_2015_11_23/tower_pedestal.txt");
//  se->registerSubsystem(mpcexDataAna);

//  mMpcExLShower* mpcexLShower = new mMpcExLShower();
// mpcexLShower->set_dead_map_path("BadChannelID.txt");
//  se->registerSubsystem(mpcexLShower);

//  SubsysReco* mpcexShower = new mMpcExShower();
//  se->registerSubsystem(mpcexShower);

  SubsysReco* AuAuAna = new mMpcExAuAuAna();
  se->registerSubsystem(AuAuAna);



  Fun4AllInputManager* mpcex_dst_eve = new Fun4AllDstInputManager("DST_EVE","DST","TOP");
  se->registerInputManager(mpcex_dst_eve);

  Fun4AllDstInputManager* mpcex_dst_mpc = new Fun4AllDstInputManager("DST_MPC","DST","TOP");
  se->registerInputManager(mpcex_dst_mpc);
  
  Fun4AllInputManager* mpcex_dst_mpcex = new Fun4AllDstInputManager("DST_MPCEX","DST","TOP");
  se->registerInputManager(mpcex_dst_mpcex);

  
//  runnumber = 448911; //430013:3sigma,428713:5sigma
//  runnumber = 450728;
  runnumber = 454785;
  rc->set_IntFlag("RUNNUMBER",runnumber);
  cout << "run number "<<runnumber<<endl;
 
//  MpcMap* map = MpcMap::instance();
//  map->Print();
  for(int i = 0;i < 1;i++){
    
//    char path_mpcex[200];    
//    sprintf(path_mpcex,"/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/OwnProduction/run_0000448000_0000449000/DST_MPCEX_MB-0000%d-00%02d.root",runnumber,i);

//    char path_mpcex[200] ="/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/online_production/John_production/DST_MPCEX_MB-0000454664-0000.root";
      char path_mpcex[200];
      sprintf(path_mpcex,"/gpfs/mnt/gpfs02/phenix/mpcex/online_production/run16_mpcex_mpcex/run_0000454000_0000455000/DST_MPCEX/DST_MPCEX_run16_mpcex_mpcex-0000%d-00%02d.root",runnumber,i);

    cout <<"open "<<path_mpcex<<endl;
    char path_mpc[200];
    sprintf(path_mpc,"/gpfs/mnt/gpfs02/phenix/mpcex/online_production/run16_mpcex_mpcex/run_0000454000_0000455000/DST_MPC/DST_MPC_run16_mpcex_mpcex-0000%d-00%02d.root",runnumber,i);
    cout <<"open "<<path_mpc<<endl;
    char path_eve[200];
    sprintf(path_eve,"/gpfs/mnt/gpfs02/phenix/mpcex/online_production/run16_mpcex_mpcex/run_0000454000_0000455000/DST_EVE/DST_EVE_run16_mpcex_mpcex-0000%d-00%02d.root",runnumber,i);
    cout <<"open "<<path_eve<<endl;
    se->fileopen(mpcex_dst_eve->Name(),path_eve);
    se->fileopen(mpcex_dst_mpc->Name(),path_mpc);
    se->fileopen(mpcex_dst_mpcex->Name(),path_mpcex);


    se->run(0);
  }


  se->End();
  char output[100];
  sprintf(output,"AuAuAna_MinBias_NoCMN_Sub-%d.root",runnumber);
//  sprintf(output,"AuAuAna_UltraPeriphMPC_CMN_Sub_test-%d.root",runnumber);
  Fun4AllHistoManager* hm = se->getHistoManager("AuAuAna");
  if(hm) hm->dumpHistos(output);

  delete se;

  cout << "Completed reconstruction." <<endl;
}

