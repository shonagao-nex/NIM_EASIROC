#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TAxis.h>
#include <TDatime.h>
#include <TStyle.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TLatex.h>
#include <TError.h>

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <map>
#include <algorithm>

struct SummaryData {
    std::vector<double> time;
    std::vector<double> hv, curr, t1;
    std::vector<std::vector<double>> idcs;
};

time_t parseTime(const std::string& timestamp) {
    struct std::tm tm {};
    strptime(timestamp.substr(0, 19).c_str(), "%Y-%m-%d %H:%M:%S", &tm);
    tm.tm_isdst = 1; // summer time !!
    return mktime(&tm);
}

// フィルタリング
SummaryData filterData(const SummaryData& src, time_t cutoff) {
    SummaryData out;
    out.idcs.resize(16);
    for (size_t i = 0; i < src.time.size(); ++i) {
        if (src.time[i] >= cutoff) {
            out.time.push_back(src.time[i]);
            out.hv.push_back(src.hv[i]);
            out.curr.push_back(src.curr[i]);
            out.t1.push_back(src.t1[i]);
            for (int j = 0; j < 16; ++j)
                out.idcs[j].push_back(src.idcs[j][i]);
        }
    }
    return out;
}

SummaryData loadSummary(const std::string& inputFile, int maxLines = 2000) {
    SummaryData data;
    data.idcs.resize(16);

    std::ifstream fin(inputFile);
    if (!fin) {
        std::cerr << "Cannot open file: " << inputFile << std::endl;
        return data;
    }

    std::string line;
    std::getline(fin, line);  // skip header

    std::vector<std::string> lines;
    while (std::getline(fin, line)) lines.push_back(line);
    if( (int)lines.size() < maxLines ) maxLines = (int)lines.size() - 1;
    int startIdx = std::max(0, (int)lines.size() - maxLines);

    for (int i = startIdx; i < (int)lines.size(); ++i) {
        std::stringstream ss(lines[i]);
        std::string token;

        std::getline(ss, token, ',');
        time_t t = parseTime(token);
        data.time.push_back(t);

        std::getline(ss, token, ','); data.hv.push_back(std::stod(token));
        std::getline(ss, token, ','); data.curr.push_back(std::stod(token));
        std::getline(ss, token, ','); data.t1.push_back(std::stod(token));
        std::getline(ss, token, ','); // skip T2

        for (int j = 0; j < 16; ++j) {
            if (!std::getline(ss, token, ',')) data.idcs[j].push_back(0);
            else data.idcs[j].push_back(std::stod(token));
        }
    }
    return data;
}

void drawHVCurrentT1(const SummaryData& data, const std::string& title, const std::string& pdfPath) {
    TCanvas* c = new TCanvas("c", title.c_str(), 1200, 1000);
    c->Divide(1,3,1E-5, 1E-5);
    std::vector<std::string> labels = {"HV(V)", "current(uA)", "T1(degC)"};
    std::vector<std::vector<double>> datas = {data.hv, data.curr, data.t1};

    std::map<std::string, std::pair<double, double>> y_ranges = {
        {"HV(V)", {0, 62}},
        {"current(uA)", {0, 50}},
        {"T1(degC)", {20, 35}},
    };

    for (int i = 0; i < 3; ++i) {
        c->cd(i+1)->SetMargin(0.15,0.05,0.15,0.10);
        TGraph* g = new TGraph(data.time.size(), &data.time[0], datas[i].data());
        g->SetLineColor(14);
        g->SetLineWidth(1);
        g->SetTitle((labels[i] + title).c_str());

        g->GetXaxis()->SetTimeDisplay(1);
        g->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
        g->GetXaxis()->SetLabelSize(0.05);
        g->GetXaxis()->SetTitleSize(0.07);
        g->GetXaxis()->CenterTitle();
        g->GetXaxis()->SetTitle("Time");

        g->GetYaxis()->CenterTitle();
        g->GetYaxis()->SetLabelSize(0.05);
        g->GetYaxis()->SetTitleSize(0.07);
        g->GetYaxis()->SetRangeUser(y_ranges[labels[i]].first, y_ranges[labels[i]].second);
        g->GetYaxis()->SetTitle(labels[i].c_str());

        g->Draw("AL");

        TLatex latex;
        latex.SetTextSize(0.10);
        latex.SetTextAlign(12);
        latex.DrawLatexNDC(0.2, 0.7, Form("%s = %.3f", labels[i].c_str(), datas[i].back()));
        latex.Draw();
    }

    c->Print(pdfPath.c_str());
    delete c;
}

void drawIDCs(const SummaryData& data, const std::string& title, const std::string& pdfPath) {
    const int nPad = 12;
    const int nCanvas = (16 + nPad - 1) / nPad;

    for (int page = 0; page < nCanvas; ++page) {
        TCanvas* c = new TCanvas(Form("c_idc_%d", page), title.c_str(), 1200, 1000);
        c->Divide(3, 4);  // 3列×4行 = 12グラフ

        for (int i = 1; i < 12; ++i) {
            int ch = i;
            if (ch >= 16) break;

            c->cd(i + 1);
            TGraph* g = new TGraph(data.time.size(), &data.time[0], data.idcs[ch].data());
            g->SetTitle(Form("IDC%d", ch));
            g->SetLineColor(16);
            g->SetLineWidth(1);
            g->SetMarkerStyle(20);
            g->SetMarkerSize(1);
            g->GetXaxis()->SetTimeDisplay(1);
            g->GetXaxis()->SetTimeFormat("%m-%d %H:%M");
            g->GetXaxis()->SetLabelSize(0.04);
            g->GetXaxis()->SetTitleSize(0.05);
            g->GetXaxis()->SetTitle("Time");
            g->GetXaxis()->SetTitleOffset(1.2);
            g->GetXaxis()->LabelsOption("45");
            g->GetYaxis()->SetLabelSize(0.04);
            g->GetYaxis()->SetTitle("Current (uA)");
            g->GetYaxis()->SetTitleSize(0.05);
            g->GetYaxis()->SetTitleOffset(1.2);
            g->GetYaxis()->SetRangeUser(-0.1, 3.0);
            g->GetXaxis()->CenterTitle();
            g->GetYaxis()->CenterTitle();
            g->Draw("AL");

            double latest_val = data.idcs[ch].back();
            TLatex latex;
            latex.SetTextSize(0.10);
            latex.SetTextAlign(12);
            latex.DrawLatexNDC(0.5, 0.6, Form("Latest: %.3f", latest_val));
            latex.Draw();
        }

        c->Print(pdfPath.c_str());
        delete c;
    }
}

int main(int argc, char** argv) {
    TApplication app("app", &argc, argv);
    gROOT->SetBatch(1);
    gErrorIgnoreLevel = kWarning;
    gStyle->SetTimeOffset(0);
    gStyle->SetOptStat(0);

    const std::string inputFile = "data/summary.log";
    const std::string outputPDF = "pdfs/summary_plots.pdf";

    gSystem->Unlink(outputPDF.c_str());

    SummaryData allData = loadSummary(inputFile, 50000);
    if (allData.time.empty()) {
        std::cerr << "No data loaded.\n";
        return 1;
    }

    time_t latest_time = allData.time.back();

    SummaryData data30h = filterData(allData, latest_time - 30 * 3600);
    SummaryData data6h  = filterData(allData, latest_time - 6 * 3600);

    TCanvas* c1 = new TCanvas("c1", "c1", 1200, 1000);
    c1->Print((outputPDF + "[").c_str());
    drawHVCurrentT1(data6h, "Last 6h", outputPDF);
    drawHVCurrentT1(data30h, "Last 30h", outputPDF);
    drawHVCurrentT1(allData, "Full", outputPDF);
//    drawIDCs(allData, "IDC Channels - Full", outputPDF);
//    drawIDCs(data12h, "IDC Channels - Last 12h", outputPDF);
//    drawIDCs(data3h,  "IDC Channels - Last 3h", outputPDF);
    c1->Print((outputPDF + "]").c_str());

    std::cout << "PDF saved to: " << outputPDF << std::endl;
    return 0;
}


