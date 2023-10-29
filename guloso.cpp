#include <iostream>
#include <vector>
#include <limits>
#include <fstream>

using namespace std;

struct Cliente
{
    int demanda;               // Demanda de cada cliente
    int custoTerceirizacao;    // Custo para terceirizar o pedido do cliente
    bool atendido = false;     // Indica se o cliente foi atendido
    bool terceirizado = false; // Indica se o cliente foi terceirizado
};

struct Veiculo
{
    int capacidadeRestante;
    vector<int> rota;
};

class CVRP
{
private:
    vector<Cliente> clientes;
    vector<Veiculo> veiculos;
    vector<vector<int>> custoRota; // Matriz de custos entre clientes e entre cliente e depósito
    int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo, custoTotal = 0;

    void debug(int veiculoAtual, int clienteAtual, int custoAdicionado, const string &tipoCusto)
    {
        cout << "DEBUG: " << tipoCusto << endl;
        cout << "Veículo " << veiculoAtual + 1 << ", Cliente " << clienteAtual + 1 << ": ";
        cout << "Custo Adicionado = " << custoAdicionado << ", ";
        cout << "Custo Total Cumulativo = " << custoTotal << endl;
    }

public:
    CVRP(const string &nomeArquivo)
    {
        // Leitura do arquivo de entrada
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open())
        {
            cerr << "Erro ao abrir o arquivo de entrada." << endl;
            exit(1);
        }

        arquivo >> totalClientes >> totalVeiculos >> capacidadeVeiculo >> entregasMinimas >> custoVeiculo;
        clientes.resize(totalClientes);
        for (int i = 0; i < totalClientes; i++)
        {
            arquivo >> clientes[i].demanda;
        }

        for (int i = 0; i < totalClientes; i++)
        {
            arquivo >> clientes[i].custoTerceirizacao;
        }

        custoRota.resize(totalClientes + 1, vector<int>(totalClientes + 1, 0));
        for (int i = 0; i <= totalClientes; i++)
        {
            for (int j = 0; j <= totalClientes; j++)
            {
                arquivo >> custoRota[i][j];
            }
        }

        arquivo.close();
        veiculos.resize(totalVeiculos, {capacidadeVeiculo});
    }

    void roteamentoVeiculos()
    {
        int entregasRealizadas = 0;
        // Itera sobre cada veículo enquanto o número mínimo de entregas não for atingido
        for (int v = 0; v < totalVeiculos; v++)
        {
            auto &veiculo = veiculos[v]; // Obtém a referência para o veículo atual
            veiculo.rota.push_back(0);   // Adiciona o depósito ao início da rota do veículo

            while (veiculo.capacidadeRestante > 0)
            {
                // Variáveis para armazenar o cliente mais próximo e o custo mínimo para chegar até ele
                int clienteMaisProximo = -1;
                int custoMinimo = numeric_limits<int>::max();

                // Itera sobre todos os clientes para encontrar o mais próximo
                for (int i = 0; i < totalClientes; i++)
                {
                    // Verifica se o cliente atual não foi atendido e se sua demanda pode ser atendida pelo veículo
                    if (!clientes[i].atendido && clientes[i].demanda <= veiculo.capacidadeRestante)
                    {
                        int ultimoCliente = veiculo.rota.back();     // Obtém o último cliente na rota do veículo
                        int custo = custoRota[ultimoCliente][i + 1]; // Calcula o custo para ir do último cliente até o cliente atual

                        // Se o custo calculado é menor que o custo mínimo atual, atualiza as variáveis
                        if (custo < custoMinimo)
                        {
                            clienteMaisProximo = i;
                            custoMinimo = custo;
                        }
                    }
                }

                if (clienteMaisProximo == -1)
                    break;

                // Marca o cliente como atendido, adiciona-o à rota, atualiza a capacidade do veículo e o custo total
                clientes[clienteMaisProximo].atendido = true;
                veiculo.rota.push_back(clienteMaisProximo + 1);
                veiculo.capacidadeRestante -= clientes[clienteMaisProximo].demanda;
                custoTotal += custoMinimo;
                entregasRealizadas++;
                debug(v, clienteMaisProximo, custoMinimo, "Custo do trajeto");
            }

            // Calcula o custo de retorno ao depósito e atualiza o custo total
            int ultimoCliente = veiculo.rota.back();
            int custoRetorno = custoRota[ultimoCliente][0];
            custoTotal += custoRetorno;
            debug(v, 0, custoRetorno, "Custo de retorno ao depósito");

            custoTotal += custoVeiculo; // Adiciona o custo fixo do veículo ao custo total
            debug(v, 0, custoVeiculo, "Custo fixo do veículo");
        }
    }

    void trocarClientes(int veiculo1, int cliente1, int veiculo2, int cliente2)
    {
        Veiculo &v1 = veiculos[veiculo1];
        Veiculo &v2 = veiculos[veiculo2];

        if (cliente1 >= 1 && cliente1 < v1.rota.size() - 1 && cliente2 >= 1 && cliente2 < v2.rota.size() - 1)
        {
            int cliente1_id = v1.rota[cliente1];
            int cliente2_id = v2.rota[cliente2];

            v1.capacidadeRestante += clientes[cliente1_id - 1].demanda - clientes[cliente2_id - 1].demanda;
            v2.capacidadeRestante += clientes[cliente2_id - 1].demanda - clientes[cliente1_id - 1].demanda;

            v1.rota[cliente1] = cliente2_id;
            v2.rota[cliente2] = cliente1_id;

            // Recalcula o custo total
            custoTotal = 0;
            for (int v = 0; v < totalVeiculos; v++)
            {
                Veiculo &veiculo = veiculos[v];
                custoTotal += custoRota[0][veiculo.rota[0]];
                for (int i = 0; i < veiculo.rota.size() - 1; i++)
                {
                    custoTotal += custoRota[veiculo.rota[i]][veiculo.rota[i + 1]];
                }
                custoTotal += custoRota[veiculo.rota.back()][0];
            }
        }
    }

    void VND()
    {
        int k = 1;
        while (k <= 2)
        { // Duas vizinhanças: troca de clientes e terceirização
            if (k == 1)
            {
                // Vizinhança 1: Troca de clientes entre veículos
                for (int v1 = 0; v1 < totalVeiculos; v1++)
                {
                    for (int c1 = 1; c1 < veiculos[v1].rota.size() - 1; c1++)
                    {
                        for (int v2 = 0; v2 < totalVeiculos; v2++)
                        {
                            for (int c2 = 1; c2 < veiculos[v2].rota.size() - 1; c2++)
                            {
                                if (v1 != v2)
                                {
                                    trocarClientes(v1, c1, v2, c2);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                // Vizinhança 2: Avaliação de terceirização
                avaliacaoTerceirizacao();
            }
            k++;
        }
    }

    void avaliacaoTerceirizacao()
    {
        // Itera sobre cada cliente para avaliar a terceirização
        for (int i = 0; i < totalClientes; i++)
        {
            // Se o cliente não foi atendido, verifica se é mais barato terceirizar
            if (!clientes[i].atendido)
            {

                // Calcula o custo de ida e volta do depósito para o cliente
                int custoIdaEVolta = custoRota[0][i + 1] + custoRota[i + 1][0];

                // Se o custo de terceirização é menor que o custo de ida e volta, marca o cliente como terceirizado
                if (clientes[i].custoTerceirizacao < custoIdaEVolta)
                {
                    clientes[i].terceirizado = true;
                    custoTotal += clientes[i].custoTerceirizacao;
                    debug(-1, i, clientes[i].custoTerceirizacao, "Custo de terceirização");
                }
            }
        }
    }

    void exibirResultados()
    {
        cout << "Custo total: " << custoTotal << endl;
        for (int i = 0; i < totalVeiculos; i++)
        {
            cout << "Rota do veiculo " << i + 1 << ": Depósito ";
            for (int cliente : veiculos[i].rota)
            {
                if (cliente == 0)
                    continue;
                cout << "-> Cliente " << cliente << " ";
            }
            if (veiculos[i].rota.size() == 1)
            {
                cout << "(veículo não utilizado)";
            }
            cout << "-> Depósito" << endl;
        }

        cout << "Clientes terceirizados: ";
        bool primeiro = true;
        for (int i = 0; i < totalClientes; i++)
        {
            if (clientes[i].terceirizado)
            {
                if (!primeiro)
                {
                    cout << ", ";
                }
                cout << "Cliente " << i + 1;
                primeiro = false;
            }
        }
        if (primeiro)
        {
            cout << "Nenhum cliente foi terceirizado.";
        }
        cout << endl;
    }
    int getCustoTotal()
    {
        return custoTotal;
    }
};

int main()
{
    ofstream arquivoCustosGuloso("custos_guloso.txt");
    ofstream arquivoCustosVND("custos_vnd.txt");

    string pasta = "instancias/";

    vector<string> listaInstancias =
        {
            "n9k5_A.txt",
            "n9k5_B.txt",
            "n9k5_C.txt",
            "n9k5_D.txt",
            "n14k5_A.txt",
            "n14k5_B.txt",
            "n14k5_C.txt",
            "n14k5_D.txt",
            "n22k3_A.txt",
            "n22k3_B.txt",
            "n22k3_C.txt",
            "n22k3_D.txt",
            "n31k5_A.txt",
            "n31k5_B.txt",
            "n31k5_C.txt",
            "n31k5_D.txt",
            "n43k6_A.txt",
            "n43k6_B.txt",
            "n43k6_C.txt",
            "n43k6_D.txt",
            "n64k9_A.txt",
            "n64k9_B.txt",
            "n64k9_C.txt",
            "n64k9_D.txt",
            "n120k7_A.txt",
            "n120k7_B.txt",
            "n120k7_C.txt",
            "n120k7_D.txt",
            "n199k17_A.txt",
            "n199k17_B.txt",
            "n199k17_C.txt",
            "n199k17_D.txt",
        };

    for (auto &instancia : listaInstancias)
    {
        CVRP problema(pasta + instancia);
        problema.roteamentoVeiculos();
        problema.avaliacaoTerceirizacao();
        problema.exibirResultados();
        arquivoCustosGuloso << problema.getCustoTotal() << endl;
        // APLICANDO VND
        problema.VND();
        cout << "----------" << endl;
        cout << "ROTEAMENTO APÓS O VND: " << endl;
        cout << "----------" << endl;
        problema.exibirResultados();
        arquivoCustosVND << problema.getCustoTotal() << endl;
    }
    arquivoCustosGuloso.close();
    arquivoCustosVND.close();
    return 0;
}
