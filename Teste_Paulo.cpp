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
private:
  vector<Cliente> clientes;
  vector<Veiculo> veiculos;
  vector<vector<int>>
      custoRota; // Matriz de custos entre clientes e entre cliente e depósito
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

public:
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

  void avaliacaoTerceirizacao() {
    // Itera sobre cada cliente para avaliar a terceirização
    for (int i = 0; i < totalClientes; i++) {
      // Se o cliente não foi atendido, verifica se é mais barato terceirizar
      if (!clientes[i].atendido) {

        // Calcula o custo de ida e volta do depósito para o cliente
        int custoIdaEVolta = custoRota[0][i + 1] + custoRota[i + 1][0];

        // Se o custo de terceirização é menor que o custo de ida e volta, marca
        // o cliente como terceirizado
        if (clientes[i].custoTerceirizacao < custoIdaEVolta) {
          clientes[i].terceirizado = true;
          custoTotal += clientes[i].custoTerceirizacao;
          debug(-1, i, clientes[i].custoTerceirizacao,
                "Custo de terceirização");
        }
      }
    }
  }

  int CalcularCustoTotal() {
    int custo = 0;

    // Calcular o custo total das rotas dos veículos
    for (int v = 0; v < totalVeiculos; v++) {
      const vector<int> &rota = veiculos[v].rota;
      for (int i = 1; i < rota.size(); i++) {
        int cliente1 = rota[i - 1];
        int cliente2 = rota[i];
        custo += custoRota[cliente1][cliente2];
      }
    }

    return custo;
  }

  void Rota_unica(int veiculoIndex) {
    Veiculo veiculo = veiculos[veiculoIndex];
    vector<int> melhorRota = veiculo.rota;
    int custoMelhorRota = CalcularCustoTotal();

    for (int i = 1; i < veiculo.rota.size() - 1; i++) {
      for (int j = i + 1; j < veiculo.rota.size(); j++) {
        vector<int> novaRota = veiculo.rota;
        swap(novaRota[i], novaRota[j]);
        veiculos[veiculoIndex].rota = novaRota;

        int custoNovaRota = CalcularCustoTotal();

        if (custoNovaRota < custoMelhorRota) {
          melhorRota = novaRota;
          custoMelhorRota = custoNovaRota;
        }
      }
    }
    veiculos[veiculoIndex].rota = melhorRota;
  }

  void rotas_multiplas() {
    int custoMelhorSolucao = CalcularCustoTotal();
    vector<vector<int>> melhorRotaVeiculos(totalVeiculos);

    for (int rota1 = 0; rota1 < totalVeiculos; rota1++) {
      for (int rota2 = rota1 + 1; rota2 < totalVeiculos; rota2++) {
        for (int i = 1; i < veiculos[rota1].rota.size() - 1; i++) {
          for (int j = 1; j < veiculos[rota2].rota.size() - 1; j++) {
            vector<int> novaRota1 = veiculos[rota1].rota;
            vector<int> novaRota2 = veiculos[rota2].rota;
            swap(novaRota1[i], novaRota2[j]);

            veiculos[rota1].rota = novaRota1;
            veiculos[rota2].rota = novaRota2;

            int custoNovaSolucao = CalcularCustoTotal();

            if (custoNovaSolucao < custoMelhorSolucao) {
              custoMelhorSolucao = custoNovaSolucao;
              melhorRotaVeiculos[rota1] = novaRota1;
              melhorRotaVeiculos[rota2] = novaRota2;
            }

            veiculos[rota1].rota = novaRota1;
            veiculos[rota2].rota = novaRota2;
          }
        }
      }
    }

    for (int i = 0; i < totalVeiculos; i++) {
      veiculos[i].rota = melhorRotaVeiculos[i];
    }
  }

  void exibirResultados() {
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
  }
};

int main() {
  CVRP problema("Entrada.txt");
  problema.roteamentoVeiculos();     // Roteamento inicial
  problema.avaliacaoTerceirizacao(); // Avaliação de terceirização
  problema.exibirResultados();
  cout << "--------------------------------------------------------------------"
          "------------";
  problema.Rota_unica(0); // Chamando Rota_unica para o primeiro veículo (índice 0)
  problema.rotas_multiplas();
  problema.avaliacaoTerceirizacao(); // Chamando rotas_multiplas
  problema.exibirResultados();       // Exibindo os resultados
  return 0;
}