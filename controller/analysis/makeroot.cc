// libarchive-devel is necessary
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <bitset>

#include <TFile.h>
#include <TTree.h>
#include <TH1.h>

#include "json.hpp"

//#include <filesystem>
//#include <vector>
#include <cstdlib>
//#include <cstdio>

#include <archive.h>
#include <archive_entry.h>

using namespace std;
using json = nlohmann::json;

const int ScalerInt = 10;
const int Nch = 16;

bool ends_with(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool extractDatFilesFromTarXZ(const std::string& archivePath, const std::string& outputFile)
{
    struct archive* a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_xz(a);

    if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive: " << archive_error_string(a) << std::endl;
        return false;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* entryName = archive_entry_pathname(entry);
        std::string name(entryName);

        if (ends_with(name, ".dat")) {
            std::ofstream ofs(outputFile.c_str(), std::ios::binary);
            const void* buff;
            size_t size;
            int64_t offset;

            while (true) {
                int r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r != ARCHIVE_OK) {
                    std::cerr << "Read error: " << archive_error_string(a) << std::endl;
                    return false;
                }
                ofs.write(reinterpret_cast<const char*>(buff), size);
            }

            ofs.close();
            archive_read_close(a);
            archive_read_free(a);
            return true;
        } else {
            archive_read_data_skip(a);  // スキップ
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    std::cerr << "No .dat file found in archive." << std::endl;
    return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::string formatBin(int num) {
    std::bitset<32> bits(num);
    std::string binaryStr = bits.to_string();
    std::string formattedStr = "";

    const int SPACE_POS[] = {12, 13, 20};
    const int SPACE_COUNT = sizeof(SPACE_POS) / sizeof(SPACE_POS[0]);

    for (size_t i = 0; i < binaryStr.size(); i++) {
        formattedStr += binaryStr[i];

        for (int j = 0; j < SPACE_COUNT; j++) {
            if (i == static_cast<size_t>(31 - SPACE_POS[j])) {
                formattedStr += " ";
            }
        }
    }
    return formattedStr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
unsigned int getBigEndian32(const char* b)
{
    return ((b[0] << 24) & 0xff000000) |
           ((b[1] << 16) & 0x00ff0000) |
           ((b[2] <<  8) & 0x0000ff00) |
           ((b[3] <<  0) & 0x000000ff);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isCheck(unsigned int data)
{
    int val = (data >> 24) & 0xff;
    return val != 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isAdcHg(unsigned int data)
{
    return (data & 0x00680000) == 0x00000000;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isAdcLg(unsigned int data)
{
    return (data & 0x00680000) == 0x00080000;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isTdcLeading(unsigned int data)
{
    return (data & 0x00601000) == 0x00201000;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isTdcTrailing(unsigned int data)
{
    return (data & 0x00601000) == 0x00200000;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool isScaler(unsigned int data)
{
    return (data & 0x00600000) == 0x00400000;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool loadParamJSON(const std::string& filename, double gain[Nch], double pedestal[Nch], double timeOffset[Nch], double timeScale[Nch])
{
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return false;
    }

    json j;
    try {
        infile >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    struct Param {
        const char* name;
        double* array;
    } params[] = {
        {"Gain",       gain},
        {"Pedestal",   pedestal},
        {"TimeOffset", timeOffset},
        {"TimeScale",  timeScale}
    };

    for (const auto& p : params) {
        if (j.find(p.name) == j.end() || !j[p.name].is_array()) {
            std::cerr << "Missing or invalid '" << p.name << "' array in JSON file." << std::endl;
            return false;
        }

        const auto& arr = j[p.name];
        if (arr.size() != Nch) {
            std::cerr << "Expected " << Nch << " values for " << p.name
                      << ", got " << arr.size() << std::endl;
            return false;
        }

        for (int i = 0; i < Nch; ++i) {
            p.array[i] = arr[i].get<double>();
        }
    }

    return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void DecodeData(const string &input_filename, const string &output_filename, const string &param_filename)
//void DecodeData(const string& filename)
{
//    string::size_type pos = filename.find(".dat");
//    if(pos == string::npos) {
//        cerr << filename << " is not a dat file" << endl;
//        return;
//    }
//    string rootfile_name(filename);
//    rootfile_name.replace(pos, 5, ".root");

    TFile *f = new TFile(output_filename.c_str(), "RECREATE");
    TTree *tree = new TTree("tree","tree");
    int evid;
    int adcH[Nch], adcL[Nch];
    int tdcL[Nch], tdcT[Nch];
    double adcoff[Nch], npe[Nch];
    double time[Nch], width[Nch];
    int scaler[Nch];
    double duration;

    tree->Branch("evid"    , &evid    , "evid/I"         );
    tree->Branch("adcH"    ,  adcH    , Form("adcH[%d]/I",Nch)     );
    tree->Branch("adcL"    ,  adcL    , Form("adcL[%d]/I",Nch)     );
    tree->Branch("tdcL"    ,  tdcL    , Form("tdcL[%d]/I",Nch)     );
    tree->Branch("tdcT"    ,  tdcT    , Form("tdcT[%d]/I",Nch)     );
    tree->Branch("adcoff"  ,  adcoff  , Form("adcoff[%d]/D",Nch)   );
    tree->Branch("npe"     ,  npe     , Form("npe[%d]/D",Nch)      );
    tree->Branch("time"    ,  time    , Form("time[%d]/D",Nch)     );
    tree->Branch("width"   ,  width   , Form("width[%d]/D",Nch)    );
    tree->Branch("scaler"  ,  scaler  , Form("scaler[%d]/I",Nch)   );
    tree->Branch("duration", &duration, "duration/D"     );

    ifstream datFile(input_filename.c_str(), ios::in | ios::binary);
    unsigned int scalerValuesArray[ScalerInt][69];
    unsigned int events = 0;
    while(datFile) {
        for(int i=0;i<Nch;i++){
          adcH[i] = adcL[i] = 0;
          tdcL[i] = tdcT[i] = -9999;
          npe[i] = time[i] = width[i] = -9999;
          scaler[i] = -99;
        }

        char headerByte[4];
        datFile.read(headerByte, 4);
        unsigned int header = getBigEndian32(headerByte);
        //cout<<"HEADER = "<<hex<<header<<endl;
        bool isHeader = ((header >> 27) & 0x01) == 0x01;
        if(!isHeader) {
            std::cerr << "Frame Error" << std::endl;
            fprintf(stderr, "    %08X\n", header);
            std::exit(1);
        }
        size_t dataSize = header & 0x0fff;
        //cout<<events<<endl;

        unsigned int scalerValues[69];
        char* dataBytes = new char[dataSize * 4];
        datFile.read(dataBytes, dataSize * 4);
        for(size_t i = 0; i < dataSize; ++i) {
            unsigned int data = getBigEndian32(dataBytes + 4 * i);
            //cout<<"DATA = "<<formatBin(data)<<endl;
            if(isCheck(data)) {
                std::cout << formatBin(data) <<std::endl;
                std::cerr << "Unknown data type " << std::endl;
                std::cout << std::endl;
                break;
            }else if(isAdcHg(data)) {
                int ch = (data >> 13) & 0x3f;
                bool otr = ((data >> 12) & 0x01) != 0;
                int value = data & 0x0fff;
                //cout<<"ADCHG = "<<hex<<data<<"  "<<dec<<ch<<"  "<<otr<<"  "<<value<<endl;
                if(!otr && ch < Nch) {
                    adcH[ch] = value;
                }
            }else if(isAdcLg(data)) {
                int ch = (data >> 13) & 0x3f;
                bool otr = ((data >> 12) & 0x01) != 0;
                int value = data & 0x0fff;
                //cout<<"ADCLG = "<<hex<<data<<"  "<<dec<<ch<<"  "<<otr<<"  "<<value<<endl;
                if(!otr && ch < Nch) {
                    adcL[ch] = value;
                }
            }else if(isTdcLeading(data)) {
                int ch = (data >> 13) & 0x3f;
                int value = data & 0x0fff;
                //cout<<"TDCL = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
                if(ch < Nch) {
                  tdcL[ch] = value;
                }
            }else if(isTdcTrailing(data)) {
                int ch = (data >> 13) & 0x3f;
                int value = data & 0x0fff;
                //cout<<"TDCT = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
                if(ch < Nch) {
                  tdcT[ch] = value;
                }
            }else if(isScaler(data)) {
                int ch = (data >> 14) & 0x7f;
                int value = data & 0x3fff;
                //cout<<"Scaler = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
                if(ch < 68) {
                    scalerValues[ch] = value;
                    //cout << "event:"<<events<<"/scalerValues["<<ch<<"]:"<<scalerValues[ch] << endl; 
                }else if(ch == 68) {
                    int scalerValuesArrayIndex = events % ScalerInt;
                    //cout<<"COPY "<<scalerValuesArrayIndex<<endl;
                    memcpy(scalerValuesArray[scalerValuesArrayIndex], scalerValues,
                           sizeof(scalerValues));
                }
            }else {
                int ch = (data >> 13) & 0x3f;
                int value = data & 0x0fff;
                std::cout << "adchg:"  << (data & 0x00680000);
                std::cout << "adclg:"  << (data & 0x00680000);
                std::cout << "tdcl:"   << (data & 0x00601000);
                std::cout << "tdct:"   << (data & 0x00601000);
                std::cout << "scaler:" << (data & 0x00600000);
                std::cout << "data:" << data << std::endl; 
                std::cout << "ch:" << ch << " value:" << value << std::endl;
                std::cerr << "Unknown data type" << std::endl;
            }
        }

        double gain[Nch], pedestal[Nch], timeOffset[Nch], timeScale[Nch];

        if (!loadParamJSON(param_filename.c_str(), gain, pedestal, timeOffset, timeScale)) {
            return;
        }

        for(int ch = 0;ch < Nch; ch++){
          adcoff[ch] = adcH[ch] - pedestal[ch];
          npe[ch] = adcoff[ch] * gain[ch];
          time[ch] = (tdcL[ch] - timeOffset[ch]) * timeScale[ch];
          width[ch] = (tdcL[ch] - tdcT[ch]) * timeScale[ch];
        }

        evid = events;

        delete[] dataBytes;
        events++;
	      if(events%25000==0) std::cout << "reading events#:" << events << std::endl;
#if 1
        if(events % ScalerInt == 0) {
            unsigned int scalerValuesSum[69];
            for(int i = 0; i < 69; ++i) {
                scalerValuesSum[i] = 0;
            }
            for(int i = 0; i < ScalerInt; ++i) {
		            for(int j = 0; j < 69; ++j) {
                    scalerValuesSum[j] += scalerValuesArray[i][j];
                }
            }

            int counterCount1MHz = scalerValuesSum[67];
            //int counterCount1KHz = scalerValuesSum[68];

            // 1count = 1.0ms
            //double counterCount = (double)counterCount1KHz + counterCount1MHz / 1000.0;
            double counterCount = counterCount1MHz / 1E+6;
            duration = counterCount;
	    //cout << "counterCount" << counterCount << endl;
            // TODO
            // Firmwareのバグを直したら消す
            //counterCount /= 2.0;

            //cout << "counterCount: " << counterCount << endl;
            for(size_t j = 0; j < Nch; ++j) {
                scaler[j] = scalerValuesSum[j];
                //double scalerCount = scalerValuesSum[j] & 0x1fff;  //changed by N.CHIKUMA 2015 Oct 6
                //cout << "scalerCount: " << scalerCount << ", ";
            }
            //cout << endl;
            //cout << endl;
        }
        tree->Fill();
#endif
    }
    tree->Write();
//    f->Write();
    f->Close();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, char** argv)
{

  string input_filename  = "hoge.dat";
  string output_filename = "hoge.root";
  string param_filename = "param/default.json";
  int ch;
  extern char *optarg;
  while((ch=getopt(argc,argv,"hf:w:p:"))!=-1){
    switch(ch){
    case 'f':
      input_filename = optarg;
      cout<<"input filename : "<<input_filename<<endl;
      break;
    case 'w':
      output_filename = optarg;
      cout<<"output filename : "<<output_filename<<endl;
      break;
    case 'p':
      param_filename = optarg;
      cout<<"parameter filename : "<<param_filename<<endl;
      break;
    case 'h':
      cout<<"example) ./offline.cc -f datfile -w rootfile -p jsonfile"<<endl;
      cout<<"-f : input filename"<<endl;
      cout<<"-w : output filename"<<endl;
      cout<<"-p : param filename"<<endl;
      return 0;
      break;
    case '?':
      cout<<"unknown option...."<<endl;
      return 0;
      break;
    default:
      cout<<"type -h to see help!!"<<endl;
      return 0;
    }
  }

   if(ends_with(input_filename, ".dat")){
     DecodeData(input_filename, output_filename, param_filename);
   } else if(ends_with(input_filename, ".tar.xz")){
     std::string tempDat = input_filename + ".temp";
     if (!extractDatFilesFromTarXZ(input_filename, tempDat)) {
       std::cerr << "Extraction failed." << std::endl;
       return 1;
     }
     DecodeData(tempDat, output_filename, param_filename);
     std::remove(tempDat.c_str());  // 一時ファイル削除
    } else {
        std::cerr << "Unsupported input file type: " << input_filename << std::endl;
        return 1;
    }

//  DecodeData(input_filename, output_filename, param_filename);

  return 0;
}
