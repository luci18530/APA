#include <iostream>
#include <vector>
#include <limits>
#include <fstream>

using namespace std;

struct Cliente {
    int demanda;
    int custoTerceirizacao;
    bool atendido = false;
    bool terceirizado = false;
};

struct Veiculo {
    int capacidadeRestante;
    vector<int> rota;
};

class CVRP {
private:
    vector<Cliente> clientes;
    vector<Veiculo> veiculos;
    vector<vector<int>> custoRota;
    int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo, custoTotal = 0;

    void debug(int veiculoAtual, int clienteAtual, int custoAdicionado, const string& tipoCusto) {
        cout << "DEBUG: " << tipoCusto << endl;
        cout << "Veículo " << veiculoAtual + 1 << ", Cliente " << clienteAtual + 1 << ": ";
        cout << "Custo Adicionado = " << custoAdicionado << ", ";
        cout << "Custo Total Cumulativo = " << custoTotal << endl;
    }

public:
    CVRP(const string& nomeArquivo) {
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir o arquivo de entrada." << endl;
            exit(1);
        }

        arquivo >> totalClientes >> totalVeiculos >> capacidadeVeiculo >> entregasMinimas >> custoVeiculo;
        clientes.resize(totalClientes);
        for (int i = 0; i < totalClientes; i++) {
            arquivo >> clientes[i].demanda;
        }

        for (int i = 0; i < totalClientes; i++) {
            arquivo >> clientes[i].custoTerceirizacao;
        }

        custoRota.resize(totalClientes + 1, vector<int>(totalClientes + 1, 0));
        for (int i = 0; i <= totalClientes; i++) {
            for (int j = 0; j <= totalClientes; j++) {
                arquivo >> custoRota[i][j];
            }
        }

        arquivo.close();
        veiculos.resize(totalVeiculos, { capacidadeVeiculo });
    }

    void roteamentoVeiculos() {
        int entregasRealizadas = 0;
        for (int v = 0; v < totalVeiculos && entregasRealizadas < entregasMinimas; v++) {
            auto& veiculo = veiculos[v];
            veiculo.rota.push_back(0);

            while (veiculo.capacidadeRestante > 0 && entregasRealizadas < entregasMinimas) {
                int clienteMaisProximo = -1;
                int custoMinimo = numeric_limits<int>::max();

                for (int i = 0; i < totalClientes; i++) {
                    if (!clientes[i].atendido && clientes[i].demanda <= veiculo.capacidadeRestante) {
                        int ultimoCliente = veiculo.rota.back();
                        int custo = custoRota[ultimoCliente][i+1];

                        if (custo < custoMinimo) {
                            clienteMaisProximo = i;
                            custoMinimo = custo;
                        }
                    }
                }

                if (clienteMaisProximo == -1) break;

                clientes[clienteMaisProximo].atendido = true;
                veiculo.rota.push_back(clienteMaisProximo + 1);
                veiculo.capacidadeRestante -= clientes[clienteMaisProximo].demanda;
                custoTotal += custoMinimo;
                entregasRealizadas++;
                debug(v, clienteMaisProximo, custoMinimo, "Custo do trajeto");
            }

            int ultimoCliente = veiculo.rota.back();
            int custoRetorno = custoRota[ultimoCliente][0];
            custoTotal += custoRetorno;
            debug(v, 0, custoRetorno, "Custo de retorno ao depósito");

            custoTotal += custoVeiculo;
            debug(v, 0, custoVeiculo, "Custo fixo do veículo");
        }
    }

    void avaliacaoTerceirizacao() {
        for (int i = 0; i < totalClientes; i++) {
            if (!clientes[i].atendido) {
                int custoIdaEVolta = custoRota[0][i+1] + custoRota[i+1][0];
                if (clientes[i].custoTerceirizacao < custoIdaEVolta) {
                    clientes[i].terceirizado = true;
                    custoTotal += clientes[i].custoTerceirizacao;
                    debug(-1, i, clientes[i].custoTerceirizacao, "Custo de terceirização");
                }
            }
        }
    }

    void exibirResultados() {
        cout << "Custo total: " << custoTotal << endl;
        for (int i = 0; i < totalVeiculos; i++) {
            cout << "Rota do veiculo " << i+1 << ": Depósito ";
            for (int cliente : veiculos[i].rota) {
                if (cliente == 0) continue;
                cout << "-> Cliente " << cliente << " ";
            }
            if (veiculos[i].rota.size() == 1) {
                cout << "(veículo não utilizado)";
            }
            cout << "-> Depósito" << endl;
        }

        cout << "Clientes terceirizados: ";
        bool primeiro = true;
        for (int i = 0; i < totalClientes; i++) {
            if (clientes[i].terceirizado) {
                if (!primeiro) {
                    cout << ", ";
                }
                cout << "Cliente " << i + 1;
                primeiro = false;
            }
        }
        if (primeiro) {
            cout << "Nenhum cliente foi terceirizado.";
        }
        cout << endl;
    }
};

int main() {
    CVRP problema("entrada2.txt");
    problema.roteamentoVeiculos();
    problema.avaliacaoTerceirizacao();
    problema.exibirResultados();
    return 0;
}
