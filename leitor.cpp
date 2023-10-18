#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main() {
    // Declaração de variáveis
    int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo;
    vector<int> demandaClientes, custoTerceirizacao;
    vector<vector<int>> custoRota;

    // Abrindo o arquivo para leitura
    ifstream arquivoEntrada("entrada.txt");
    if (!arquivoEntrada.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    // Leitura das informações básicas
    arquivoEntrada >> totalClientes >> totalVeiculos >> capacidadeVeiculo >> entregasMinimas >> custoVeiculo;

    // Leitura da demanda dos clientes
    demandaClientes.resize(totalClientes);
    for (int i = 0; i < totalClientes; i++) {
        arquivoEntrada >> demandaClientes[i];
    }

    // Leitura do custo de terceirização para cada cliente
    custoTerceirizacao.resize(totalClientes);
    for (int i = 0; i < totalClientes; i++) {
        arquivoEntrada >> custoTerceirizacao[i];
    }

    // Leitura da matriz de custo de rota
    custoRota.resize(totalClientes + 1, vector<int>(totalClientes + 1));
    for (int i = 0; i <= totalClientes; i++) {
        for (int j = 0; j <= totalClientes; j++) {
            arquivoEntrada >> custoRota[i][j];
        }
    }

    arquivoEntrada.close();

    // TODO: lógica da busca local / meta-heurística

    return 0;
}
