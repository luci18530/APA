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

    // custo_rota = [35, 61, 27, 66, 75]
    // custo_terceirizacoes = [15, 14, 32, 22]
    // custo_caminhoes = custo_caminhao*quantidade_veiculos
    // Função para aplicar a melhoria 2-opt a uma rota

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

vector<int> vnd(vector<int> &rota)
{
    // Abordagem 2-Opt
    vector<int> rotaMelhorada = rota;
    bool melhorRota = true;

    while (melhorRota)
    {
        melhorRota = false;
        for (int i = 1; i < rotaMelhorada.size() - 2; i++)
        {
            for (int j = i + 1; j < rotaMelhorada.size() - 1; j++)
            {
                vector<int> novaRota = rotaMelhorada;
                // Aplicar a troca 2-opt
                for (int k = i, l = j; k < l; k++, l--)
                {
                    swap(novaRota, k, l);
                }

                double avaliacaoAtual = calculaCustoRota(rotaMelhorada);
                double novaAvaliacao = calculaCustoRota(novaRota);

                if (novaAvaliacao < avaliacaoAtual)
                {
                    rotaMelhorada = novaRota;
                    melhorRota = true;
                }
            }
        }
    }

    return rotaMelhorada;
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
    vector<int> rota = {0, 3, 4, 5, 1, 6, 2};
    int custoInicial = calculaCustoRota(rota);
    vector<int> novaRota = vnd(rota);
    int custoNovo = calculaCustoRota(novaRota);

    cout << "Custo inicial da rota: " << custoInicial << endl;
    cout << "Custo após 2-opt: " << custoNovo << endl;

    return 0;
}
