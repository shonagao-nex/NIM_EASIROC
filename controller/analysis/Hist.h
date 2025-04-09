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

class Hist {
public:
  Hist();

  // ヒストグラム追加
  void AddHist1D(const std::string& name, const std::string& title,
                 int nbins, double xmin, double xmax);

  void AddHist2D(const std::string& name, const std::string& title,
                 int nx, double xmin, double xmax,
                 int ny, double ymin, double ymax);

  void AddGraph(const std::string& name, const std::string& title);

  // テンプレート型付き取得
  template <typename T>
  T* Get(const std::string& name);

  // ヒスト名一覧
  std::vector<std::string> GetNames() const;

private:
  std::map<std::string, TObject*> objects_;
};

#endif

