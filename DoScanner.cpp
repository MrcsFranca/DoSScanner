#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <queue>

using namespace std;

const int QUEUE_LIMIT = 10, IP_LIMIT = 6, SECONDS = 5;

struct requestData {
    string ip;
    time_t timeStamp;
};

class Detector {
    private:
            queue<requestData> filaRequisicoes;
    public:
        void saveRequest(requestData req) {
            time_t now = time(NULL);
            req.timeStamp = now;
            filaRequisicoes.push(req);

            if(filaRequisicoes.size() > QUEUE_LIMIT) {
                filaRequisicoes.pop();
            }
            
            cout << "Requisição registrada de " << req.ip << " em " << ctime(&req.timeStamp) << endl;
            verifyDos(req);
        }

        void verifyDos(requestData req) {
            if(filaRequisicoes.size() < IP_LIMIT) {
                return;
            }

            vector<requestData> aux;
            while(!filaRequisicoes.empty()) {
                aux.push_back(filaRequisicoes.front());
                filaRequisicoes.pop();
            }
            
            int numberOfResquests = 1;
            bool detectedAttack = false;
            requestData mostRecentRequest = aux.front();
            for(int i = 1; (i <= IP_LIMIT) && (detectedAttack == false); i++) {
                if((difftime((time_t) (mostRecentRequest.timeStamp), aux[i].timeStamp) <= SECONDS) && (mostRecentRequest.ip == aux[i].ip)) {
                    cout << "numero : " << numberOfResquests << endl;
                    numberOfResquests++;
                    if(numberOfResquests >= IP_LIMIT) {
                        detectedAttack = true;
                    }
                }
            }

            if(detectedAttack) {
                cout << "Possivel ataque de " << req.ip << endl << IP_LIMIT << " requisicoes em menos de " << SECONDS << " segundos" << endl;
            }
        }
};

int main() {
    Detector scanner;
    requestData req;
    req.timeStamp = 0;
    for(int i = 0; i < 13; i++) {
        req.ip = "192.168.0." + to_string(i);
        scanner.saveRequest(req);
    }
}
