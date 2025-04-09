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
  int adcH[16];
  double adcoff[16];
  double npe[16];

private:
  TFile* file_;
  TTree* tree_;
};

#endif

