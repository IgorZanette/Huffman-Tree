#include <iostream>
#include <vector>
#include <fstream>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <queue>
#include <cwctype>
#include <unordered_map>
#include <functional> // Adicionando inclusão para std::function

using namespace std;

struct node {
    wchar_t chave;
    int freq;
    node* left;
    node* right;

    // Construtor para facilitar a criação de nós
    node(wchar_t c, int f, node* l = nullptr, node* r = nullptr) : chave(c), freq(f), left(l), right(r) {}
};

// Função de comparação para ordenar os nós pela frequência
bool compareNodes(const node& a, const node& b) {
    return a.freq > b.freq; // Alterado para ordenar do maior para o menor
}

// Função para construir a árvore de Huffman
node* buildHuffmanTree(const unordered_map<wchar_t, int>& freq_map) {
    // Utilizando std::function para definir o tipo de função lambda
    priority_queue<node*, vector<node*>, function<bool(node*, node*)>> pq([](node* a, node* b) { return a->freq > b->freq; });

    // Adicionar todos os nós à fila de prioridade
    for (const auto& pair : freq_map) {
        pq.push(new node(pair.first, pair.second));
    }

    // Construir a árvore combinando os nós de menor frequência
    while (pq.size() > 1) {
        node* left = pq.top();
        pq.pop();
        node* right = pq.top();
        pq.pop();

        // Criar um novo nó com a soma das frequências
        node* combined = new node(L'\0', left->freq + right->freq, left, right);

        // Adicionar o novo nó à fila de prioridade
        pq.push(combined);
    }

    // O último nó restante na fila de prioridade é a raiz da árvore de Huffman
    return pq.top();
}

// Função para exibir a árvore de Huffman
void printHuffmanTree(node* root, wstring prefix = L"") {
    if (root == nullptr) return;

    if (root->chave != L'\0') {
        wcout << L"Caractere: " << root->chave << L" Frequência: " << root->freq << L" Prefixo: " << prefix << endl;
    }

    printHuffmanTree(root->left, prefix + L"0");
    printHuffmanTree(root->right, prefix + L"1");
}

int main() {
    // Configura a saída para UTF-8
    locale utf8_locale(locale(), new codecvt_utf8<wchar_t>);
    wcout.imbue(utf8_locale);

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
        if (iswprint(ch)) {
            freq_map[ch]++;
        }
    }

    arq.close();

    // Construir a árvore de Huffman
    node* root = buildHuffmanTree(freq_map);

    // Exibir a árvore de Huffman
    wcout << L"Arvore de Huffman:" << endl;
    printHuffmanTree(root);

    return 0;
}