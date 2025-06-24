#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <queue>
#include <fstream>
#include <unordered_map>

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
            try {
                log.open("requests.log", ios::app);
                if(!log) {
                    throw new runtime_error("erro");
                } else {
                    log << req.ip << " -- " << ctime(&req.timeStamp) << endl;
                }
            } catch(runtime_error) {
                cout << "erro ao abrir arquivo de logging" << endl;
            }
            log.close();
        }

        void saveBlockedIps(requestData req) {
            ofstream file;
            try {
                file.open("blocked_ips.txt", ios::app);
                if(!file) {
                    throw new runtime_error("erro");
                } else {
                    file << req.ip << endl;
                }
            } catch(runtime_error) {
                cout << "erro ao abrir o arquivo de ips bloqueados" << endl;
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
            queue<requestData> requestQueue;
            FileManager manager;
    public:
        void saveRequest(requestData req, unordered_map<string, bool> blocked) {
            time_t now = time(NULL);
            req.timeStamp = now;
            requestQueue.push(req);

            if(requestQueue.size() > QUEUE_LIMIT) {
                requestQueue.pop();
            }
            
            cout << "Requisição registrada de " << req.ip << " em " << ctime(&req.timeStamp) << endl;
            manager.logRequest(req);

            if(blocked[req.ip]) {
                cout << "@@@" << req.ip << " está fazendo requisições, porém está bloqueado" << endl;
                return;
            }

            verifyDos(req);
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
                cout << "Possivel ataque de " << req.ip << endl << IP_LIMIT << " requisicoes em menos de " << SECONDS << " segundos" << endl;
                manager.saveBlockedIps(req);
                //system("sleep 10");
            }
        }
};



int main() {
    Detector scanner;
    FileManager manager;
    requestData req;

    srand(time(NULL));

    req.timeStamp = 0;
    while(true) {
        req.ip = "192.168.0." + to_string((rand() % 256));
        manager.loadBlockedIps();
        scanner.saveRequest(req, manager.getBlockedList());
    }
}
