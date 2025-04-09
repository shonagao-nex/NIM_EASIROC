#ifndef DRAW_H
#define DRAW_H

#include <TMath.h>
#include <TH1D.h>
#include "Tree.h"
#include "Hist.h"


double VoigtFunc(Double_t *x, Double_t *par);
double PoisLorentzFunc(Double_t *x, Double_t *par);

//class VoigtFunc {
//public:
//  static Double_t Eval(Double_t *x, Double_t *par) {
//    return par[0] * TMath::Voigt(x[0] - par[1], par[2], par[3]);
//  }
//};
//
//class PoisLorentzFunc {
//public:
//  static Double_t Eval(Double_t *x, Double_t *par) {
//    double offset = par[0];
//    double mu     = par[1];
//    double sigma  = par[2];
//    double gamma  = par[3];
//    double fano   = par[4];
//    double scale  = par[5];
//    double linear = par[6];
//    double norm   = par[7];
//
//    Double_t xx = x[0];
//    Double_t sum = 0.0;
//
//    sum += TMath::Poisson(0, mu) * TMath::Voigt(xx - offset, sigma, gamma);
//    sum += TMath::Poisson(1, mu) * TMath::Voigt(xx - offset - scale, sigma, gamma);
//
//    for (Int_t k = 2; k <= 20; ++k) {
//      Double_t pk = TMath::Poisson(k, mu);
//      if (pk < 1e-6) continue;
//
//      Double_t mean_k = std::pow(k, linear) * (scale - offset);
//      Double_t sigma_k = sigma * fano * std::sqrt(k);
//      Double_t gamma_k = gamma * fano * std::sqrt(k);
//      sum += pk * TMath::Voigt(xx - mean_k, sigma_k, gamma_k);
//    }
//
//    return norm * sum;
//  }
//};

void CreateHist( Hist &hm );
//void FillHist(TreeReader& reader, TH1D* hist, int ch = 1);

#endif

