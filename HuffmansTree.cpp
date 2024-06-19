#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>

using namespace std;

struct No {
    unsigned char dado;
    int frequencia;
    No *esquerda, *direita;
    string codigo; // Novo campo para armazenar o código Huffman
};

No* criarNo(unsigned char dado, int frequencia) {
    No* novoNo = new No();
    novoNo->dado = dado;
    novoNo->frequencia = frequencia;
    novoNo->esquerda = novoNo->direita = nullptr;
    return novoNo;
}

struct comparar {
    bool operator()(No* l, No* r) {
        return (l->frequencia > r->frequencia);
    }
};

bool ehCaractereAceitavel(unsigned char c) {
    return (c >= 32 && c <= 126) || (c >= 128 && c <= 255);
}

void codificar(No* raiz, string str, unordered_map<unsigned char, string> &codigoHuffman) {
    if (raiz == nullptr)
        return;
    
    if (!raiz->esquerda && !raiz->direita) {
        codigoHuffman[raiz->dado] = str;
    }

    raiz->codigo = str; // Armazenar o código Huffman no nó atual
    
    codificar(raiz->esquerda, str + "0", codigoHuffman);
    codificar(raiz->direita, str + "1", codigoHuffman);
}

void escreverCompactado(const string& texto, const unordered_map<unsigned char, string>& codigoHuffman, const string& nomeArquivoCompactado) {
    ofstream outFile(nomeArquivoCompactado, ios::binary);
    if (!outFile) {
        cerr << "Nao foi possível criar o arquivo compactado.";
        exit(1);
    }

    string codigo;
    for (unsigned char c : texto) {
        if (ehCaractereAceitavel(c)) {
            codigo += codigoHuffman.at(c);
        }
    }

    // Converter a string de códigos binários em bytes
    int numBytes = (codigo.length() + 7) / 8;
    for (int i = 0; i < numBytes; ++i) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; ++j) {
            if (i * 8 + j < codigo.length() && codigo[i * 8 + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outFile.put(byte);
    }

    outFile.close();
}

void construirArvoreHuffman(const string& texto) {
    unordered_map<unsigned char, int> frequencia;

    for (unsigned char c : texto) {
        if (ehCaractereAceitavel(c)) {
            ++frequencia[c];
        }
    }

    priority_queue<No*, vector<No*>, comparar> pq;
    for (auto par : frequencia) {
        pq.push(criarNo(par.first, par.second));
    }
    while (pq.size() != 1) {
        No* esquerda = pq.top(); pq.pop();
        No* direita = pq.top(); pq.pop();
        int soma = esquerda->frequencia + direita->frequencia;
        No* pai = criarNo('\0', soma);
        pai->esquerda = esquerda;
        pai->direita = direita;
        pq.push(pai);
    }
    No* raiz = pq.top();
    unordered_map<unsigned char, string> codigoHuffman;
    codificar(raiz, "", codigoHuffman);

    // Imprimir os códigos Huffman gerados
    cout << "Codigos Huffman gerados:" << endl;
    for (auto par : codigoHuffman) {
        cout << "Caractere: " << par.first << " -> Codigo Huffman: " << par.second << endl;
    }

    // Exportar a contagem de frequência e os códigos Huffman para um arquivo TXT
    ofstream outFileFreq("contagem_frequencia_huffman.txt");
    if (!outFileFreq) {
        cerr << "Nao foi possível criar contagem_frequencia_huffman.txt";
        exit(1);
    }
    for (auto par : frequencia) {
        outFileFreq << "Caractere: " << par.first << "   ->  Frequencia: " << par.second
                    << "   ->  Codigo Huffman: " << codigoHuffman[par.first] << endl;
    }
    cout << "Arquivo contagem_frequencia_huffman.txt gerado." << endl;
    outFileFreq.close();

    // Compactar o arquivo original usando os códigos Huffman
    escreverCompactado(texto, codigoHuffman, "arquivo_compactado.bin");
    cout << "Arquivo Compactado arquivo_compactado.bin gerado." << endl;
}

int main() {
    ifstream inFile("../clarissa.txt", ios::binary);
    if (!inFile) {
        cerr << "Nao foi possível abrir o arquivo clarissa.txt";
        exit(1);
    }

    string texto((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    construirArvoreHuffman(texto);

    return 0;
}
