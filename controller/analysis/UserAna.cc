#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TApplication.h>
#include <TMath.h>
#include <TStyle.h>
#include <iostream>
#include <fstream>
#include <cmath>

#include "Tree.h"
#include "Hist.h"
using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++
double VoigtFunc(Double_t *x, Double_t *par) {
  return par[0] * TMath::Voigt(x[0] - par[1], par[2], par[3]);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
double PoisLorentzFunc(Double_t *x, Double_t *par) {
  double offset = par[0];
  double mu     = par[1];
  double sigma  = par[2];
  double gamma  = par[3];
  double fano   = par[4];
  double scale  = par[5];
  double linear = par[6];
  double norm   = par[7];

  Double_t xx = x[0];
  Double_t sum = 0.0;

  sum += TMath::Poisson(0, mu) * TMath::Voigt(xx - offset, sigma, gamma);
  sum += TMath::Poisson(1, mu) * TMath::Voigt(xx - offset - scale, sigma, gamma);

  for (Int_t k = 2; k <= 20; ++k) {
    Double_t pk = TMath::Poisson(k, mu);
    if (pk < 1e-6) continue;

    Double_t mean_k = std::pow(k, linear) * (scale - offset);
    Double_t sigma_k = sigma * fano * std::sqrt(k);
    Double_t gamma_k = gamma * fano * std::sqrt(k);
    sum += pk * TMath::Voigt(xx - mean_k, sigma_k, gamma_k);
  }

  return norm * sum;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
double ComputeVoigtFWHM(double sigma, double gamma) {
  return 0.5346 * 2 * gamma + sqrt(0.2166 * 4 * gamma * gamma + 2.0 * log(2) * sigma * sigma * 8.0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void CreateHist( Hist &hm ){
  for(int ch=1;ch<=12;ch++){
    hm.AddHist1D(Form("h_adcoff%02d",ch)   ,Form("ADC-Ped (ch%02d);ADC;Counts",ch)   , 800, -100,  700, 1);
    hm.AddHist1D(Form("h_adc%02d", ch)     ,Form("ADC (ch%02d);ADC;Counts", ch)      , 500,  600, 1600, 1);
    hm.AddHist1D(Form("h_adcwt%02d", ch)   ,Form("ADC w/TDC(ch%02d);ADC;Counts", ch) , 500,  600, 1600,13,3003);
    hm.AddHist1D(Form("h_tdc%02d", ch)     ,Form("TDC (ch%02d);TDC;Counts", ch)      , 500,    0, 1000, 1);
    hm.AddHist1D(Form("h_npe%02d", ch)     ,Form("NPE (ch%02d);NPE;Counts", ch)      , 200,    0,   20, 1);
//    hm.AddHist2D(Form("h_adc_evid%02d", ch),Form("ADC vs EvID (ch%02d);EvID;ADC", ch), 200,    0, 50000, 200, 600, 1600);
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void FitAdcoff(TH1D* h1, int ch) {
  if (!h1) return;

  double p1[4];
  TF1 *f1 = new TF1(Form("f1_%s", h1->GetName()), VoigtFunc, -50, 50, 4);
  f1->SetNpx(500);
  f1->SetLineStyle(2); f1->SetLineColor(2); f1->SetLineWidth(1);
  f1->SetParNames("norm", "mean", "sigma", "gamma");
  f1->SetParameters(100, 0.0, 10.0, 1.0);
  h1->Fit(f1, "0Q", "", -50, 10);
  f1->GetParameters(p1);
  h1->Fit(f1, "0Q", "", -50, p1[1] + p1[2] + p1[3]);
  f1->GetParameters(p1);

  TF1 *f2 = new TF1(Form("f2_%s", h1->GetName()), PoisLorentzFunc, -100, 500, 8);
  f2->SetNpx(500);
  f2->SetLineStyle(2); f2->SetLineColor(4); f2->SetLineWidth(1);
  f2->SetParNames("offset", "mean", "sigma", "gamma", "fano", "xscale", "nonlin", "norm");
  f2->SetParameters(p1[1], 4.0, p1[2], p1[3], 0.8, 35.0, 1.0, 10000);
  f2->FixParameter(0, p1[1]);
  f2->SetParLimits(1, 2.0, 10.0);
  f2->SetParLimits(2, p1[2]*0.8, p1[2]*1.1);
  f2->SetParLimits(3, p1[3]*0.8, p1[3]*1.1);
  f2->SetParLimits(4, 0.5, 1.0);
  f2->SetParLimits(6, 0.99, 1.1);
//  h1->Fit(f2, "0Q", "", -100, 500);
// ROOT bug? 
  TH1D *hnew = (TH1D*)h1->Clone("hnew");
  hnew->Fit(f2, "0Q", "", -100, 500);
  h1->GetListOfFunctions()->Add(f2);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void SaveHistToPDF(const Hist& hm, const string& pdffile = "hist.pdf") {
  gStyle->SetOptStat(1110);
  gStyle->SetStatW(0.15);
  gStyle->SetStatFontSize(0.03);
  gStyle->SetStatTextColor(1);
  gStyle->SetStatFont(42);
  int xsize = 1000;
  int ysize = 1000;

  TCanvas *c1 = new TCanvas("c1","c1", xsize, ysize);
  c1->Divide(3, 4, 1E-5, 1E-5);
  c1->Print((pdffile + "[").c_str());
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto h = hm.Get<TH1D>(Form("h_adc%02d", ch));
    h->Draw();
    h = hm.Get<TH1D>(Form("h_adcwt%02d", ch));
    h->Draw("same");
  }
  c1->Update();
  c1->Print(pdffile.c_str());
  delete c1;

  TCanvas *c2 = new TCanvas("c2","c2", xsize, ysize);
  c2->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c2->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto h = hm.Get<TH1D>(Form("h_tdc%02d", ch));
    h->Draw();
  }
  c2->Update();
  c2->Print(pdffile.c_str());
  delete c2;

  TCanvas *c3 = new TCanvas("c3","c3", xsize, ysize);
  c3->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c3->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto h = hm.Get<TH1D>(Form("h_adcoff%02d", ch));
    h->Draw();
    TF1 *f1 = h->GetFunction(Form("f1_%s",h->GetName()));
    TF1 *f2 = h->GetFunction(Form("f2_%s",h->GetName()));
    h->GetListOfFunctions()->ls();
    if (f1) f1->Draw("same");
    else cout<<"no1"<<endl;
    if (f2) f2->Draw("same");
    else cout<<"no2"<<endl;

    double fwhm1 = f1 ? ComputeVoigtFWHM(f1->GetParameter(2), f1->GetParameter(3)) : 0;
    double fwhm2 = f2 ? ComputeVoigtFWHM(f2->GetParameter(2), f2->GetParameter(3)) : 0;
    double xscale = f2 ? f2->GetParameter(5) : 0;
    
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.03);
    latex.DrawLatex(0.65, 0.85, Form("FWHM_{Voigt} = %.2f", fwhm1));
    latex.DrawLatex(0.65, 0.80, Form("FWHM_{PL} = %.2f", fwhm2));
    latex.DrawLatex(0.65, 0.75, Form("xscale = %.2f", xscale));  }
  c3->Update();
  c3->Print(pdffile.c_str());
  delete c3;

  TCanvas* c_dummy = new TCanvas("c_dummy", "dummy");
  c_dummy->Print((pdffile + "]").c_str());
  delete c_dummy;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void SaveHistToFile(const Hist& hm, const std::string& rootfile) {
  TFile* fout = new TFile(rootfile.c_str(), "RECREATE");
  for (const auto& name : hm.GetNames()) {
    auto hist = hm.Get<TH1D>(name);
    if (hist) hist->Write();
  }
  fout->Close();
  delete fout;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void SaveCSV(const Hist& hm, const string& filename) {
  ofstream fout(filename);
  fout << "ch,f1_norm,f1_mean,f1_sigma,f1_gamma,f1_chi2NDF"
       << ",f2_offset,f2_mean,f2_sigma,f2_gamma,f2_fano,f2_scale,f2_nonlin,f2_norm,f2_chi2NDF"
       << ",FWHM_Voigt,FWHM_PoisLorentz,xscale" << endl;

  for (int ch = 1; ch <= 12; ++ch) {
    auto h = hm.Get<TH1D>(Form("h_adcoff%02d", ch));
    if (!h) continue;
    TF1* f1 = h->GetFunction(Form("f1_h_adcoff%02d", ch));
    TF1* f2 = h->GetFunction(Form("f2_h_adcoff%02d", ch));
    if (!f1 || !f2) continue;

    double fwhm1 = ComputeVoigtFWHM(f1->GetParameter(2), f1->GetParameter(3));
    double fwhm2 = ComputeVoigtFWHM(f2->GetParameter(2), f2->GetParameter(3));
    double xscale = f2->GetParameter(5);
    double chi2f1 = f1->GetChisquare() / f1->GetNDF();
    double chi2f2 = f2->GetChisquare() / f2->GetNDF();

    fout << ch;
    for (int i = 0; i < 4; ++i) fout << "," << f1->GetParameter(i);
    fout << "," << chi2f1;
    for (int i = 0; i < 8; ++i) fout << "," << f2->GetParameter(i);
    fout << "," << chi2f2;
    fout << "," << fwhm1 << "," << fwhm2 << "," << xscale << endl;
  }
  fout.close();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, char** argv) {
  string input_filename  = "hoge.root";
  int ch;
  extern char *optarg;
  while((ch=getopt(argc,argv,"hf:"))!=-1){
    switch(ch){
    case 'f': input_filename = optarg; break;
    case 'h':
      cout<<"Usage ./UserAna -f input.root"<<endl;
      cout<<"-f : input filename"<<endl;
      return 0;
      break;
    default:
      cout<<"type -h to see help!!"<<endl;
      return 0;
    }
  }

  TApplication app("app", &argc, argv);
  gROOT->SetBatch(1);
  time_t start, end;
  start = time(NULL);
  time(&start);

  TreeReader t(input_filename.c_str());
  if (!t.IsValid()) return 1;

  Hist hm;
  CreateHist(hm);

  int N = t.GetEntries();
  for (int n = 0; n < N; n++) {
    if (!t.GetEntry(n)) continue;

    if (t.evid > 2000) {
      for( int ch = 1;ch<=12;ch++){
        auto h1 = hm.Get<TH1D>(Form("h_adcoff%02d",ch));
        h1->Fill(t.adcoff[ch]);
        h1 = hm.Get<TH1D>(Form("h_adc%02d",ch));
        h1->Fill(t.adcH[ch]);
        if(t.tdcL[ch] > 0){
          h1 = hm.Get<TH1D>(Form("h_adcwt%02d",ch));
          h1->Fill(t.adcH[ch]);
        }
        h1 = hm.Get<TH1D>(Form("h_tdc%02d",ch));
        h1->Fill(t.tdcL[ch]);
        h1 = hm.Get<TH1D>(Form("h_npe%02d",ch));
        h1->Fill(t.npe[ch]);
      }
    }

    if(n%10000==0){
     char clout[100];
     end = time(NULL);
     time(&end);
     sprintf(clout,"%.0f sec",difftime(end,start));
     cout<<n<<" / "<<N<<" : "<<endl;
    }
  } // for i

  for (int ch = 1; ch <= 12; ++ch) {
    TH1D *h1 = hm.Get<TH1D>(Form("h_adcoff%02d", ch));
    FitAdcoff(h1, ch);
  }

//  TCanvas *c1 = new TCanvas("c1", "c1", 1600, 1000);
//  c1->Divide(1,1,1E-5,1E-5);
//  c1->cd(1)->SetMargin(0.15,0.10,0.15,0.10);
//  h1->Draw();
//  f1->Draw("same");
//  f2->Draw("same");

  SaveHistToPDF(hm, "hist.pdf");
//  SaveHistToFile(hm, "hist.root");
//  SaveCSV(hm, "hist.csv");

//  app.Run();
  return 0;
}

