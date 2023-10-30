#include <iostream>
#include <vector>
#include <vector>
#include <limits>
#include <fstream>
#include <chrono>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace std::chrono;

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
public:
    vector<Cliente> clientes;
    vector<Veiculo> veiculos;
    vector<vector<int>> custoRota; // Matriz de custos entre clientes e entre cliente e depósito
    int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas, custoVeiculo, custoTotal = 0;

    void debug(int veiculoAtual, int clienteAtual, int custoAdicionado,
               const string &tipoCusto)
    {
        cout << "DEBUG: " << tipoCusto << endl;
        cout << "Veículo " << veiculoAtual + 1 << ", Cliente " << clienteAtual + 1
             << ": ";
        cout << "Custo Adicionado = " << custoAdicionado << ", ";
        cout << "Custo Total Cumulativo = " << custoTotal << endl;
    }

    CVRP(const string &nomeArquivo)
    {
        // Leitura do arquivo de entrada
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open())
        {
            cerr << "Erro ao abrir o arquivo de entrada." << endl;
            exit(1);
        }

        arquivo >> totalClientes >> totalVeiculos >> capacidadeVeiculo >>
            entregasMinimas >> custoVeiculo;
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
        // Itera sobre cada veículo enquanto o número mínimo de entregas não for
        // atingido
        for (int v = 0; v < totalVeiculos && entregasRealizadas < entregasMinimas;
             v++)
        {
            auto &veiculo = veiculos[v]; // Obtém a referência para o veículo atual
            veiculo.rota.push_back(
                0); // Adiciona o depósito ao início da rota do veículo

            while (veiculo.capacidadeRestante > 0 &&
                   entregasRealizadas < entregasMinimas)
            {
                // Variáveis para armazenar o cliente mais próximo e o custo mínimo para
                // chegar até ele
                int clienteMaisProximo = -1;
                int custoMinimo = numeric_limits<int>::max();

                // Itera sobre todos os clientes para encontrar o mais próximo
                for (int i = 0; i < totalClientes; i++)
                {
                    // Verifica se o cliente atual não foi atendido e se sua demanda pode
                    // ser atendida pelo veículo
                    if (!clientes[i].atendido &&
                        clientes[i].demanda <= veiculo.capacidadeRestante)
                    {
                        int ultimoCliente =
                            veiculo.rota
                                .back(); // Obtém o último cliente na rota do veículo
                        int custo = custoRota[ultimoCliente]
                                             [i + 1]; // Calcula o custo para ir do último
                                                      // cliente até o cliente atual

                        // Se o custo calculado é menor que o custo mínimo atual, atualiza
                        // as variáveis
                        if (custo < custoMinimo)
                        {
                            clienteMaisProximo = i;
                            custoMinimo = custo;
                        }
                    }
                }

                if (clienteMaisProximo == -1)
                    break;

                // Marca o cliente como atendido, adiciona-o à rota, atualiza a
                // capacidade do veículo e o custo total
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

            custoTotal +=
                custoVeiculo; // Adiciona o custo fixo do veículo ao custo total
            debug(v, 0, custoVeiculo, "Custo fixo do veículo");
        }
    }

    int avaliacaoTerceirizacao()
    {
        for (int i = 0; i < totalClientes; i++)
        {
            if (!clientes[i].atendido &&
                !clientes[i].terceirizado) // Apenas clientes não atendidos e não
                                           // terceirizados
            {
                int custoIdaEVolta = custoRota[0][i + 1] + custoRota[i + 1][0];
                if (clientes[i].custoTerceirizacao < custoIdaEVolta)
                {
                    clientes[i].terceirizado = true;
                    clientes[i].atendido = true;
                    clientes[i].atendido = true;
                    custoTotal += clientes[i].custoTerceirizacao;
                    debug(-1, i, clientes[i].custoTerceirizacao,
                          "Custo de terceirização");
                }
            }
        }
        return custoTotal;
    }

    int exibirResultados()
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
        return custoTotal;
    }

    int calcularCustoRota(const vector<int> &rota)
    {
        int custo = 0;
        for (int i = 0; i < rota.size() - 1; i++)
        {
            custo += custoRota[rota[i]][rota[i + 1]];
        }
        return custo;
    }

    int CalculoTotalTercerizacao()
    {
        int custoTotalSemTerceirizacao = 0l;
        for (Veiculo &veiculo : veiculos)
        {
            if (!veiculo.rota.empty())
            {
                custoTotalSemTerceirizacao += calcularCustoRota(veiculo.rota);
                custoTotalSemTerceirizacao += custoVeiculo;
            }
        }
        for (const Cliente &cliente : clientes)
        {
            if (cliente.terceirizado)
            {
                custoTotalSemTerceirizacao += cliente.custoTerceirizacao;
            }
        }
        return custoTotalSemTerceirizacao;
    }

    void RotaUnica()
    {
        for (int veiculoIdx = 0; veiculoIdx < totalVeiculos; veiculoIdx++)
        {
            Veiculo &veiculo = veiculos[veiculoIdx];
            vector<int> &rota = veiculo.rota;

            for (int i = 1; i < rota.size() - 1; i++)
            {
                for (int j = i + 1; j < rota.size(); j++)
                {
                    swap(rota[i], rota[j]);

                    // Avalie o custo da nova rota após a troca
                    int novoCusto = calcularCustoRota(rota);

                    int custoTotal_Antigo = custoTotal;
                    int custoTotal_Novo = CalculoTotalTercerizacao();
                    if (custoTotal_Antigo > custoTotal_Novo)
                    {
                        custoTotal = custoTotal_Novo;
                    }
                }
            }
        }
    }

    void RotasMultiplas()
    {
        vector<Veiculo> melhorSolucao = veiculos;

        for (int veiculo1 = 0; veiculo1 < totalVeiculos; veiculo1++)
        {
            for (int veiculo2 = veiculo1 + 1; veiculo2 < totalVeiculos; veiculo2++)
            {
                Veiculo &v1 = veiculos[veiculo1];
                Veiculo &v2 = veiculos[veiculo2];

                for (int i = 1; i < v1.rota.size(); i++)
                {
                    for (int j = 1; j < v2.rota.size(); j++)
                    {
                        // Realize a troca
                        swap(v1.rota[i], v2.rota[j]);

                        // Verifique a capacidade dos veículos após a troca
                        int capacidadeV1 = capacidadeVeiculo;
                        int capacidadeV2 = capacidadeVeiculo;

                        for (int k = 1; k < v1.rota.size(); k++)
                        {
                            capacidadeV1 -= clientes[v1.rota[k] - 1].demanda;
                        }

                        for (int k = 1; k < v2.rota.size(); k++)
                        {
                            capacidadeV2 -= clientes[v2.rota[k] - 1].demanda;
                        }

                        int custoTotal_Antigo = custoTotal;
                        // Calcule o custo total depois da troca
                        int custoNovo = CalculoTotalTercerizacao();

                        // Verifique se houve melhoria no custo total
                        if (custoNovo < custoTotal_Antigo)
                        {
                            custoTotal = custoNovo;
                        }
                    }
                }
            }
        }
    }

    void VND()
    {
        int custoAtual = custoTotal;
        int custoNovo;

        do
        {
            custoNovo = custoTotal;

            for (Veiculo &veiculo : veiculos)
            {
                twoOpt(veiculo);
            }

            avaliacaoTerceirizacao();

        } while (custoTotal < custoNovo);

        if (custoTotal < custoAtual)
        {
            cout << "Melhoria com VND. Custo novo: " << custoTotal << endl;
        }
        else
        {
            cout << "Sem melhoria com VND. Custo: " << custoTotal << endl;
        }
    }

    void twoOpt(Veiculo &veiculo)
    {
        vector<int> &rota = veiculo.rota;
        int n = rota.size();

        for (int i = 1; i < n - 2; i++)
        {
            for (int j = i + 1; j < n - 1; j++)
            {
                vector<int> novaRota = rota;
                troca(novaRota, i, j);

                if (isTrocaValida(veiculo, novaRota, i, j))
                {
                    int custoNovo = CalculoTotalTercerizacao();

                    if (custoNovo < custoTotal)
                    {
                        rota = novaRota;
                        custoTotal = custoNovo;
                    }
                }
            }
        }
    }

    // Verifica se a troca respeita a capacidade dos veículos
    bool isTrocaValida(Veiculo &veiculo, vector<int> &novaRota, int i, int j)
    {
        int capacidadeRestante = veiculo.capacidadeRestante;

        for (int k = i + 1; k < j; k++)
        {
            int cliente = novaRota[k] - 1;
            capacidadeRestante -= clientes[cliente].demanda;
            if (capacidadeRestante < 0)
            {
                return false;
            }
        }

        return true;
    }

    void troca(vector<int> &rota, int i, int j)
    {
        while (i < j)
        {
            swap(rota[i], rota[j]);
            i++;
            j--;
        }
    }

    void gerarArquivoSaida(const string &nomeArquivo)
    {
        ofstream arquivoSaida(nomeArquivo);
        if (!arquivoSaida.is_open())
        {
            cerr << "Erro ao abrir o arquivo de saída." << endl;
            return;
        }

        int custoRoteamento = 0;
        int custoVeiculos = 0;
        int custoTerceirizacao = 0;

        for (const auto &veiculo : veiculos)
        {
            if (veiculo.rota.size() > 1)
            {
                custoVeiculos += custoVeiculo;
                for (size_t i = 0; i < veiculo.rota.size() - 1; ++i)
                {
                    custoRoteamento += custoRota[veiculo.rota[i]][veiculo.rota[i + 1]];
                }
                custoRoteamento += custoRota[veiculo.rota.back()][0];
            }
        }

        for (const auto &cliente : clientes)
        {
            if (cliente.terceirizado)
            {
                custoTerceirizacao += cliente.custoTerceirizacao;
            }
        }

        arquivoSaida << custoTotal << endl
                     << endl;
        arquivoSaida << custoRoteamento << endl
                     << endl;
        arquivoSaida << custoVeiculos << endl
                     << endl;
        arquivoSaida << custoTerceirizacao << endl
                     << endl;

        bool primeiro = true;
        for (const auto &cliente : clientes)
        {
            if (cliente.terceirizado)
            {
                if (!primeiro)
                {
                    arquivoSaida << " ";
                }
                arquivoSaida << (&cliente - &clientes[0] + 1);
                primeiro = false;
            }
        }
        arquivoSaida << endl
                     << endl;

        arquivoSaida << veiculos.size() << endl
                     << endl;
        for (const auto &veiculo : veiculos)
        {
            if (veiculo.rota.size() > 1)
            {
                for (int cliente : veiculo.rota)
                {
                    arquivoSaida << cliente << " ";
                }
                arquivoSaida << endl;
            }
            else
            {
                arquivoSaida << "0 " << endl;
            }
        }

        arquivoSaida.close();
    }
};

int main()
{
    ofstream arquivoCustosGuloso("custos/custos_guloso.txt");
    ofstream arquivoCustosVND("custos/custos_vnd.txt");
    ofstream arquivoTempoExecGuloso("tempos_de_execucao/tempo_exec_guloso.txt");
    ofstream arquivoTempoExecVND("tempos_de_execucao/tempo_exec_vnd.txt");

    string pasta_instancias = "instancias/";

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
        CVRP problema(pasta_instancias + instancia);

        // APLICANDO ALGORITMO GULOSO
        auto inicio_guloso = high_resolution_clock::now();
        problema.roteamentoVeiculos();
        auto fim_guloso = high_resolution_clock::now();

        problema.avaliacaoTerceirizacao();
        problema.exibirResultados();

        string stringsaida = "guloso_saida_" + instancia;
        problema.gerarArquivoSaida("arquivos_saidas/" + stringsaida + ".txt");

        arquivoCustosGuloso << problema.custoTotal << endl;

        // APLICANDO VND
        auto inicio_vnd = high_resolution_clock::now();
        problema.VND();
        auto fim_vnd = high_resolution_clock::now();
        stringsaida = "vnd_saida_" + instancia;
        problema.gerarArquivoSaida("arquivos_saidas/" + stringsaida + ".txt");

        cout << "----------" << endl
             << "ROTEAMENTO APÓS O VND: " << endl
             << "----------" << endl;

        problema.exibirResultados();
        arquivoCustosVND << problema.custoTotal << endl;

        auto tempo_exec_guloso = duration_cast<microseconds>(fim_guloso - inicio_guloso);
        auto tempo_exec_vnd = duration_cast<microseconds>(fim_vnd - inicio_vnd);

        // Escreve os tempos de execução para cada instancia
        arquivoTempoExecGuloso << tempo_exec_guloso.count() << endl;
        arquivoTempoExecVND << tempo_exec_vnd.count() << endl;
    }

    arquivoCustosGuloso.close();
    arquivoCustosVND.close();
    return 0;
}