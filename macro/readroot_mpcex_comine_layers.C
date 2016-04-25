TH1D* hsensor_high[2][8][24];
TH1D* hsensor_low[2][8][24];
void readroot_mpcex(){
  gSystem->Load("libMyMpcEx.so");
  double max_sensor_x[2][8][24] = {{{0.}}};
  double max_sensor_y[2][8][24] = {{{0.}}};
  double min_sensor_x[2][8][24] = {{{0.}}};
  double min_sensor_y[2][8][24] = {{{0.}}};

  for(int iarm = 0;iarm < 2;iarm++){
    for(int ilayer = 0;ilayer < 8;ilayer++){
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
  MpcExMapper* mapper = MpcExMapper::instance();
  TFile* rfile = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728.root","READONLY");
  TH2D* hkey_adc_high = (TH2D*)rfile->Get("hkey_adc_high"); 
  TH2D* hkey_adc_low = (TH2D*)rfile->Get("hkey_adc_low");
  for(int i = 0;i < 50000;i++){
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
    double x = mapper->get_x(i);
    double y = mapper->get_y(i);
    hsensor_high[arm][layer][index]->Add(htemp0);
    hsensor_low[arm][layer][index]->Add(htemp1);
    
    if(max_sensor_x[arm][layer][index] < x) max_sensor_x[arm][layer][index] = x;
    if(max_sensor_y[arm][layer][index] < y) max_sensor_y[arm][layer][index] = y;
    if(min_sensor_x[arm][layer][index] > x) min_sensor_x[arm][layer][index] = x;
    if(min_sensor_y[arm][layer][index] > y) min_sensor_y[arm][layer][index] = y; 
  }
  

  TFile* rfile = new TFile("output_mpcex_low_layer_odd.root","RECREATE");
  for(int iarm = 0;iarm < 2;iarm++){
    char cname[100];
    sprintf(cname,"c_arm%d",iarm);
    TCanvas* c = new TCanvas(cname,cname,800,800);
//    for(int ilayer = 0;ilayer < 8;ilayer++){
//      char cname[100];
//      sprintf(cname,"c_arm%d_layer%d",iarm,ilayer);
//      TCanvas* c = new TCanvas(cname,cname,800,800);
      for(int index = 0;index < 24;index++){
	char pname[100];
//        sprintf(pname,"sensor_arm%d_layer%d_index%d",iarm,ilayer,index);
        sprintf(pname,"sensor_arm%d_index%d",iarm,index);
	double x0 = min_sensor_x[iarm][1][index];
	double x1 = max_sensor_x[iarm][1][index];
	double y0 = min_sensor_y[iarm][1][index];
	double y1 = max_sensor_y[iarm][1][index];
        ilayer = 1;
	if(ilayer%2 == 0){
	  y0 = y0 - 0.8;
	  y1 = y1 + 0.8;
	}
	if(ilayer%2 == 1){
          x0 = x0 - 0.8;
	  x1 = x1 + 0.8;
	}
	TPad* pad = new TPad(pname,pname,0.5+x0/40.,0.5+y0/40.,0.5+x1/40.,0.5+y1/40.);
        pad->cd();
	pad->SetLogy();
//	pad->SetGridx();
//	pad->SetGridy();
	for(int ilayer = 1;ilayer < 8;ilayer += 2){
	  hsensor_low[iarm][ilayer][index]->SetLineColor(ilayer/2+1);
	  hsensor_low[iarm][ilayer][index]->SetAxisRange(0,200,"X");
	  if(ilayer == 1) hsensor_low[iarm][ilayer][index]->Draw("");
	  else hsensor_low[iarm][ilayer][index]->Draw("same");
//        hsensor_low[iarm][ilayer][index]->SetLineColor(kRed);
//	hsensor_low[iarm][ilayer][index]->Draw("same");
//        pad->SetFillColor(kYellow);
	}
	c->cd();
	pad->DrawClone("same");
	delete pad;
      }
      rfile->cd();
      c->Write();
//    }//ilayer old
  }//arm
}
