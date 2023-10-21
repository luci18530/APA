#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Declaração de variáveis
int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo;
vector<int> demandaClientes, custoTerceirizacao;
vector<vector<int>> custoRotas;

int calculaCustoRota(vector<int> &rota)
{
    // CUSTO GLOBAL -> SUM(custo_rota) + custo_caminhoes + custo_terceirizacoes
    int custo = 0;
    int deposito = rota[0];
    int ultimo_destino = rota.back();

    for (int i = 0; i < rota.size() - 1; i++)
    {
        int origem = rota[i];
        int destino = rota[i + 1];
        custo += custoRotas[origem][destino];
    }

    custo += custoRotas[ultimo_destino][deposito];

    return custo;
}

void swap(vector<int> &rota, int i, int j)
{
    int aux = rota[i];
    rota[i] = rota[j];
    rota[j] = aux;
}

int main()
{
    // Abrindo o arquivo para leitura
    ifstream arquivoEntrada("entrada.txt");
    if (!arquivoEntrada.is_open())
    {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    // Leitura das informações básicas
    arquivoEntrada >> totalClientes >> totalVeiculos >> capacidadeVeiculo >> entregasMinimas >> custoVeiculo;

    // Leitura da demanda dos clientes
    demandaClientes.resize(totalClientes);
    for (int i = 0; i < totalClientes; i++)
    {
        arquivoEntrada >> demandaClientes[i];
    }

    // Leitura do custo de terceirização para cada cliente
    custoTerceirizacao.resize(totalClientes);
    for (int i = 0; i < totalClientes; i++)
    {
        arquivoEntrada >> custoTerceirizacao[i];
    }

    // Leitura da matriz de custo de rota
    custoRotas.resize(totalClientes + 1, vector<int>(totalClientes + 1));
    for (int i = 0; i <= totalClientes; i++)
    {
        for (int j = 0; j <= totalClientes; j++)
        {
            arquivoEntrada >> custoRotas[i][j];
        }
    }

    arquivoEntrada.close();

    // TODO: lógica da busca local / meta-heurística

    // TESTE CALCULA CUSTO ROTA
    // vector<int> rota = {0, 3, 4, 5};
    // int custo = calculaCustoRota(rota);
    // cout << custo << endl;

    return 0;
}
