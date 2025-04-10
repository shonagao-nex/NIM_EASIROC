#ifndef HIST_H
#define HIST_H

#include <string>
#include <map>
#include <vector>
#include <TObject.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TROOT.h>
#include <iostream>

class Hist {
public:
  Hist();

  void AddHist1D(const std::string& name, const std::string& title,
                 int nbins, double xmin, double xmax, int color=1, int fill=0);

  void AddHist2D(const std::string& name, const std::string& title,
                 int nx, double xmin, double xmax,
                 int ny, double ymin, double ymax);

  void AddGraph(const std::string& name, const std::string& title);

  template <typename T>
  T* Get(const std::string& name) const {
    TObject* obj = gROOT->FindObject(name.c_str());
    if (!obj) {
      std::cerr << "Warning: object '" << name << "' not found!" << std::endl;
      return nullptr;
    }
    return dynamic_cast<T*>(obj);
  }

  std::vector<std::string> GetNames() const;

private:
  std::map<std::string, TObject*> objects_;
};

#endif

