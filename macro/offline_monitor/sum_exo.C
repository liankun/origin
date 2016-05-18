
void sum_exo(int runnumber = 454808){
  gSystem->Load("libMyMpcEx.so");
  char name[500];
  Exogram* hgrammy_high[2];
  Exogram* hgrammy_low[2];
  Exogram* hgrammy_combine[2];

  hgrammy_high[0] = new Exogram("mgrammy_high0","Exogram high arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);

  hgrammy_high[1] = new Exogram("mgrammy_high1","Exogram high arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);

  hgrammy_low[0] = new Exogram("mgrammy_low0","Exogram low arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);
  
  hgrammy_low[1] = new Exogram("mgrammy_low1","Exogram low arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);

  hgrammy_combine[0] = new Exogram("mgrammy_combine0","Exogram Combine arm 0",900,-24,24,900,-24,24,8,-0.5,7.5);

  hgrammy_combine[1] = new Exogram("mgrammy_combine1","Exogram Combine arm 1",900,-24,24,900,-24,24,8,-0.5,7.5);
  
  for(int i = 0;i < 9;i++){
    sprintf(name,"%d/Run16Ana_MinBias_NoCMN_Sub-%d-%d.root",runnumber,runnumber,i);
    cout<<"Name: "<<name<<endl; 
    TFile* infile = new TFile(name,"READONLY");
    if(!infile) continue;
//    infile->Print();    
    Exogram* htemp = (Exogram*)infile->Get("hgrammy_high0");
    if(!htemp) continue; 
    hgrammy_high[0]->Add(htemp);

    htemp = (Exogram*)infile->Get("hgrammy_high1");
    if(!htemp) continue;
    hgrammy_high[1]->Add(htemp);

    htemp = (Exogram*)infile->Get("hgrammy_low0");
    if(!htemp) continue;
    hgrammy_low[0]->Add(htemp);

    htemp = (Exogram*)infile->Get("hgrammy_low1");
    if(!htemp) continue;
    hgrammy_low[1]->Add(htemp);

    htemp = (Exogram*)infile->Get("hgrammy_combine0");
    if(!htemp) continue;
    hgrammy_combine[0]->Add(htemp);

    htemp = (Exogram*)infile->Get("hgrammy_combine1");
    if(!htemp) continue;
    hgrammy_combine[1]->Add(htemp);
    
    infile->Close();
  }
  
  sprintf(name,"%d/exo_sum_out.root",runnumber);
  TFile* exo_sum_out = new TFile(name,"RECREATE");
  for(int iarm = 0;iarm < 2;iarm++){
    hgrammy_high[iarm]->Write();
    hgrammy_low[iarm]->Write();
    hgrammy_combine[iarm]->Write();
  }
}
