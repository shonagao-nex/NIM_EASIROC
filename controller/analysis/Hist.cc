#include "Hist.h"
#include <TROOT.h>
#include <iostream>

Hist::Hist() {}

void Hist::AddHist1D(const std::string& name, const std::string& title,
                            int nbins, double xmin, double xmax, int color, int fill) {
  if (gROOT->FindObject(name.c_str())) return;
  TH1D* hist = new TH1D(name.c_str(), title.c_str(), nbins, xmin, xmax);
  objects_[name] = hist;

  hist->SetLineColor(color);
  hist->SetFillStyle(fill);
  hist->SetFillColor(color);

  hist->SetTitleSize(0.04,"");
  hist->SetTitleFont(42,"");

  hist->GetXaxis()->CenterTitle();
  hist->GetXaxis()->SetLabelFont(42);
  hist->GetXaxis()->SetTitleOffset(0.90);
  hist->GetXaxis()->SetTitleSize(0.06);
  hist->GetXaxis()->SetLabelOffset(0.01);

  hist->GetYaxis()->CenterTitle();
  hist->GetYaxis()->SetTitleFont(42);
  hist->GetYaxis()->SetTitleOffset(1.10);
  hist->GetYaxis()->SetTitleSize(0.06);
  hist->GetYaxis()->SetLabelFont(42);
  hist->GetYaxis()->SetLabelOffset(0.01);

}

void Hist::AddHist2D(const std::string& name, const std::string& title,
                            int nx, double xmin, double xmax,
                            int ny, double ymin, double ymax) {
  if (gROOT->FindObject(name.c_str())) return;
  TH2D* hist2 = new TH2D(name.c_str(), title.c_str(), nx, xmin, xmax, ny, ymin, ymax);
  objects_[name] = hist2;
}

void Hist::AddGraph(const std::string& name, const std::string& title) {
  if (gROOT->FindObject(name.c_str())) return;
  TGraph* graph = new TGraph();
  graph->SetName(name.c_str());
  graph->SetTitle(title.c_str());
  objects_[name] = graph;
}

template TH1D* Hist::Get<TH1D>(const std::string&) const;
template TH2D* Hist::Get<TH2D>(const std::string&) const;
template TGraph* Hist::Get<TGraph>(const std::string&) const;
template TGraph2D* Hist::Get<TGraph2D>(const std::string&) const;

std::vector<std::string> Hist::GetNames() const {
  std::vector<std::string> names;
  for (const auto& [key, _] : objects_) names.push_back(key);
  return names;
}

