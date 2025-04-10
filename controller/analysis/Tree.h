#ifndef TREE_H
#define TREE_H

#include <TFile.h>
#include <TTree.h>
#include <iostream>

class TreeReader {
public:
  TreeReader(const char* filename);
  ~TreeReader();
  bool IsValid() const;
  bool GetEntry(int i);
  int GetEntries() const;

  int evid;
  int adcH[16], adcL[16];
  int tdcL[16], tdcT[16];
  double adcoff[16],npe[16];
  double time[16], width[16];
  int scaler[16];
  double duration;


private:
  TFile* file_;
  TTree* tree_;
};

#endif

