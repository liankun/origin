//compare CMN and NOCMN
TH1D* hsensor_high[2][8][24];
TH1D* hsensor_low[2][8][24];

TH1D* hsensor_high_cmn[2][8][24];
TH1D* hsensor_low_cmn[2][8][24];

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
        
	sprintf(name,"hsensor_arm%d_layer%d_index%d_high_cmn",iarm,ilayer,isen);
        hsensor_high_cmn[iarm][ilayer][isen] = new TH1D(name,name,300,-40.5,259.5);
      	sprintf(name,"hsensor_arm%d_layer%d_index%d_low_cmn",iarm,ilayer,isen);
        hsensor_low_cmn[iarm][ilayer][isen] = new TH1D(name,name,300,-40.5,259.5);

	
	max_sensor_x[iarm][ilayer][isen] = -9999;
        max_sensor_y[iarm][ilayer][isen] = -9999;
        min_sensor_x[iarm][ilayer][isen] = 9999;
        min_sensor_y[iarm][ilayer][isen] = 9999;
      }
    }
  }
  MpcExMapper* mapper = MpcExMapper::instance();
  TFile* rfile0 = new TFile("AuAuAna_MinBias_CMN_Sub-448911.root","READONLY");
  TH2D* hkey_adc_high0 = (TH2D*)rfile0->Get("hkey_adc_high"); 
  hkey_adc_high0->SetName("hkey_adc_high_cmn");
  TH2D* hkey_adc_low0 = (TH2D*)rfile0->Get("hkey_adc_low");
  hkey_adc_low0->SetName("hkey_adc_low_cmn");

  TFile* rfile1 = new TFile("AuAuAna_MinBias_NoCMN_Sub-448911.root","READONLY");
  TH2D* hkey_adc_high1 = (TH2D*)rfile1->Get("hkey_adc_high");
  TH2D* hkey_adc_low1 = (TH2D*)rfile1->Get("hkey_adc_low");

  for(int i = 0;i < 50000;i++){
    hkey_adc_high0->SetAxisRange(i,i,"X");
    hkey_adc_low0->SetAxisRange(i,i,"X");
    hkey_adc_high1->SetAxisRange(i,i,"X");
    hkey_adc_low1->SetAxisRange(i,i,"X");

    TH1D* htemp00 = hkey_adc_high0->ProjectionY();
    TH1D* htemp01 = hkey_adc_low0->ProjectionY();
   
    TH1D* htemp10 = hkey_adc_high1->ProjectionY();
    TH1D* htemp11 = hkey_adc_low1->ProjectionY();

    
    if(htemp00->GetEntries() < 10) continue;
    int arm = mapper->get_arm(i);
    int quadrant = mapper->get_quadrant(i);
    int sensor = mapper->get_sensor_in_quadrant(i);
    int index = 6*quadrant+sensor;
    int layer = mapper->get_layer(i);
    double x = mapper->get_x(i);
    double y = mapper->get_y(i);
    hsensor_high[arm][layer][index]->Add(htemp10);
    hsensor_low[arm][layer][index]->Add(htemp11);
    hsensor_high_cmn[arm][layer][index]->Add(htemp00);
    hsensor_low_cmn[arm][layer][index]->Add(htemp01);


    
    if(max_sensor_x[arm][layer][index] < x) max_sensor_x[arm][layer][index] = x;
    if(max_sensor_y[arm][layer][index] < y) max_sensor_y[arm][layer][index] = y;
    if(min_sensor_x[arm][layer][index] > x) min_sensor_x[arm][layer][index] = x;
    if(min_sensor_y[arm][layer][index] > y) min_sensor_y[arm][layer][index] = y; 
  }
  

  TFile* rfile = new TFile("output_mpcex_low_cmn_vs_no_cmn_layer.root","RECREATE");
  for(int iarm = 0;iarm < 2;iarm++){
//    char cname[100];
//    sprintf(cname,"c_arm%d",iarm);
//    TCanvas* c = new TCanvas(cname,cname,800,800);
    for(int ilayer = 0;ilayer < 8;ilayer++){
      char cname[100];
      sprintf(cname,"c_arm%d_layer%d",iarm,ilayer);
      TCanvas* c = new TCanvas(cname,cname,800,800);
      for(int index = 0;index < 24;index++){
	char pname[100];
        sprintf(pname,"sensor_arm%d_layer%d_index%d",iarm,ilayer,index);
//        sprintf(pname,"sensor_arm%d_index%d",iarm,index);
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
//	pad->SetGridx();
//	pad->SetGridy();
	hsensor_low_cmn[iarm][ilayer][index]->SetLineColor(kRed);
	hsensor_low_cmn[iarm][ilayer][index]->Draw("");

	hsensor_low[iarm][ilayer][index]->Draw("same");
	c->cd();
	pad->DrawClone("same");
	delete pad;
      }
      rfile->cd();
      c->Write();
    }//ilayer old
  }//arm
}
