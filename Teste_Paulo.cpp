#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

struct Cliente {
  int demanda;               // Demanda de cada cliente
  int custoTerceirizacao;    // Custo para terceirizar o pedido do cliente
  bool atendido = false;     // Indica se o cliente foi atendido
  bool terceirizado = false; // Indica se o cliente foi terceirizado
};

struct Veiculo {
  int capacidadeRestante;
  vector<int> rota;
};

class CVRP {
public:
  vector<Cliente> clientes;
  vector<Veiculo> veiculos;
  vector<vector<int>> custoRota; // Matriz de custos entre clientes e entre cliente e depósito
  int totalClientes, totalVeiculos, capacidadeVeiculo, entregasMinimas,
      custoVeiculo, custoTotal = 0;

  void debug(int veiculoAtual, int clienteAtual, int custoAdicionado,
             const string &tipoCusto) {
    cout << "DEBUG: " << tipoCusto << endl;
    cout << "Veículo " << veiculoAtual + 1 << ", Cliente " << clienteAtual + 1
         << ": ";
    cout << "Custo Adicionado = " << custoAdicionado << ", ";
    cout << "Custo Total Cumulativo = " << custoTotal << endl;
  }

  CVRP(const string &nomeArquivo) {
    // Leitura do arquivo de entrada
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
      cerr << "Erro ao abrir o arquivo de entrada." << endl;
      exit(1);
    }

    arquivo >> totalClientes >> totalVeiculos >> capacidadeVeiculo >>
        entregasMinimas >> custoVeiculo;
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
    veiculos.resize(totalVeiculos, {capacidadeVeiculo});
  }

  void roteamentoVeiculos() {
    int entregasRealizadas = 0;
    // Itera sobre cada veículo enquanto o número mínimo de entregas não for
    // atingido
    for (int v = 0; v < totalVeiculos && entregasRealizadas < entregasMinimas;
         v++) {
      auto &veiculo = veiculos[v]; // Obtém a referência para o veículo atual
      veiculo.rota.push_back(
          0); // Adiciona o depósito ao início da rota do veículo

      while (veiculo.capacidadeRestante > 0 &&
             entregasRealizadas < entregasMinimas) {
        // Variáveis para armazenar o cliente mais próximo e o custo mínimo para
        // chegar até ele
        int clienteMaisProximo = -1;
        int custoMinimo = numeric_limits<int>::max();

        // Itera sobre todos os clientes para encontrar o mais próximo
        for (int i = 0; i < totalClientes; i++) {
          // Verifica se o cliente atual não foi atendido e se sua demanda pode
          // ser atendida pelo veículo
          if (!clientes[i].atendido &&
              clientes[i].demanda <= veiculo.capacidadeRestante) {
            int ultimoCliente =
                veiculo.rota
                    .back(); // Obtém o último cliente na rota do veículo
            int custo = custoRota[ultimoCliente]
                                 [i + 1]; // Calcula o custo para ir do último
                                          // cliente até o cliente atual

            // Se o custo calculado é menor que o custo mínimo atual, atualiza
            // as variáveis
            if (custo < custoMinimo) {
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

  void avaliacaoTerceirizacao()
  {
      for (int i = 0; i < totalClientes; i++)
      {
          if (!clientes[i].atendido && !clientes[i].terceirizado) // Apenas clientes não atendidos e não terceirizados
          {
              int custoIdaEVolta = custoRota[0][i + 1] + custoRota[i + 1][0];
              if (clientes[i].custoTerceirizacao < custoIdaEVolta)
              {
                  clientes[i].terceirizado = true;
                  clientes[i].atendido = true;
                  custoTotal += clientes[i].custoTerceirizacao;
                  debug(-1, i, clientes[i].custoTerceirizacao, "Custo de terceirização");
              }
          }
      }
  }

  int exibirResultados() {
      cout << "Custo total: " << custoTotal << endl;
      for (int i = 0; i < totalVeiculos; i++) {
          cout << "Rota do veiculo " << i + 1 << ": Depósito ";
          for (int cliente : veiculos[i].rota) {
              if (cliente == 0)
                  continue;
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
      cout << "custo total no exibir " << custoTotal << endl;
      return custoTotal;
  }

  int calcularCustoRota(const vector<int> &rota) {
      int custo = 0;
      for (int i = 0; i < rota.size() - 1; i++) {
          custo += custoRota[rota[i]][rota[i + 1]];
      }
      return custo;
  }
  void vizinhancaRotaUnica() {
    for (int veiculoIdx = 0; veiculoIdx < totalVeiculos; veiculoIdx++) {
        Veiculo &veiculo = veiculos[veiculoIdx];
        vector<int> &rota = veiculo.rota;
        int custoAntigo = calcularCustoRota(rota); // Custo da rota atual

        for (int i = 1; i < rota.size() - 1; i++) {
            for (int j = i + 1; j < rota.size(); j++) {
                swap(rota[i], rota[j]);

                // Avalie o custo da nova rota após a troca
                int novoCusto = calcularCustoRota(rota);

                // Se o novo custo for menor, mantenha a troca; caso contrário, desfaça a troca
                if (novoCusto < custoAntigo) {
                    custoAntigo = novoCusto; // Atualiza o custo antigo
                } else {
                    swap(rota[i], rota[j]);
                }
            }
        }
    }
  }

void buscaVizinhancaMultiplaRotas() {
          int custoOriginal = custoTotal; // Custo original antes da busca na vizinhança
  
          for (int veiculo1 = 0; veiculo1 < totalVeiculos; veiculo1++) {
              for (int veiculo2 = veiculo1 + 1; veiculo2 < totalVeiculos; veiculo2++) {
                  Veiculo& v1 = veiculos[veiculo1];
                  Veiculo& v2 = veiculos[veiculo2];
  
                  for (int i = 1; i < v1.rota.size(); i++) {
                      for (int j = 1; j < v2.rota.size(); j++) {
                          // Realiza a troca dos clientes entre as rotas v1 e v2
                          int cliente1 = v1.rota[i];
                          int cliente2 = v2.rota[j];
                          int custoAntes = custoRota[v1.rota[i - 1]][cliente1] + custoRota[cliente1][v1.rota[i + 1]] +
                              custoRota[v2.rota[j - 1]][cliente2] + custoRota[cliente2][v2.rota[j + 1]];
                          int custoDepois = custoRota[v1.rota[i - 1]][cliente2] + custoRota[cliente2][v1.rota[i + 1]] +
                              custoRota[v2.rota[j - 1]][cliente1] + custoRota[cliente1][v2.rota[j + 1]];
  
                          // Calcula o custo total depois da troca
                          int custoNovo = custoTotal - custoAntes + custoDepois;
  
                          if (custoNovo < custoTotal) {
                              // Realiza a troca se o custo diminuir
                              swap(v1.rota[i], v2.rota[j]);
                              custoTotal = custoNovo;
                          }
                      }
                  }
              }
          }
        cout << "Cheguei até aqui" << endl;
  
          // Verifica se houve melhoria no custo total
          if (custoTotal < custoOriginal) {
             cout << "entro aqui" << endl;
              buscaVizinhancaMultiplaRotas();
          }
      }
};

int main() {
    CVRP cvrp("Entrada.txt");

    cvrp.roteamentoVeiculos();
    cvrp.avaliacaoTerceirizacao();

    cout << "Custo total antex o roteamento inicial: " << cvrp.exibirResultados() << endl;
    cout << "-----------------------------------------" << endl;
    cvrp.vizinhancaRotaUnica();
  
    cout << "Custo total após a otimização com a função de troca: " << cvrp.exibirResultados() << endl;

    return 0;
}