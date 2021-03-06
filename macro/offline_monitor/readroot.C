#include<vector>
#include<string>

void readroot(int runnumber = 454802){
  cout<<"Run Number: "<<runnumber<<endl;

  gSystem->Load("libMyMpcEx.so");
  char path[500];
  sprintf(path,"%d/sum.root",runnumber);
  TFile* ifile0 = new TFile(path,"READONLY");
  cout<<"Open "<<path<<endl;
  if(!ifile0){
    cout << "Open "<<path<<" failed"<<endl;
    exit(0);
  }
  sprintf(path,"%d/exo_sum_out.root",runnumber);
  TFile* ifile1 = new TFile(path,"READONLY");
  cout<<"Open "<<path<<endl;
  if(!ifile1){
    cout<<"Open "<<path<<" failed !"<<endl;
    exit(0);
  }


  TH2D* hkey_adc_high;
  TH2D* hkey_adc_low;
  TH2D* hkey_rawadc_high;
  TH2D* hkey_rawadc_low;

  TH2D* hHL_sensor[2][8][24];
  TH2D* hHL_sensor_raw[2][8][24];
  Exogram* hgrammy_high[2];
  Exogram* hgrammy_low[2];
  Exogram* hgrammy_combine[2];
  TH2D* hadc_mpc_e[2];
  TH2D* hlayer_adc_high[2];
  TH2D* hlayer_adc_low[2];
  TH2D* htower_e;
  TH2D* htower_tdc;

  TH2D* hbbc_adc[2];
  TH2D* hbbc_adc_low[2];
  TH2D* hbbc_nhits[2];

  char hname[500];
  vector<TH2D*> h2d_list;
  hkey_adc_high = (TH2D*)ifile0->Get("hkey_adc_high");
  h2d_list.push_back(hkey_adc_high);

  hkey_adc_low = (TH2D*)ifile0->Get("hkey_adc_low");
  h2d_list.push_back(hkey_adc_low);
  
  hkey_rawadc_high = (TH2D*)ifile0->Get("hkey_rawadc_high");
  h2d_list.push_back(hkey_rawadc_high);

  hkey_rawadc_low = (TH2D*)ifile0->Get("hkey_rawadc_low");
  h2d_list.push_back(hkey_rawadc_low);

  hbbc_nhits[0] = (TH2D*)ifile0->Get("hbbc_nhits_arm0");
  hbbc_nhits[1] = (TH2D*)ifile0->Get("hbbc_nhits_arm1");
  h2d_list.push_back(hbbc_nhits[0]);
  h2d_list.push_back(hbbc_nhits[1]);
  
  hbbc_adc[0] = (TH2D*)ifile0->Get("hbbc_adc_arm0");
  hbbc_adc[1] = (TH2D*)ifile0->Get("hbbc_adc_arm1");
  h2d_list.push_back(hbbc_adc[0]);
  h2d_list.push_back(hbbc_adc[1]);
  
  hbbc_adc_low[0] = (TH2D*)ifile0->Get("hbbc_adc_low_arm0");
  hbbc_adc_low[1] = (TH2D*)ifile0->Get("hbbc_adc_low_arm1");
  h2d_list.push_back(hbbc_adc_low[0]);
  h2d_list.push_back(hbbc_adc_low[1]);
  
  hlayer_adc_high[0] = (TH2D*)ifile0->Get("hlayer_adc_high_arm0");
  hlayer_adc_high[1] = (TH2D*)ifile0->Get("hlayer_adc_high_arm1");
  h2d_list.push_back(hlayer_adc_high[0]);
  h2d_list.push_back(hlayer_adc_high[1]);

  hlayer_adc_low[0] = (TH2D*)ifile0->Get("hlayer_adc_low_arm0");
  hlayer_adc_low[1] = (TH2D*)ifile0->Get("hlayer_adc_low_arm1");
  h2d_list.push_back(hlayer_adc_low[0]);
  h2d_list.push_back(hlayer_adc_low[1]);

  htower_e = (TH2D*)ifile0->Get("htower_e");
  h2d_list.push_back(htower_e);

  htower_tdc = (TH2D*)ifile0->Get("htower_tdc");
  h2d_list.push_back(htower_tdc);

  hadc_mpc_e[0] = (TH2D*)ifile0->Get("hadc_mpc_e_arm0");
  hadc_mpc_e[1] = (TH2D*)ifile0->Get("hadc_mpc_e_arm1");
  h2d_list.push_back(hadc_mpc_e[0]);
  h2d_list.push_back(hadc_mpc_e[1]);

  for(unsigned int i = 0;i < h2d_list.size();i++){
    string s = h2d_list[i]->GetName();
    stringstream ss("");
    ss<<s<<"_"<<runnumber;
    s=ss.str();
//    cout << s <<endl;
    TCanvas* c = new TCanvas(s.c_str(),s.c_str(),1200,800);
    c->SetLogz();
    h2d_list[i]->Draw("colz");
    ss.str("");
    ss << runnumber <<"/"<<s<<".gif";
    s=ss.str();
    c->Print(s.c_str(),"gif");
    delete c;
  }

  vector<Exogram*> hexo_list;
  hgrammy_high[0] = (Exogram*)ifile1->Get("mgrammy_high0");
  hgrammy_high[1] = (Exogram*)ifile1->Get("mgrammy_high1");
  hexo_list.push_back(hgrammy_high[0]);
  hexo_list.push_back(hgrammy_high[1]);

  hgrammy_low[0] = (Exogram*)ifile1->Get("mgrammy_low0");
  hgrammy_low[1] = (Exogram*)ifile1->Get("mgrammy_low1");
  hexo_list.push_back(hgrammy_low[0]);
  hexo_list.push_back(hgrammy_low[1]);

  hgrammy_combine[0] = (Exogram*)ifile1->Get("mgrammy_combine0");
  hgrammy_combine[1] = (Exogram*)ifile1->Get("mgrammy_combine1");
  hexo_list.push_back(hgrammy_combine[0]);
  hexo_list.push_back(hgrammy_combine[1]);
  
  for(unsigned int i = 0;i < hexo_list.size();i++){
    string s = hexo_list[i]->GetName();
    stringstream ss("");
    ss<<s<<"_"<<runnumber;
    s=ss.str();
    TCanvas* c = new TCanvas(s.c_str(),s.c_str(),1400,800);
    c->Divide(4,2);
    double max = hexo_list[i]->GetBinContent(hexo_list[i]->GetMaximumBin());
    for(unsigned int j = 0;j < 8;j++){
      c->cd(j+1);
      TPad *pd =(TPad*)c->cd(j+1);
      pd->SetLogz();
      hexo_list[i]->SetAxisRange(j,j,"z");
      TH2D* htemp = hexo_list[i]->Project3D("yx");
      string tmp_s = htemp->GetName();
      stringstream tmp_ss("");
      tmp_ss<<tmp_s<<"_"<<"layer"<<j;
      tmp_s = tmp_ss.str();
      htemp->SetName(tmp_s.c_str());
      htemp->SetMaximum(max);
      htemp->Draw("colz");
    }
    ss.str("");
    ss << runnumber <<"/"<<s<<".gif";
    s=ss.str();
    c->Print(s.c_str(),"gif");
    delete c;
  }

  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
      for(int isen = 0;isen< 24;isen++){
        char hname[500];
	sprintf(hname,"hHL_sensor_arm%d_layer%d_sensor%d",iarm,ilayer,isen);
	hHL_sensor[iarm][ilayer][isen] = (TH2D*)ifile0->Get(hname);
	sprintf(hname,"hHL_sensor_raw_arm%d_layer%d_sensor%d",iarm,ilayer,isen);
	hHL_sensor_raw[iarm][ilayer][isen] = (TH2D*)ifile0->Get(hname);
      }
    }
  }

  //sensor adc for each layer
  double max_sensor_x[2][8][24] = {{{0.}}};
  double max_sensor_y[2][8][24] = {{{0.}}};
  double min_sensor_x[2][8][24] = {{{0.}}};
  double min_sensor_y[2][8][24] = {{{0.}}};
  
  TH1D* hsensor_high[2][8][24];
  TH1D* hsensor_low[2][8][24];
  TH1D* hmpcex_layer_high[2][8];
  TH1D* hmpcex_layer_low[2][8];

  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char hname[500];
      sprintf(hname,"hmpcex_layer_high_arm%d_layer%d",iarm,ilayer);
      hmpcex_layer_high[iarm][ilayer] = new TH1D(hname,hname,300,-40.5,259.5);
      hmpcex_layer_high[iarm][ilayer]->GetXaxis()->SetTitle("High");
      sprintf(hname,"hmpcex_layer_low_arm%d_layer%d",iarm,ilayer);
      hmpcex_layer_low[iarm][ilayer] = new TH1D(hname,hname,300,-40.5,259.5);
      hmpcex_layer_low[iarm][ilayer]->GetXaxis()->SetTitle("low");
      for(int isen = 0;isen < 24;isen++){
        char name[100];
        sprintf(name,"hsensor_arm%d_layer%d_index%d_high",iarm,ilayer,isen);
        hsensor_high[iarm][ilayer][isen] = new TH1D(name,name,300,-40.5,259.5);
        sprintf(name,"hsensor_arm%d_layer%d_index%d_low",iarm,ilayer,isen);
        hsensor_low[iarm][ilayer][isen] = new TH1D(name,name,300,-40.5,259.5);
        max_sensor_x[iarm][ilayer][isen] = -9999;
        max_sensor_y[iarm][ilayer][isen] = -9999;
        min_sensor_x[iarm][ilayer][isen] = 9999;
        min_sensor_y[iarm][ilayer][isen] = 9999;
      }
    }
  }
  
  ifstream sensor_pos("sensor_position.txt");
  string s;
  while(getline(sensor_pos,s)){
    stringstream ss(s);
    int arm = 0;
    int layer = 0;
    int sensor = 0;
    double x0=0;
    double x1=0;
    double y0=0;
    double y1=0;
    ss>>arm>>layer>>sensor>>x0>>x1>>y0>>y1;
//    cout <<arm <<" "<<layer<<" "<<sensor<<" "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<endl;
    max_sensor_x[arm][layer][sensor] = x1;
    min_sensor_x[arm][layer][sensor] = x0;
    max_sensor_y[arm][layer][sensor] = y1;
    min_sensor_y[arm][layer][sensor] = y0;
  }
 
  MpcExMapper* mapper = MpcExMapper::instance(); 
  for(unsigned int i = 0;i < 50000;i++){
    hkey_adc_high->SetAxisRange(i,i,"X");
    hkey_adc_low->SetAxisRange(i,i,"X");
    TH1D* htemp0 = hkey_adc_high->ProjectionY();
    TH1D* htemp1 = hkey_adc_low->ProjectionY();
    if(htemp0->GetEntries() < 10) continue;
    int arm = mapper->get_arm(i);
    int quadrant = mapper->get_quadrant(i);
    int sensor = mapper->get_sensor_in_quadrant(i);
    int index = 6*quadrant+sensor;
    int layer = mapper->get_layer(i);
    hsensor_high[arm][layer][index]->Add(htemp0);
    hsensor_low[arm][layer][index]->Add(htemp1);
    hmpcex_layer_high[arm][layer]->Add(htemp0);
    hmpcex_layer_low[arm][layer]->Add(htemp1);
  }
  
  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char cname[100];
      sprintf(cname,"csensor_arm%d_layer%d_%d",iarm,ilayer,runnumber);
      TCanvas* c = new TCanvas(cname,cname,1400,800);
      for(int index = 0;index < 24;index++){
        char pname[100];
        sprintf(pname,"sensor_arm%d_layer%d_index%d",iarm,ilayer,index);
        double x0 = min_sensor_x[iarm][ilayer][index];
        double x1 = max_sensor_x[iarm][ilayer][index];
        double y0 = min_sensor_y[iarm][ilayer][index];
        double y1 = max_sensor_y[iarm][ilayer][index];
        if(ilayer%2 == 0){
          y0 = y0 - 0.85;
          y1 = y1 + 0.85;
        }
        if(ilayer%2 == 1){
          x0 = x0 - 0.85;
          x1 = x1 + 0.85;
        }
        TPad* pad = new TPad(pname,pname,0.5+x0/40.,0.5+y0/40.,0.5+x1/40.,0.5+y1/40.);
        pad->cd();
        pad->SetLogy();
	hsensor_high[iarm][ilayer][index]->Draw("");
	hsensor_low[iarm][ilayer][index]->SetLineColor(kRed);
        hsensor_low[iarm][ilayer][index]->Draw("same");
        c->cd();
        pad->DrawClone("same");
        delete pad;
      }
      sprintf(cname,"%d/csensor_arm%d_layer%d_%d.gif",runnumber,iarm,ilayer,runnumber);
      c->Print(cname,"gif"); 
      delete c;
    }
  }

  //mpcex layer high ADC
  for(int iarm = 0;iarm < 2;iarm++){
    char cname[500];
    sprintf(cname,"cmpcex_arm%d_high",iarm,ilayer);
    TCanvas* c = new TCanvas(cname,cname,1400,800);
    c->SetLogy();
    for(int ilayer = 0;ilayer < 8;ilayer++){
      hmpcex_layer_high[iarm][ilayer]->SetLineColor(ilayer+1);
      if(ilayer == 0) hmpcex_layer_high[iarm][ilayer]->Draw("");
      else hmpcex_layer_high[iarm][ilayer]->Draw("same");
    }
    sprintf(cname,"%d/cmpcex_arm%d_high_%d.gif",runnumber,iarm,runnumber);
    c->Print(cname,"gif");
    delete c;
  }

  //mpcex layer low ADC
  for(int iarm = 0;iarm < 2;iarm++){
    char cname[500];
    sprintf(cname,"cmpcex_arm%d_low",iarm,ilayer);
    TCanvas* c = new TCanvas(cname,cname,1400,800);
    c->SetLogy();
    for(int ilayer = 0;ilayer < 8;ilayer++){
      hmpcex_layer_low[iarm][ilayer]->SetLineColor(ilayer+1);
      if(ilayer == 0) hmpcex_layer_low[iarm][ilayer]->Draw("");
      else hmpcex_layer_low[iarm][ilayer]->Draw("same");
    }
    sprintf(cname,"%d/cmpcex_arm%d_low_%d.gif",runnumber,iarm,runnumber);
    c->Print(cname,"gif");
    delete c;
  }

  //mpcex high low ratio
  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char cname[100];
      sprintf(cname,"csensor_HL_arm%d_layer%d_%d",iarm,ilayer,runnumber);
      TCanvas* c = new TCanvas(cname,cname,1400,800);
      for(int index = 0;index < 24;index++){
        char pname[100];
        sprintf(pname,"psensor_HL_arm%d_layer%d_index%d",iarm,ilayer,index);
        double x0 = min_sensor_x[iarm][ilayer][index];
        double x1 = max_sensor_x[iarm][ilayer][index];
        double y0 = min_sensor_y[iarm][ilayer][index];
        double y1 = max_sensor_y[iarm][ilayer][index];
        if(ilayer%2 == 0){
          y0 = y0 - 0.85;
          y1 = y1 + 0.85;
        }
        if(ilayer%2 == 1){
          x0 = x0 - 0.85;
          x1 = x1 + 0.85;
        }
        TPad* pad = new TPad(pname,pname,0.5+x0/40.,0.5+y0/40.,0.5+x1/40.,0.5+y1/40.);
        pad->cd();
        pad->SetLogy();
        hHL_sensor[iarm][ilayer][index]->Draw("colz");        
        c->cd();
	pad->DrawClone("same");
	delete pad;
      }
      sprintf(cname,"%d/csensor_HL_arm%d_layer%d_%d.gif",runnumber,iarm,ilayer,runnumber);
      c->Print(cname,"gif"); 
      delete c;
    }
  }  
  
  //mpc tower
  MpcMap* mpcmap;
  mpcmap = MpcMap::instance();
  for(int iarm = 0;iarm < 2;iarm++){
    char cname[100];
    sprintf(cname,"c_tower_arm%d_%d",iarm,runnumber);
    TCanvas* c = new TCanvas(cname,cname,1400,800);
    for(unsigned int i = 0;i < 288;i++ ){
      float tx = mpcmap->getX(i+288*iarm);
      float ty = mpcmap->getY(i+288*iarm);
      float tz = mpcmap->getZ(i+288*iarm);
      if(fabs(tz) < 2) continue;
      htower_e->SetAxisRange(i,i,"x");
      htower_e->SetAxisRange(0,20,"y");
      TH1D* htemp_tower = htower_e->ProjectionY();
      double entries = htemp_tower->GetEntries();
//      if(entries < 10) continue;
      char pname[200];
      sprintf(pname,"tower_%d_%d",iarm,i);
      htemp_tower->SetName(pname);
      sprintf(pname,"tower_arm%d_%d",iarm,i);
      TPad* pad = new TPad(pname,pname,0.5+(tx-1.0)/48.,0.5+(ty-1.0)/48.,0.5+(tx+1.0)/48.,0.5+(ty+1.0)/48.);
      pad->cd();
      pad->SetLogy();
      htemp_tower->Draw("");
      c->cd();
      pad->DrawClone("same");
      delete pad;
    }
    sprintf(cname,"%d/ctower_arm%d_%d.gif",runnumber,iarm,runnumber);
    c->Print(cname,"gif"); 
    delete c;
  }

  //mpc eta phi
  TH2D* htower_eta_phi[2];
  TH2D* htower_eta_phi[0] = new TH2D("htower_eta_phi0","Tower Eta vs Phi Arm 0",20,-3.1416,3.1416,10,3,4);
  htower_eta_phi[0]->GetXaxis()->SetTitle("Phi");
  htower_eta_phi[0]->GetYaxis()->SetTitle("|Eta|");

  TH2D* htower_eta_phi[1] = new TH2D("htower_eta_phi1","Tower Eta vs Phi Arm 1",20,-3.1416,3.1416,10,3,4);
  htower_eta_phi[1]->GetXaxis()->SetTitle("Phi");
  htower_eta_phi[1]->GetYaxis()->SetTitle("|Eta|");

  for(int iarm = 0;iarm < 2;iarm++){
//cout<<"iarm "<<iarm<<endl;    
    for(unsigned int i = 0;i < 288;i++ ){
//cout <<i+288*iarm<<endl;
      float tx = mpcmap->getX(i+288*iarm);
      float ty = mpcmap->getY(i+288*iarm);
      float tz = mpcmap->getZ(i+288*iarm);
      if(fabs(tz) < 2) continue;
//cout<<"tx "<<tx<<" ty "<<ty<<" tz "<<tz<<endl;
      float r = sqrt(tx*tx+ty*ty);
      float phi = atan2(ty,tx);
      double theta = atan(r/fabs(tz));
      double eta = -log(tan(theta/2.));
//cout<<"phi: "<<phi<<" eta: "<<fabs(eta)<<endl;
      htower_e->SetAxisRange(i,i,"x");
      TH1D* htemp_tower = htower_e->ProjectionY();
      double means = htemp_tower->GetMean();
      htower_eta_phi[iarm]->Fill(phi,fabs(eta),means); 
    }
    sprintf(cname,"ctower_eta_phi_arm%d_%d",iarm,runnumber);
    TCanvas* c = new TCanvas(cname,cname,1400,800);
    htower_eta_phi[iarm]->Draw("colz");
    sprintf(cname,"%d/ctower_eta_phi_arm%d_%d.gif",runnumber,iarm,runnumber);
    c->Print(cname,"gif");
    delete c;
  }

}
