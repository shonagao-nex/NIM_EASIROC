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
#include <TDatime.h>
#include <iostream>
#include <fstream>
#include <cmath>

#include "Tree.h"
#include "Hist.h"
using namespace std;

//++++++++++++++++++++++++++++++++++++++++++++++++++
void CreateHist( Hist &hm ){
  for(int ch=1;ch<=12;ch++){
    hm.AddHist1D(Form("h_adc%02d", ch)     ,Form("ADC (ch%02d);ADC;Counts", ch)          , 500,  600, 1600,  1);
    hm.AddHist1D(Form("h_ped%02d", ch)     ,Form("ADC Pedestal (ch%02d);ADC;Counts", ch) , 500,  600, 1600,  1);
    hm.AddHist1D(Form("h_adcwt%02d", ch)   ,Form("ADC w/TDC(ch%02d);ADC;Counts", ch)     , 500,  600, 1600,  1);
    hm.AddHist1D(Form("h_tdcl%02d", ch)    ,Form("TDC Leading (ch%02d);TDC;Counts", ch)  , 500,    0, 1000,  1);
    hm.AddHist1D(Form("h_tdct%02d", ch)    ,Form("TDC Trailing (ch%02d);TDC;Counts", ch) , 500,    0, 1000,  1);
    hm.AddHist1D(Form("h_scale%02d", ch)   ,Form("Scaler (ch%02d);Rate;Counts", ch)      , 200,    0, 5E+5,  1);
    hm.AddHist1D(Form("hr_adc%02d", ch)    ,Form("ADC (ch%02d);ADC;Counts", ch)          , 500,  600, 1600,626, 3001);
    hm.AddHist1D(Form("hr_ped%02d", ch)    ,Form("ADC Pedestal (ch%02d);ADC;Counts", ch) , 500,  600, 1600,626, 3001);
    hm.AddHist1D(Form("hr_adcwt%02d", ch)  ,Form("ADC w/TDC(ch%02d);ADC;Counts", ch)     , 500,  600, 1600,626, 3001);
    hm.AddHist1D(Form("hr_tdcl%02d", ch)   ,Form("TDC Leading (ch%02d);TDC;Counts", ch)  , 500,    0, 1000,626, 3001);
    hm.AddHist1D(Form("hr_tdct%02d", ch)   ,Form("TDC Trailing (ch%02d);TDC;Counts", ch) , 500,    0, 1000,626, 3001);
    hm.AddHist1D(Form("hr_scale%02d", ch)  ,Form("Scaler (ch%02d);Rate;Counts", ch)      , 200,    0, 5E+5,626, 3001);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++
void SaveHistToPDF(const Hist& hm, const string& inputfile, const string& goldfile, const string& pdffile = "hist.pdf") {
  gErrorIgnoreLevel = kWarning;
  gStyle->SetOptStat(1110);
  gStyle->SetStatW(0.15);
  gStyle->SetStatFontSize(0.03);
  gStyle->SetStatTextColor(1);
  gStyle->SetStatFont(42);
  int xsize = 1000;
  int ysize = 1000;

  TCanvas *c1 = new TCanvas("c1","c1", xsize, ysize);
  c1->Print((pdffile + "[").c_str());

  TDatime now;
  TString dateStr = Form("%02d-%02d-%04d %02d:%02d",now.GetMonth(), now.GetDay(), now.GetYear(),now.GetHour(), now.GetMinute());

  c1->Clear();
  TLatex title;
  title.SetTextSize(0.05);
  title.SetTextAlign(22);
  title.DrawLatexNDC(0.5, 0.6, "SiPM Four Symbols Online Hist");
  title.SetTextSize(0.03);
  title.DrawLatexNDC(0.5, 0.5, dateStr);
  string input_str = "Data = " + inputfile;
  title.DrawLatexNDC(0.5, 0.4, input_str.c_str());
  string gold_str = "GOLD = " + goldfile;
  title.SetTextColor(626);
  title.DrawLatexNDC(0.5, 0.3, gold_str.c_str());
  c1->Print(pdffile.c_str());

  TLatex legend;
  legend.SetTextSize(0.05);
  legend.SetTextAlign(12);

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto hr = hm.Get<TH1D>(Form("hr_ped%02d", ch));
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_ped%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto hr = hm.Get<TH1D>(Form("hr_adc%02d", ch));
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_adc%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto hr = hm.Get<TH1D>(Form("hr_adcwt%02d", ch));
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_adcwt%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto hr = hm.Get<TH1D>(Form("hr_tdcl%02d", ch));
    gPad->SetLogy();
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_tdcl%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.10);
    auto hr = hm.Get<TH1D>(Form("hr_tdct%02d", ch));
    gPad->SetLogy();
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_tdct%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());

  c1->Clear();
  c1->Divide(3, 4, 1E-5, 1E-5);
  for(int ch=1;ch<=12;ch++){
    c1->cd(ch)->SetMargin(0.15,0.05,0.15,0.15);
    auto hr = hm.Get<TH1D>(Form("hr_scale%02d", ch));
    hr->Draw("");
    auto h = hm.Get<TH1D>(Form("h_scale%02d", ch));
    h->Draw("same");
    legend.SetTextColor(1);   legend.DrawLatexNDC(0.6, 0.8, "DATA");
    legend.SetTextColor(626); legend.DrawLatexNDC(0.6, 0.7, "REFERENCE");
  }
  c1->Update();
  c1->Print(pdffile.c_str());
  c1->Print((pdffile + "]").c_str());

}

//++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, char** argv) {
  string input_filename  = "hoge.root";
  string gold_filename  = "hoge.root";
  string pdf_filename  = "hist.pdf";
  int ch;
  extern char *optarg;
  while((ch=getopt(argc,argv,"hf:g:o:"))!=-1){
    switch(ch){
    case 'f': input_filename = optarg; break;
    case 'g': gold_filename  = optarg; break;
    case 'o': pdf_filename   = optarg; break;
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

//  TApplication app("app", &argc, argv);
  gROOT->SetBatch(1);

  TreeReader t(input_filename.c_str());
  if (!t.IsValid()) return 1;
  TreeReader tr(gold_filename.c_str());
  if (!tr.IsValid()) return 1;

  Hist hm;
  CreateHist(hm);

  int N = t.GetEntries();
  for (int n = 0; n < N; n++) {
    if (!t.GetEntry(n)) continue;
    if (t.evid < 2000) {  // Pedestal
      for( int ch = 1;ch<=12;ch++){
        auto h1 = hm.Get<TH1D>(Form("h_ped%02d",ch));
        h1->Fill(t.adcH[ch]);
      }
    } else if (t.evid >= 2000) {
      for( int ch = 1;ch<=12;ch++){
        auto h1 = hm.Get<TH1D>(Form("h_adc%02d",ch));
        h1->Fill(t.adcH[ch]);
        if(t.tdcL[ch] > 0){
          h1 = hm.Get<TH1D>(Form("h_adcwt%02d",ch));
          h1->Fill(t.adcH[ch]);
        }
        h1 = hm.Get<TH1D>(Form("h_tdcl%02d",ch));
        h1->Fill(t.tdcL[ch]);
        h1 = hm.Get<TH1D>(Form("h_tdct%02d",ch));
        h1->Fill(t.tdcT[ch]);
        h1 = hm.Get<TH1D>(Form("h_scale%02d",ch));
        h1->Fill(t.scaler[ch] / t.duration);
      }
    }
  } // for GetEntries

  int Nr = tr.GetEntries();
  for (int n = 0; n < Nr; n++) {
    if (!tr.GetEntry(n)) continue;
    if (tr.evid < 2000) {  // Pedestal
      for( int ch = 1;ch<=12;ch++){
        auto h1 = hm.Get<TH1D>(Form("hr_ped%02d",ch));
        h1->Fill(tr.adcH[ch]);
      }
    } else if (tr.evid >= 2000) {
      for( int ch = 1;ch<=12;ch++){
        auto h1 = hm.Get<TH1D>(Form("hr_adc%02d",ch));
        h1->Fill(tr.adcH[ch]);
        if(tr.tdcL[ch] > 0){
          h1 = hm.Get<TH1D>(Form("hr_adcwt%02d",ch));
          h1->Fill(tr.adcH[ch]);
        }
        h1 = hm.Get<TH1D>(Form("hr_tdcl%02d",ch));
        h1->Fill(tr.tdcL[ch]);
        h1 = hm.Get<TH1D>(Form("hr_tdct%02d",ch));
        h1->Fill(tr.tdcT[ch]);
        h1 = hm.Get<TH1D>(Form("hr_scale%02d",ch));
        h1->Fill(tr.scaler[ch] / tr.duration);
      }
    }
  } // for GetEntries

  SaveHistToPDF(hm, input_filename, gold_filename, pdf_filename);

//  app.Run();
  return 0;
}

