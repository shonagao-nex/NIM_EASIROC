#include "Tree.h"

TreeReader::TreeReader(const char* filename) {
  file_ = TFile::Open(filename);
  if (!file_ || file_->IsZombie()) {
    file_ = nullptr;
    return;
  }

  tree_ = (TTree*)file_->Get("tree");
  if (!tree_) {
    file_->Close();
    file_ = nullptr;
    return;
  }

  tree_->SetBranchAddress("evid"    , &evid    );
  tree_->SetBranchAddress("adcH"    , adcH     );
  tree_->SetBranchAddress("adcL"    , adcL     );
  tree_->SetBranchAddress("tdcL"    , tdcL     );
  tree_->SetBranchAddress("tdcT"    , tdcT     );
  tree_->SetBranchAddress("adcoff"  , adcoff   );
  tree_->SetBranchAddress("npe"     , npe      );
  tree_->SetBranchAddress("time"    , time     );
  tree_->SetBranchAddress("width"   , width    );
  tree_->SetBranchAddress("scaler"  , scaler   );
  tree_->SetBranchAddress("duration", &duration);
}

TreeReader::~TreeReader() {
  if (file_) file_->Close();
}

bool TreeReader::IsValid() const {
  return file_ != nullptr;
}

bool TreeReader::GetEntry(int i) {
  if (!tree_) return false;
  return tree_->GetEntry(i) > 0;
}

int TreeReader::GetEntries() const {
  return tree_ ? tree_->GetEntries() : 0;
}
