#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

struct Cliente {
    int demanda;
    int custoTerceirizacao;
    bool atendido = false;
};

struct Veiculo {
    int capacidadeRestante;
    vector<int> rota;
};

void debug(int veiculoAtual, int clienteAtual, int custoAdicionado, int custoTotal, const string& tipoCusto) {
    cout << "DEBUG: " << tipoCusto << endl;
    cout << "Veículo " << veiculoAtual + 1 << ", Cliente " << clienteAtual + 1 << ": ";
    cout << "Custo Adicionado = " << custoAdicionado << ", ";
    cout << "Custo Total Cumulativo = " << custoTotal << endl;
}

int main() {
    ifstream arquivo("entrada2.txt");
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo de entrada." << endl;
        return 1;
    }

    int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo;
    arquivo >> totalClientes >> totalVeiculos >> capacidadeVeiculo >> entregasMinimas >> custoVeiculo;

    vector<Cliente> clientes(totalClientes);
    for (int i = 0; i < totalClientes; i++) {
        arquivo >> clientes[i].demanda;
    }

    for (int i = 0; i < totalClientes; i++) {
        arquivo >> clientes[i].custoTerceirizacao;
    }


    vector<vector<int>> custoRota(totalClientes + 1, vector<int>(totalClientes + 1, 0));
    for (int i = 0; i <= totalClientes; i++) {
        for (int j = 0; j <= totalClientes; j++) {
            arquivo >> custoRota[i][j];
        }
    }

    arquivo.close();

    vector<Veiculo> veiculos(totalVeiculos, {capacidadeVeiculo});
    int custoTotal = 0;

    for (int v = 0; v < totalVeiculos; v++) {
        auto& veiculo = veiculos[v];
        veiculo.rota.push_back(0);

        while (veiculo.capacidadeRestante > 0) {
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
            debug(v, clienteMaisProximo, custoMinimo, custoTotal, "Custo do trajeto");
        }
        
        int ultimoCliente = veiculo.rota.back();
        int custoRetorno = custoRota[ultimoCliente][0];
        custoTotal += custoRetorno;
        debug(v, 0, custoRetorno, custoTotal, "Custo de retorno ao depósito");

        custoTotal += custoVeiculo;
        debug(v, 0, custoVeiculo, custoTotal, "Custo fixo do veículo");
    }

    for (int i = 0; i < totalClientes; i++) {
    if (!clientes[i].atendido) {
        custoTotal += clientes[i].custoTerceirizacao;
        debug(0, i, clientes[i].custoTerceirizacao, custoTotal, "Custo de terceirização");
    }
}

//    for (int i = 0; i < totalClientes; i++) {
//    cout << "Cliente " << i+1 << ": Terceirização = " << clientes[i].custoTerceirizacao << endl;
//}

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
        if (!clientes[i].atendido) {
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

    return 0;
}
