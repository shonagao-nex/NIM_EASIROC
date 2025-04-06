#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <bitset>

#include <TFile.h>
#include <TTree.h>
#include <TH1.h>

using namespace std;

#include <iostream>
#include <bitset>

#include <iostream>
#include <bitset>

std::string formatBin(int num) {
    std::bitset<32> bits(num);
    std::string binaryStr = bits.to_string();
    std::string formattedStr = "";

    const int SPACE_POS[] = {12, 13, 20};
    const int SPACE_COUNT = sizeof(SPACE_POS) / sizeof(SPACE_POS[0]);

    for (size_t i = 0; i < binaryStr.size(); i++) {
        formattedStr += binaryStr[i];

        for (int j = 0; j < SPACE_COUNT; j++) {
            if (i == 31 - SPACE_POS[j]) {
                formattedStr += " ";
            }
        }
    }
    return formattedStr;
}

//std::string formatBin(int num) {
//    std::bitset<32> bits(num); // 32ビットのビットセット
//    std::string binaryStr = bits.to_string(); // 文字列に変換
//    std::string formattedStr = "";
//
//    for (size_t i = 0; i < binaryStr.size(); i++) {
//        formattedStr += binaryStr[i];
//        if ((i + 1) % 8 == 0 && i != binaryStr.size() - 1) {
//            formattedStr += " "; // 8桁ごとにスペースを追加
//        }
//    }
//    return formattedStr;
//}


unsigned int getBigEndian32(const char* b)
{
    return ((b[0] << 24) & 0xff000000) |
           ((b[1] << 16) & 0x00ff0000) |
           ((b[2] <<  8) & 0x0000ff00) |
           ((b[3] <<  0) & 0x000000ff);
}

bool isCheck(unsigned int data)
{
    int val = (data >> 24) & 0xff;
    return val != 0;
}

bool isAdcHg(unsigned int data)
{
    return (data & 0x00680000) == 0x00000000;
}

bool isAdcLg(unsigned int data)
{
    return (data & 0x00680000) == 0x00080000;
}

bool isTdcLeading(unsigned int data)
{
    return (data & 0x00601000) == 0x00201000;
}

bool isTdcTrailing(unsigned int data)
{
    return (data & 0x00601000) == 0x00200000;
}

bool isScaler(unsigned int data)
{
    return (data & 0x00600000) == 0x00400000;
}

void hist(const string& filename)
{
    string::size_type pos = filename.find(".dat");
    if(pos == string::npos) {
        cerr << filename << " is not a dat file" << endl;
        return;
    }
    string rootfile_name(filename);
    rootfile_name.replace(pos, 5, ".root");

    TFile *f = new TFile(rootfile_name.c_str(), "RECREATE");
    TTree *tree = new TTree("tree","tree");
    int evid;
    int adcH[64], adcL[64];
    int tdcL[64], tdcT[64];

    tree->Branch("evid", &evid, "evid/I" );
    tree->Branch("adcH",  adcH, "adcH[16]/I" );
    tree->Branch("adcL",  adcL, "adcL[16]/I" );
    tree->Branch("tdcL",  tdcL, "tdcL[16]/I" );
    tree->Branch("tdcT",  tdcT, "tdcT[16]/I" );

//    TH1F* scaler[67];

    int nbin = 4096;
    for(int i = 0; i < 64; ++i) {
//        scaler[i] = new TH1F(Form("SCALER_%d", i),
//                             Form("Scaler %d", i),
//                             //4096, 0, 5.0);
//                             nbin, 0, 5.0*20.);
    }
//    scaler[64] = new TH1F("SCALER_OR32U", "Scaler OR32U",
//                          4096, 0, 200);
//    scaler[65] = new TH1F("SCALER_OR32L", "Scaler OR32L",
//                          4096, 0, 200);
//    scaler[66] = new TH1F("SCALER_OR64", "Scaler OR64",
//                          //4096, 0, 200);
//                          4096*10, 0, 200*10);


    ifstream datFile(filename.c_str(), ios::in | ios::binary);
    unsigned int scalerValuesArray[10][69];
    unsigned int events = 0;
    while(datFile) {
        for(int i=0;i<64;i++){
          adcH[i] = adcL[i] = 0;
          tdcL[i] = tdcT[i] = -9999;
        }

//cout<<"EventID = "<<dec<<events<<endl;
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
                if(!otr) {
//                    adcHigh[ch]->Fill(value);
                    adcH[ch] = value;
                }
            }else if(isAdcLg(data)) {
                int ch = (data >> 13) & 0x3f;
                bool otr = ((data >> 12) & 0x01) != 0;
                int value = data & 0x0fff;
//cout<<"ADCLG = "<<hex<<data<<"  "<<dec<<ch<<"  "<<otr<<"  "<<value<<endl;
                if(!otr) {
//                    adcLow[ch]->Fill(value);
                    adcL[ch] = value;
                }
            }else if(isTdcLeading(data)) {
                int ch = (data >> 13) & 0x3f;
                int value = data & 0x0fff;
//cout<<"TDCL = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
//                tdcLeading[ch]->Fill(value);
                tdcL[ch] = value;
            }else if(isTdcTrailing(data)) {
                int ch = (data >> 13) & 0x3f;
                int value = data & 0x0fff;
//cout<<"TDCT = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
//                tdcTrailing[ch]->Fill(value);
                tdcT[ch] = value;
            }else if(isScaler(data)) {
                int ch = (data >> 14) & 0x7f;
                int value = data & 0x3fff;
//cout<<"Scaler = "<<hex<<data<<"  "<<dec<<ch<<"  "<<value<<endl;
                if(ch < 68) {
                    scalerValues[ch] = value;
                  //cout << "event:"<<events<<"/scalerValues["<<ch<<"]:"<<scalerValues[ch] << endl; 
                }else if(ch == 68) {
                    int scalerValuesArrayIndex = events % 10;
//cout<<"COPY "<<scalerValuesArrayIndex<<endl;
                    memcpy(scalerValuesArray[scalerValuesArrayIndex], scalerValues,
                           sizeof(scalerValues));
                }
#if 1
//                if(ch == 68) {
//                    int scalerValuesArrayIndex = events % 100;
//                    memcpy(scalerValuesArray[scalerValuesArrayIndex], scalerValues,
//                           sizeof(scalerValues));
//                }
#else

                if(ch == 68) {
                    int counterCount1MHz = scalerValues[67] & 0x1fff;
                    int counterCount1KHz = scalerValues[68] & 0x1fff;

                    // 1count = 1.0ms
                    double counterCount = (double)counterCount1KHz + counterCount1MHz / 1000.0;
                    // TODO
                    // Firmwareのバグを直したら消す
                    counterCount *= 2.0;
                    //cout << "counterCount: " << counterCount << endl;
                    for(size_t j = 0; j < 67; ++j) {
                        bool ovf = ((scalerValues[j] >> 13) & 0x01) != 0;
                        ovf = false;
                        double scalerCount = scalerValues[j] & 0x1fff;
                        //cout << "scalerCount: " << j << " " << scalerCount << endl;
                        if(!ovf && scalerCount != 0) {
                            double rate = scalerCount / counterCount; // kHz
                            //cout << "rate: " << rate << endl;
                            scaler[j]->Fill(rate);
                        }
                    }
                    //cout << endl;
                    //cout << endl;
                }
#endif
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
        evid = events;
        tree->Fill();

        delete[] dataBytes;
        events++;
	if(events%25000==0) std::cout << "reading events#:" << events << std::endl;
#if 1
        if(events % 10 == 0) {
            unsigned int scalerValuesSum[69];
            for(int i = 0; i < 69; ++i) {
                scalerValuesSum[i] = 0;
            }
            for(int i = 0; i < 10; ++i) {
		for(int j = 0; j < 69; ++j) {
                    scalerValuesSum[j] += scalerValuesArray[i][j];
                }
            }

            int counterCount1MHz = scalerValuesSum[67];
            int counterCount1KHz = scalerValuesSum[68];

            // 1count = 1.0ms
            double counterCount = (double)counterCount1KHz + counterCount1MHz / 1000.0;
	    //cout << "counterCount" << counterCount << endl;
            // TODO
            // Firmwareのバグを直したら消す
            counterCount /= 2.0;

            //cout << "counterCount: " << counterCount << endl;
            for(size_t j = 0; j < 67; ++j) {
                //cout << j << " scalerValuesSun: " << scalerValuesSum[j] << ", ";
		bool ovf = ((scalerValuesSum[j] >> 13) & 0x01) != 0;
                ovf = true;
                //double scalerCount = scalerValuesSum[j] & 0x1fff;  //changed by N.CHIKUMA 2015 Oct 6
                double scalerCount = scalerValuesSum[j] & 0xffff;
                //cout << "scalerCount: " << scalerCount << ", ";
                if(!ovf && scalerCount != 0) {
                    double rate = scalerCount / counterCount;
                    //cout << "rate: " << rate << endl;
//                    scaler[j]->Fill(rate);
                }
            }
            //cout << endl;
            //cout << endl;
        }
#endif
    }
    tree->Write();
//    f->Write();
    f->Close();
}

int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "hist <dat file>" << endl;
        return -1;
    }
    hist(argv[1]);

    return 0;
}
