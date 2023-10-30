#include <iostream>
#include <vector>
#include <limits>
#include <fstream>
#include <chrono>

using namespace std;
using namespace std::chrono;
const string CUSTOS_GULOSO = "custos/custos_guloso.txt";
const string CUSTOS_VND = "custos/custos_vnd.txt";
const string TEMPO_EXEC_GULOSO = "tempos_de_execucao/tempo_exec_guloso.txt";
const string TEMPO_EXEC_VND = "tempos_de_execucao/tempo_exec_vnd.txt";
const string PASTA_INSTANCIAS = "instancias/";
const string PASTA_SAIDAS = "arquivos_saidas/";

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
    vector<vector<int>> custoRota; 
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

    CVRP(const string &nomeArquivo){
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

    void roteamentoVeiculos(){
        int entregasRealizadas = 0;

        for (int v = 0; v < totalVeiculos && entregasRealizadas < entregasMinimas;
             v++)
        {
            auto &veiculo = veiculos[v]; 
            veiculo.rota.push_back(
                0); 

            while (veiculo.capacidadeRestante > 0 &&
                   entregasRealizadas < entregasMinimas)
            {

                int clienteMaisProximo = -1;
                int custoMinimo = numeric_limits<int>::max();

                for (int i = 0; i < totalClientes; i++)
                {

                    if (!clientes[i].atendido &&
                        clientes[i].demanda <= veiculo.capacidadeRestante)
                    {
                        int ultimoCliente =
                            veiculo.rota
                                .back(); 
                        int custo = custoRota[ultimoCliente] [i + 1]; 

                        if (custo < custoMinimo)
                        {
                            clienteMaisProximo = i;
                            custoMinimo = custo;
                        }
                    }
                }

                if (clienteMaisProximo == -1)
                    break;

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

    int avaliacaoTerceirizacao(){
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

                    custoTotal += clientes[i].custoTerceirizacao;
                    debug(-1, i, clientes[i].custoTerceirizacao,
                          "Custo de terceirização");
                }
            }
        }
        return custoTotal;
    }

    int exibirResultados(){
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

    int calcularCustoRota(const vector<int> &rota){
        int custo = 0;
        for (int i = 0; i < rota.size() - 1; i++)
        {
            custo += custoRota[rota[i]][rota[i + 1]];
        }
        return custo;
    }

    int CalculoTotalTercerizacao(){
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

    bool isTrocaValida(Veiculo &veiculo, vector<int> &novaRota, int i, int j){
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

    void troca(vector<int> &rota, int i, int j){
        while (i < j)
        {
            swap(rota[i], rota[j]);
            i++;
            j--;
        }
    }

    void RotaUnica() {
        for (int veiculoIdx = 0; veiculoIdx < totalVeiculos; veiculoIdx++) {
            Veiculo &veiculo = veiculos[veiculoIdx];
            vector<int> &rota = veiculo.rota;
    
            for (int i = 1; i < rota.size() - 1; i++) {
                for (int j = i + 1; j < rota.size(); j++) {
                    swap(rota[i], rota[j]);
    
                    if (isTrocaValida(veiculo, rota, i, j)) {
                        int custoTotal_Antigo = custoTotal;
                        int custoTotal_Novo = CalculoTotalTercerizacao();
                        if (custoTotal_Antigo > custoTotal_Novo) {
                            custoTotal = custoTotal_Novo;
                        } else {
                            swap(rota[i], rota[j]);
                        }
                    } else {
                        swap(rota[i], rota[j]);
                    }
                }
            }
        }
    }
    
    void RotasMultiplas() {
        vector<Veiculo> melhorSolucao = veiculos;
    
        for (int veiculo1 = 0; veiculo1 < totalVeiculos; veiculo1++) {
            for (int veiculo2 = veiculo1 + 1; veiculo2 < totalVeiculos; veiculo2++) {
                Veiculo &v1 = veiculos[veiculo1];
                Veiculo &v2 = veiculos[veiculo2];
    
                for (int i = 1; i < v1.rota.size(); i++) {
                    for (int j = 1; j < v2.rota.size(); j++) {
                        swap(v1.rota[i], v2.rota[j]);
    
                        if (isTrocaValida(v1, v1.rota, i, v1.rota.size()) &&
                            isTrocaValida(v2, v2.rota, j, v2.rota.size())) {
                            int custoTotal_Antigo = custoTotal;
                            int custoTotal_Novo = CalculoTotalTercerizacao();
    
                            if (custoTotal_Antigo > custoTotal_Novo) {
                                custoTotal = custoTotal_Novo;
                            } else {
                                swap(v1.rota[i], v2.rota[j]);
                            }
                        } else {
                            swap(v1.rota[i], v2.rota[j]);
                        }
                    }
                }
            }
        }
    }

    void twoOpt(Veiculo &veiculo){
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

    void VND(){
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

    void VND_Unica_Multipla() {
      int v = 2;
      int k = 1;
      int custoAntesVND = custoTotal;
      int custoDepoisVND = custoTotal;
  
      do {
          custoAntesVND = custoDepoisVND; 
  
          switch (k) {
              case 1:
                  RotaUnica();
                  break;
              case 2:
                  RotasMultiplas();
                  break;
              default:
                  break;
          }
  
          custoDepoisVND = custoTotal;
  
          if (custoDepoisVND < custoAntesVND) {
              k = 1; 
          } else {
              k++;
          }
      } while (k <= v);
  
      if (custoDepoisVND < custoAntesVND) {
          cout << "Melhoria com VND com Rota Unica e Multipla. Custo novo: " << custoTotal << endl;
      } else {
          cout << "Sem melhoria com VND com Rota Unica e Multipla. Custo: " << custoTotal << endl;
      }
  }

    void gerarArquivoSaida(const string &nomeArquivo){
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

void aplicarAlgoritmoGuloso(CVRP &problema, ofstream &arquivoCustos, ofstream &arquivoTempoExec) {
    auto inicio = high_resolution_clock::now();
    problema.roteamentoVeiculos();
    auto fim = high_resolution_clock::now();
    problema.avaliacaoTerceirizacao();
    problema.exibirResultados();
    arquivoCustos << problema.custoTotal << endl;
    auto tempo_exec = duration_cast<microseconds>(fim - inicio);
    arquivoTempoExec << tempo_exec.count() << endl;
}

void aplicarVND(CVRP &problema, ofstream &arquivoCustos, ofstream &arquivoTempoExec) {
    auto inicio = high_resolution_clock::now();
    problema.VND();
    auto fim = high_resolution_clock::now();

    cout << "----------" << endl
         << "ROTEAMENTO APÓS O VND: " << endl
         << "----------" << endl;

    problema.exibirResultados();
    arquivoCustos << problema.custoTotal << endl;

    auto tempo_exec = duration_cast<microseconds>(fim - inicio);
    arquivoTempoExec << tempo_exec.count() << endl;
}

int main(){
    ofstream arquivoCustosGuloso(CUSTOS_GULOSO);
    ofstream arquivoCustosVND(CUSTOS_VND);
    ofstream arquivoTempoExecGuloso(TEMPO_EXEC_GULOSO);
    ofstream arquivoTempoExecVND(TEMPO_EXEC_VND);



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

    for (auto &instancia : listaInstancias) {
        CVRP problema(PASTA_INSTANCIAS + instancia);

        aplicarAlgoritmoGuloso(problema, arquivoCustosGuloso, arquivoTempoExecGuloso);

        string stringsaidaGuloso = "guloso_saida_" + instancia;
        problema.gerarArquivoSaida(PASTA_SAIDAS + stringsaidaGuloso + ".txt");

        aplicarVND(problema, arquivoCustosVND, arquivoTempoExecVND);

        string stringsaidaVND = "vnd_saida_" + instancia;
        problema.gerarArquivoSaida(PASTA_SAIDAS + stringsaidaVND + ".txt");
    }

    return 0;
}
