#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TMath.h>
#include <iostream>
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
void CreateHist( Hist &hm ){
  hm.AddHist1D("h_adcoff", "ADC - Pedestal;ADC (ch);Counts", 800, -100, 700);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, char** argv) {
  string input_filename  = "hoge.root";
  int ch;
  extern char *optarg;
  while((ch=getopt(argc,argv,"hf:"))!=-1){
    switch(ch){
    case 'f':
      input_filename = optarg;
      cout<<"input filename : "<<input_filename<<endl;
      break;
    case 'h':
      cout<<"example) ./draw.cc -f rootfile"<<endl;
      cout<<"-f : input filename"<<endl;
      return 0;
      break;
    case '?':
      cout<<"unknown option...."<<endl;
      return 0;
      break;
    default:
      cout<<"type -h to see help!!"<<endl;
      return 0;
    }
  }

  TApplication app("app", &argc, argv);

  TreeReader tree(input_filename.c_str());
  if (!tree.IsValid()) return 1;

  Hist hm;
  CreateHist(hm);  // ここでまとめて定義
  TH1D *h1;

  int N = tree.GetEntries();
  for (int i = 0; i < N; ++i) {
    if (!tree.GetEntry(i)) continue;
    if (tree.evid > 1000) {
      h1 = hm.Get<TH1D>("h_adcoff");  h1->Fill(tree.adcoff[1]);
    }
  }

  h1 = hm.Get<TH1D>("h_adcoff");

  double p1[4];
  TF1 *f1 = new TF1("f1",VoigtFunc,-50,50,4);
  f1->SetNpx(500);
  f1->SetLineStyle(7);
  f1->SetLineColor(2);
  f1->SetLineWidth(6);
  f1->SetParNames("norm", "mean", "sigma", "gamma");
  f1->SetParameters(100, 0.0, 10.0, 1.0);
  h1->Fit(f1,"0Q","",-50,10);
  f1->GetParameters(p1);
  h1->Fit(f1,"0Q","",-50,p1[1] + p1[2] + p1[3]);
  f1->GetParameters(p1);

  double p2[8];
  TF1 *f2 = new TF1("f2",PoisLorentzFunc, -100, 500, 8);
  f2->SetNpx(500);
  f2->SetLineStyle(7);
  f2->SetLineColor(4);
  f2->SetLineWidth(6);
  f2->SetParNames("offset", "mean", "sigma", "gamma", "fano", "xscale", "nonlin", "norm");
  f2->SetParameters(p1[1] ,    4.0,   p1[2],   p1[3],    0.8,     35.0,     1.00,  10000);
  f2->FixParameter(   0, p1[1]);
  f2->SetParLimits(   1, 2.0, 10.0);
  f2->SetParLimits(   2, p1[2]*0.8, p1[2]*1.1);
  f2->SetParLimits(   3, p1[3]*0.8, p1[3]*1.1);
  f2->SetParLimits(   4,       0.5,       1.0);
//  f2->SetParLimits(   5,       1.0,     100.0);
  f2->SetParLimits(   6,      0.99,      1.1);

  h1->Fit(f2, "0", "", -100, 500);
  f2->GetParameters(p2);

  TCanvas *c1 = new TCanvas("c1", "c1", 1600, 1000);
  c1->Divide(1,1,1E-5,1E-5);
  c1->cd(1)->SetMargin(0.15,0.10,0.15,0.10);
  h1->Draw();
  f1->Draw("same");
  f2->Draw("same");

  app.Run();
  return 0;
}

//double Voigt(Double_t *x, Double_t *par) {
//  return par[0] * TMath::Voigt(x[0] - par[1], par[2], par[3]);
//}
//
//double PoisLorentz(Double_t *x, Double_t *par) {
//  double offset = par[0];   // pedestal Offset
//  double mu     = par[1];   // Poisson Mean
//  double sigma  = par[2];   // Gaussian Width (resolution)
//  double gamma  = par[3];   // Lorentzian Width (tail)
//  double fano   = par[4];   // Fano Factor
//  double scale  = par[5];   // ADC scaling factor
//  double linear = par[6];   // ADC non-linearity
//  double norm   = par[7];   // normalization factor
//
//  Double_t xx = x[0];
//  Double_t sum = 0.0;
//
//  // k = 0: pedestal
////  sum += TMath::Poisson(0, mu) * TMath::Gaus(xx, 0.0, sigma, kTRUE);
//  sum += TMath::Poisson(0, mu) * TMath::Voigt(xx - offset, sigma, gamma);
//
//  // k = 1: OPE
////  sum += TMath::Poisson(1, mu) * TMath::Gaus(xx, scale, sigma, kTRUE);
//  sum += TMath::Poisson(1, mu) * TMath::Voigt(xx - offset - scale, sigma, gamma);
//
//  // k >= 2: NPE
//  Int_t k_max = 20;
//  for (Int_t k = 2; k <= k_max; k++) {
//    Double_t pk = TMath::Poisson(k, mu);
//    if (pk < 1e-6) continue;
//
//    Double_t mean_k = pow(k,linear) * (scale - offset);
//    Double_t sigma_k = sigma * fano * pow(k,0.5);
//    Double_t gamma_k = gamma * fano * pow(k,0.5);
////    sum += pk * TMath::Gaus(xx, mean_k, sigma_k, kTRUE);
//    sum += pk * TMath::Voigt(xx - mean_k, sigma_k, gamma_k);
//  }
//
//  return norm * sum;
//}

//void draw(string input_filename) {
//  TFile *file = TFile::Open(input_filename.c_str());
//  if (!file || file->IsZombie()) {
//    std::cerr << "Error opening file!" << std::endl;
//    return;
//  }
//
//  TTree *tree = (TTree*)file->Get("tree");
//  if (!tree) {
//    std::cerr << "TTree 'tree' not found!" << std::endl;
//    return;
//  }
//
//  Int_t evid;
//  Int_t adcH[16];
//  Double_t adcoff[16];
//  Double_t npe[16];
//
//  tree->SetBranchAddress("evid", &evid);
//  tree->SetBranchAddress("adcH", adcH);
//  tree->SetBranchAddress("adcoff", adcoff);
//  tree->SetBranchAddress("npe", npe);
//
//  // ヒストグラム作成
//  TH1D *h_adcoff = new TH1D("h_adcoff", "ADC - Pedestal;ADC (ch);Counts", 800, -100, 700);
//
//  Long64_t nentries = tree->GetEntries();
//  for (Long64_t i = 0; i < nentries; ++i) {
//    tree->GetEntry(i);
//    if (evid > 1000) {
//      h_adcoff->Fill(adcoff[1]);
//    }
//  }
//
//
//  double p1[4];
//  TF1 *f1 = new TF1("f1",Voigt,-50,50,4);
//  f1->SetNpx(500);
//  f1->SetLineStyle(7);
//  f1->SetLineColor(2);
//  f1->SetLineWidth(6);
//  f1->SetParNames("norm", "mean", "sigma", "gamma");
//  f1->SetParameters(100, 0.0, 10.0, 1.0);
//  h_adcoff->Fit(f1,"0Q","",-50,10);
//  f1->GetParameters(p1);
//  h_adcoff->Fit(f1,"0Q","",-50,p1[1] + p1[2] + p1[3]);
//  f1->GetParameters(p1);
//
//  double p2[8];
//  TF1 *f2 = new TF1("f2", PoisLorentz, -100, 500, 8);
//  f2->SetNpx(500);
//  f2->SetLineStyle(7);
//  f2->SetLineColor(4);
//  f2->SetLineWidth(6);
//  f2->SetParNames("offset", "mean", "sigma", "gamma", "fano", "xscale", "nonlin", "norm");
//  f2->SetParameters(p1[1] ,    4.0,   p1[2],   p1[3],    0.8,     35.0,     1.00,  10000);
//  f2->FixParameter(   0, p1[1]);
//  f2->SetParLimits(   1, 2.0, 10.0);
//  f2->SetParLimits(   2, p1[2]*0.8, p1[2]*1.1);
//  f2->SetParLimits(   3, p1[3]*0.8, p1[3]*1.1);
//  f2->SetParLimits(   4,       0.5,       1.0);
////  f2->SetParLimits(   5,       1.0,     100.0);
//  f2->SetParLimits(   6,      0.99,      1.1);
//
//  h_adcoff->Fit(f2, "0", "", -100, 500);
//  f2->GetParameters(p2);
//
//  TCanvas *c1 = new TCanvas("c1", "c1", 1600, 1000);
//  c1->Divide(1,1,1E-5,1E-5);
//  c1->cd(1)->SetMargin(0.15,0.10,0.15,0.10);
//  h_adcoff->Draw();
//  f1->Draw("same");
//  f2->Draw("same");
//}
//
//int main(int argc, char** argv) {
//
//  string input_filename  = "hoge.root";
//  int ch;
//  extern char *optarg;
//  while((ch=getopt(argc,argv,"hf:"))!=-1){
//    switch(ch){
//    case 'f':
//      input_filename = optarg;
//      cout<<"input filename : "<<input_filename<<endl;
//      break;
//    case 'h':
//      cout<<"example) ./draw.cc -f rootfile"<<endl;
//      cout<<"-f : input filename"<<endl;
//      return 0;
//      break;
//    case '?':
//      cout<<"unknown option...."<<endl;
//      return 0;
//      break;
//    default:
//      cout<<"type -h to see help!!"<<endl;
//      return 0;
//    }
//  }
//
//  TApplication app("app", &argc, argv);
//  draw(input_filename);
//  app.Run();
//  return 0;
//}

