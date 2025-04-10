#include <TCanvas.h>
#include <TGraph.h>
#include <TString.h>
#include <TSystem.h>
#include <TAxis.h>
#include <TTimeStamp.h>

#include <dirent.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

using namespace std;

void draw_log(const char* directorypath) {
    vector<double> time, HV, current, T1;
    array<vector<double>, 12> DAC;

    string time_str, HV_str, current_str, T1_str;
    array<string, 12> DAC_str;

    double time_tmp, HV_tmp, current_tmp, T1_tmp;
    array<double, 12> DAC_tmp;

    string directoryname = string(directorypath).substr(string(directorypath).find_last_of('/') + 1);
    DIR* directory = opendir(directorypath);
    if (!directory) {
        cerr << "Error: Could not open directory " << directoryname << endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(directory)) != nullptr) {
        string filename = entry->d_name;
        if (filename == "." || filename == "..") {
            continue;
        }
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".log") {
            cerr << "Skipped file " << filename << endl;
            continue;
        }
        if (filename == "run_20250404_134712.log") {
            cerr << "Skipped file " << filename << endl;
            continue;
        }

        string filepath = string(directorypath) + "/" + filename;
        ifstream file(filepath);
        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            continue;
        }
        cout << "Loading: " << filename << endl;

        string header;
        getline(file, header);

        int count;
        string line;
        string dummy;
        while (getline(file, line)) {
            istringstream stream(line);
            if (!getline(stream, time_str, ',')) continue;
            if (!getline(stream, HV_str, ',')) continue;
            if (!getline(stream, current_str, ',')) continue;
            if (!getline(stream, T1_str, ',')) continue;
            if (!getline(stream, dummy, ',')) continue;
            if (!getline(stream, dummy, ',')) continue;
            for (int i = 0; i < 12; i++) {
                if(!getline(stream, DAC_str[i], ',')) continue;
            }

            int year, month, day, hour, minute, second, end;
            if (sscanf(time_str.c_str(), "%d-%d-%d %d:%d:%d -%d", &year, &month, &day, &hour, &minute, &second, &end) != 7) {
                cerr << "Warning: Invalid time format in line " << count + 2 << " of " << filename << endl;
                continue;
            }
            TTimeStamp timestamp(year, month, day, hour, minute, second);
            try {
                HV_tmp = stod(HV_str);
                current_tmp = stod(current_str);
                T1_tmp = stod(T1_str);
                for (int i = 0; i < 12; i++) {
                    DAC_tmp[i] = stod(DAC_str[i]);
                }
            } catch (const invalid_argument& e) {
                cerr << "Warning: Invalid value format in line " << count + 2 << " of " << filename << endl;
                continue;
            }

            time.push_back(timestamp.GetSec());
            HV.push_back(HV_tmp);
            current.push_back(current_tmp);
            T1.push_back(T1_tmp);
            for (int i = 0; i < 12; i++) {
                DAC[i].push_back(DAC_tmp[i]);
            }

            count++;
        }
        file.close();
    }

    closedir(directory);

    if (time.empty()) {
        cerr << "No valid data found!" << endl;
        return;
    }

    TCanvas* canvas[2];
    canvas[0] = new TCanvas("", "", 1280, 720);
    canvas[1] = new TCanvas("", "", 1280, 720);
    canvas[0]->Divide(2, 2);
    canvas[1]->Divide(4, 3);

    TGraph* graph[15];
    graph[0] = new TGraph(time.size(), &time[0], &HV[0]);
    graph[1] = new TGraph(time.size(), &time[0], &current[0]);
    graph[2] = new TGraph(time.size(), &time[0], &T1[0]);
    graph[0]->SetTitle("Time vs. HV;Time;HV / V");
    graph[1]->SetTitle("Time vs. Current;Time;Current / #muA");
    graph[2]->SetTitle("Time vs. T1;Time;T1 / ^{#circ}C");
    for (int i = 0; i < 12; i++) {
        graph[i + 3] = new TGraph(time.size(), &time[0], &DAC[i][0]);
        graph[i + 3]->SetTitle(Form("Time vs. DAC%d;Time;DAC%d;", i + 1, i + 1));
    }

    for (int i = 0; i < 15; i++) {
        double xmin = graph[i]->GetXaxis()->GetXmin();
        double xmax = graph[i]->GetXaxis()->GetXmax();
        double ymin = graph[i]->GetYaxis()->GetXmin();

        graph[i]->GetXaxis()->SetTickLength(0);
        graph[i]->GetXaxis()->SetLabelSize(0);
        graph[i]->GetXaxis()->SetTitle("");
        graph[i]->SetMarkerStyle(20);
        graph[i]->SetMarkerSize(0.25);
        graph[i]->SetLineWidth(1);

        TBox *box = new TBox(xmin, - 1e4, xmax, ymin - 0.0025);
        box->SetFillColor(kWhite);
        box->SetLineColor(kWhite);
        box->SetLineWidth(0);

        TGaxis* gaxis[2];
        for (int j = 0; j < 2; j++){
            gaxis[j] = new TGaxis(xmin, ymin, xmax, ymin, xmin, xmax, 510, "+t");
            gaxis[j]->SetTimeOffset(0, "gmt");
            gaxis[j]->SetLabelSize(0.025);
            gaxis[j]->SetLabelOffset(0.025 + 0.0375 * j);
            gaxis[j]->SetTextAngle(40);
        }
        gaxis[0]->SetTimeFormat("%Y-%m-%d       ");
        gaxis[1]->SetTimeFormat("       %H:%M:%S");

        if (i < 3) {
            canvas[0]->cd(i + 1);
        } else {
            canvas[1]->cd(i - 2);
        }
        graph[i]->Draw("APL");
        box->Draw("same");
        gaxis[0]->Draw();
        gaxis[1]->Draw();
    }

    TString output[2];
    output[0] = "/data/JLab/SciFi/sipm_test/pictures/" + directoryname + "_env.pdf";
    output[1] = "/data/JLab/SciFi/sipm_test/pictures/" + directoryname + "_DAC.pdf";
    canvas[0]->SaveAs(output[0]);
    canvas[1]->SaveAs(output[1]);
}
