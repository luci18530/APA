#include <iostream>
#include <fstream>
#include <vector>

int main() {
    // Declaração de variáveis
    int n, k, Q, L, r;
    std::vector<int> d, p;
    std::vector<std::vector<int>> c;

    // Abrindo o arquivo para leitura
    std::ifstream infile("entrada.txt");
    if (!infile.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }

    // Leitura das informações básicas
    infile >> n >> k >> Q >> L >> r;

    // Leitura do array d
    d.resize(n);
    for (int i = 0; i < n; i++) {
        infile >> d[i];
    }

    // Leitura do array p
    p.resize(n);
    for (int i = 0; i < n; i++) {
        infile >> p[i];
    }

    // Leitura da matriz c
    c.resize(n+1, std::vector<int>(n+1));
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) {
            infile >> c[i][j];
        }
    }

    infile.close();

    // lógica da busca local ou meta-heurística.

    return 0;
}
