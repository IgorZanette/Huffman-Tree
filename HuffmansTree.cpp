#include <iostream>
#include <vector>
#include <fstream>
#include <locale>
#include <codecvt>
#include <unordered_map>
#include <algorithm>

using namespace std;

struct node {
    wchar_t chave;
    int freq;
};

// Função de comparação para ordenar os nodes pela frequência
bool compareNodes(const node& a, const node& b) {
    return a.freq < b.freq;
}

int main() {
    // Abrir o arquivo em modo de entrada
    wifstream arq("../clarissa.txt");
    if (!arq) {
        cerr << "Erro ao abrir o arquivo...\n";
        return 1;
    }

    // Converter a entrada de UTF-8 para wide string (wchar_t)
    arq.imbue(locale(arq.getloc(), new codecvt_utf8<wchar_t>));

    wchar_t ch;
    unordered_map<wchar_t, int> freq_map;

    // Ler caracteres Unicode do arquivo, ignorando linhas em branco
    while (arq.get(ch)) {
        if (ch != L' ' && ch != L'\n' && ch != L'\r' && ch != L'\t') {
            freq_map[ch]++;
        }
    }

    arq.close();

    // Converter o mapa para um vetor de nodes
    vector<node> vetor;
    for (const auto& pair : freq_map) {
        node novo;
        novo.chave = pair.first;
        novo.freq = pair.second;
        vetor.push_back(novo);
    }

    // Ordenar o vetor pela frequência (menor para maior)
    sort(vetor.begin(), vetor.end(), compareNodes);

    // Exibir o vetor na ordem inversa (do fim para o início), ignorando caracteres vazios
    for (auto it = vetor.rbegin(); it != vetor.rend(); ++it) {
        node n = *it;
        if (n.chave != L'\0') { // Verifica se o caractere não é nulo
            wcout << L"Caractere: " << n.chave << L" Frequência: " << n.freq << endl;
        }
    }

    return 0;
}
