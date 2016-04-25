#include<fstream>
using namespace std;

double X0[2][8][24];
double X1[2][8][24];
double Y0[2][8][24];
double Y1[2][8][24];
void readroot_mpcex_hl(){
  TFile* rfile = new TFile("AuAuAna_MinBias_NoCMN_Sub-450728.root","READONLY");
  if(!rfile){
    cout<<"Open AuAuAna_MinBias_NoCMN_Sub-450728.root failed !!!"<<endl;
    return;
  }
  ifstream intext("sensor_position.txt");
  string s;
  if(intext.is_open()){
    while(getline(intext,s)){
//      cout <<s<<endl;
      stringstream ss(s);
      int arm;
      int layer;
      int index;
      double x0;
      double x1;
      double y0;
      double y1;
      ss >> arm >> layer >> index
         >> x0 >> x1 >> y0 >>y1;
      X0[arm][layer][index] = x0;
      X1[arm][layer][index] = x1;
      Y0[arm][layer][index] = y0;
      Y1[arm][layer][index] = y1;
//      cout<<arm<<" "<<layer<<" "<<index<<" "
//          <<x0 <<" "<<x1<<" "<<y0<<" "<<y1<<endl;

//      cout << ss<<endl;
    }
  }
  else{
    cout <<"open test file failed !!!"<<endl;
    return;
  }

  TFile* ofile = new TFile("output_mpcex_layer_HL.root","RECREATE");
 
  for(int iarm = 0;iarm < 2;iarm++){
     for(int ilayer = 0;ilayer < 8;ilayer++){
       char cname[100];
       sprintf(cname,"c_arm%d_layer%d",iarm,ilayer);
       TCanvas* c = new TCanvas(cname,cname,800,800); 
      for(int index = 0;index < 24;index++){
        char hname[200];
        double x0 = X0[iarm][ilayer][index];
	double x1 = X1[iarm][ilayer][index];
	double y0 = Y0[iarm][ilayer][index];
	double y1 = Y1[iarm][ilayer][index];
cout<<iarm<<" "<<ilayer<<" "<<index<<" "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<endl;
	char pname[100];
        sprintf(pname,"sensor_arm%d_layer%d_index%d",iarm,ilayer,index);
	if(ilayer%2 == 0){
	  y0 = y0 - 0.85;
	  y1 = y1 + 0.85;
	}
	if(ilayer%2 == 1){
          x0 = x0 - 0.85;
	  x1 = x1 + 0.85;
	}

cout<<iarm<<" "<<ilayer<<" "<<index<<" "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<endl;
	TPad* pad = new TPad(pname,pname,0.5+x0/40.,0.5+y0/40.,0.5+x1/40.,0.5+y1/40.);
        pad->cd();
//	pad->SetFillColor(kRed);
	sprintf(hname,"hHL_sensor_arm%d_layer%d_sensor%d",iarm,ilayer,index);
        TH2D* htemp = (TH2D*)rfile->Get(hname);
	htemp->Draw("colz");
	c->cd();
	pad->DrawClone("same");
	delete pad;
      }
      ofile->cd();
      c->Write();
    }
  }
}
