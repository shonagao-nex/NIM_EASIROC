#include "Hist.h"
#include <TROOT.h>
#include <iostream>

Hist::Hist() {}

void Hist::AddHist1D(const std::string& name, const std::string& title,
                            int nbins, double xmin, double xmax) {
  if (gROOT->FindObject(name.c_str())) return;
  TH1D* hist = new TH1D(name.c_str(), title.c_str(), nbins, xmin, xmax);
  objects_[name] = hist;
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

template <typename T>
T* Hist::Get(const std::string& name) {
  TObject* obj = gROOT->FindObject(name.c_str());
  if (!obj) {
    std::cerr << "Warning: object '" << name << "' not found!" << std::endl;
    return nullptr;
  }
  return dynamic_cast<T*>(obj);
}

// 明示的インスタンス化（.ccに必要）
template TH1D* Hist::Get<TH1D>(const std::string&);
template TH2D* Hist::Get<TH2D>(const std::string&);
template TGraph* Hist::Get<TGraph>(const std::string&);
template TGraph2D* Hist::Get<TGraph2D>(const std::string&);

std::vector<std::string> Hist::GetNames() const {
  std::vector<std::string> names;
  for (const auto& [key, _] : objects_) names.push_back(key);
  return names;
}

