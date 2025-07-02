#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <queue>
#include <fstream>
#include <unordered_map>
#include <pcapplusplus/PcapLiveDeviceList.h>
#include <pcapplusplus/SystemUtils.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/IPv4Layer.h>

using namespace std;

const int QUEUE_LIMIT = 10, IP_LIMIT = 6, SECONDS = 5;

struct requestData {
    string ip;
    time_t timeStamp;
};

class FileManager {
    private:
        unordered_map<string, bool> blocked;
    public:
        void logRequest(requestData req) {
            ofstream log;
            log.open("requests.log", ios::app);
            if(!log) {
                throw new runtime_error("erro");
            } else {
                log << req.ip << " -- " << ctime(&req.timeStamp) << endl;
            }
            log.close();
        }

        void saveBlockedIps(requestData req) {
            ofstream file;
            file.open("blocked_ips.txt", ios::app);
            if(!file) {
                throw new runtime_error("erro");
            } else {
                file << req.ip << endl;
            }
            file.close();
        }

        void loadBlockedIps() {
            ifstream file("blocked_ips.txt");
            string blockedIp;
            while(getline(file, blockedIp)) {
                blocked[blockedIp] = true;
            }
            file.close();
        }

        unordered_map<string, bool> getBlockedList() {
            return blocked;
        }
};

class Detector {
    private:
        FileManager manager;
        requestData req;
        queue<requestData> requestQueue;
        vector<pcpp::PcapLiveDevice*> devList = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();
        pcpp::PcapLiveDevice *device = devList[0];
    public:
        void saveRequest(requestData req, unordered_map<string, bool> blocked) {
            time_t now = time(NULL);
            req.timeStamp = now;
            requestQueue.push(req);

            if(requestQueue.size() >= QUEUE_LIMIT) {
                requestQueue.pop();
            }

            if(blocked[req.ip]) {
                cout << "\033[33m[AVISO]   " << req.ip << " está fazendo requisições, porém está bloqueado" << endl << endl;
                return;
            } else {
                clog << "\033[32m[INFO]    Requisição registrada de " << req.ip << " em " << ctime(&req.timeStamp) << endl;
                try {
                    manager.logRequest(req);
                } catch(const exception&) {{
                    cerr << "[ERRO]\tErro ao abrir arquivo de log" << endl;
                }}
                verifyDos(req);
            }

        }

        void verifyDos(requestData req) {
            if(requestQueue.size() < IP_LIMIT) {
                return;
            }

            vector<requestData> aux;
            while(!requestQueue.empty()) {
                aux.push_back(requestQueue.front());
                requestQueue.pop();
            }
            
            int numberOfResquests = 1;
            bool detectedAttack = false;
            requestData mostRecentRequest = aux.front();
            for(int i = 1; (i <= IP_LIMIT) && (detectedAttack == false); i++) {
                if((difftime((time_t) (mostRecentRequest.timeStamp), aux[i].timeStamp) <= SECONDS) && (mostRecentRequest.ip == aux[i].ip)) {
                    numberOfResquests++;
                    if(numberOfResquests >= IP_LIMIT) {
                        detectedAttack = true;
                    }
                }
            }

            if(detectedAttack) {
                cout << "\033[31m[ALERTA]  Possivel ataque de " << req.ip << endl << endl;
                try {
                    manager.saveBlockedIps(req);
                } catch(const exception&) {
                    cerr << "[ERRO]\tErro ao salvar ip bloqueado" << endl;
                }
            }
        }

        void startCapturing() {
            if(devList.empty()) {
                cerr << "[ERRO]\tNenhuma interface de rede encontrada" << endl;
            }
            if(device == NULL) {
                cerr << "[ERRO]\tErro ao encontrar interface de captura de pacotes" << endl;
            }
            if(!device->open()) {
                cerr << "[ERRO]\tErro ao abrir interface de captura de pacores" << endl;
            }
            cout << "Capturando pacotes com a interface: " << device->getName() << endl;

            device->startCapture(onPacketArrives, this);

            while(true) {
                pcpp::multiPlatformSleep(1);
            }
            device->stopCapture();
        }

        static void onPacketArrives(pcpp::RawPacket *rawPacket, pcpp::PcapLiveDevice *device, void *cookie) {
            auto *dec = static_cast<Detector*>(cookie);
            try {
                dec->manager.loadBlockedIps();
            } catch(const exception&) {
                cerr << "Erro ao abrir arquivos de IPs bloqueados" << endl;
            }
            pcpp::Packet packet(rawPacket);

            if(packet.isPacketOfType(pcpp::IPv4)) {
                pcpp::IPv4Layer *ip = packet.getLayerOfType<pcpp::IPv4Layer>();
                if(ip) {
                    dec->req.ip = ip->getSrcIPAddress().toString();
                    dec->req.timeStamp = time(NULL);

                    dec->saveRequest(dec->req, dec->manager.getBlockedList());
                }
            }
        }
};

int main() {
    Detector scanner;
    scanner.startCapturing();
    return 0;
}
