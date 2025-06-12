#include <iostream>
#include <chrono>
#include <ctime>
#include <stack>
#include <vector>
#include <string>

using namespace std;

const int QUANT_REQUISICOES = 10, LIMITE_IP = 6, INTERVALO_SEGUNDOS = 5;

struct requestData {
    string ip;
    time_t timeStamp;
};

class Detector {
    private:
            stack<requestData> pilhaRequisicoes;
    public:
        void saveRequest(requestData req) {
            time_t now = time(NULL);
            req.timeStamp = now;
            pilhaRequisicoes.push(req);

            if(pilhaRequisicoes.size() > QUANT_REQUISICOES) {
                vector<requestData> temp;
                while(pilhaRequisicoes.size() > 1) {
                    temp.push_back(pilhaRequisicoes.top());
                    pilhaRequisicoes.pop();
                }
                pilhaRequisicoes.pop();
                for(auto it = temp.begin(); it != temp.end(); it++) {
                    pilhaRequisicoes.push(*it);
                }
            }
            stack<requestData> temp;
            requestData aux;
        }

        /*
        void verifyDos() {

        }
        */
        
        
        void imprimePilha() {
            requestData aux;
            while(pilhaRequisicoes.size() > 0) {
                aux = pilhaRequisicoes.top();
                cout << aux.ip << " : " << ctime(&aux.timeStamp) << endl;
                pilhaRequisicoes.pop();
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
    scanner.imprimePilha();
    
}
