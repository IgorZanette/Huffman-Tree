#include <iostream>
#include <string>
#include <vector> 
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <queue>
#include <codecvt>
#include <locale>
#include <bitset>
#include <iomanip>

using namespace std;

struct nodo {
    wchar_t caractere;
    int frequencia;
    nodo *esquerdo;
    nodo *direito;
};

nodo *gerar_no(wchar_t ch, int freq, nodo *esquerdo, nodo *direito) {
    nodo *p = new nodo;
    p->caractere = ch;
    p->frequencia = freq;
    p->esquerdo = esquerdo;
    p->direito = direito;
    return p;
}

struct prioridade {
    bool operator()(nodo* esquerdo, nodo* direito) {
        return esquerdo->frequencia > direito->frequencia;
    }
};

string conversao_utf8(const wstring& string) {
    wstring_convert<codecvt_utf8<wchar_t>> convertido;
    return convertido.to_bytes(string);
}

string identificar_nodo(const void* identidade) {
    stringstream ss;
    ss << reinterpret_cast<uintptr_t>(identidade);
    return ss.str();
}

void codificar(nodo *raiz, wstring string, unordered_map<wchar_t, wstring> &codigo_huffman) {
    if (!raiz) return;
    if (!raiz->esquerdo && !raiz->direito)
        codigo_huffman[raiz->caractere] = string;
    codificar(raiz->esquerdo, string + L'0', codigo_huffman);
    codificar(raiz->direito, string + L'1', codigo_huffman);
}

void exportar_dot(const nodo* raiz, const std::string& arquivo, const unordered_map<wchar_t, wstring> &codigo_huffman) {
    ofstream dot_arq(arquivo);
    dot_arq << "digraph G {\n";
    queue<const nodo*> fila;
    fila.push(raiz);

    while (!fila.empty()) {
        const nodo* atual = fila.front();
        fila.pop();
        string identificacao_nodo = "nodo" + to_string(reinterpret_cast<uintptr_t>(atual));

        string label;
        bool marcador = false;
        if (atual->caractere == L'\0') {
            label = to_string(atual->frequencia);
            marcador = true;
        } else {
            string valor_atual;
            if (atual->caractere == L' ')
                valor_atual = "SPACE";
            else if (atual->caractere == L'"')
                valor_atual = "Aspas Duplas";
            else if (atual->caractere == L'{')
                valor_atual = "Abre Chave";
            else if (atual->caractere == L'}')
                valor_atual = "Fecha Chave";
            else if (atual->caractere == L'|')
                valor_atual = "Pipe";
            else if (atual->caractere == L'\n')
                valor_atual = "Nova Linha";
            else if (atual->caractere == L'>')
                valor_atual = "Maior que";
            else if (atual->caractere == L'<')
                valor_atual = "Menor que";
            else
                valor_atual = conversao_utf8(wstring(1, atual->caractere));

            string codigo_binario = conversao_utf8(codigo_huffman.at(atual->caractere));
            label = "{{\' " + valor_atual + " \'|" + to_string(atual->frequencia) + "}|" + codigo_binario + "}";
        }

        if (!marcador) {
            dot_arq << "\t" << identificacao_nodo << " [shape=record, label=\"" << label << "\"];\n";
        } else {
            dot_arq << "\t" << identificacao_nodo << " [label=\"" << label << "\"];\n";
        }

        if (atual->esquerdo) {
            string nome_nodo_esquerdo = "nodo" + identificar_nodo(atual->esquerdo);
            dot_arq << "    " << identificacao_nodo << " -> " << nome_nodo_esquerdo << " [label=\"0\"];\n";
            fila.push(atual->esquerdo);
        }
        if (atual->direito) {
            string nome_nodo_direito = "nodo" + identificar_nodo(atual->direito);
            dot_arq << "    " << identificacao_nodo << " -> " << nome_nodo_direito << " [label=\"1\"];\n";
            fila.push(atual->direito);
        }
    }
    dot_arq << "}\n";
}

// Gera a imagem png do graphviz para a arvore
void draw(const nodo* raiz, const unordered_map<wchar_t, wstring>& codigo_huffman) {
    string arquivo_dot = "arquivo_pronto.dot";
    exportar_dot(raiz, arquivo_dot, codigo_huffman);

    system("dot -Tpng arquivo_pronto.dot -o ../arvore_gerada.png");
}

void gerar_arquivo_comprimido(wifstream &arquivo, const unordered_map<wchar_t, wstring> &codigo_huffman, const string& nome_arquivo_comprimido) {
    ofstream saida_comprimida(nome_arquivo_comprimido, ios::binary); // Abre o arquivo de saída em modo binário
    if (!saida_comprimida.is_open()) {
        cerr << "Não foi possível abrir o arquivo para escrita: " << nome_arquivo_comprimido << endl;
        return;
    }

    // Limpa o estado do arquivo de entrada e move o cursor para o início
    arquivo.clear();
    arquivo.seekg(0, ios::beg);
    wchar_t caractere;
    string buffer;

    while (arquivo.get(caractere)) {
        // Adiciona a codificação do caractere ao buffer
        buffer += conversao_utf8(codigo_huffman.at(caractere));
        
        // Enquanto o buffer tiver pelo menos 8 bits
        while (buffer.size() >= 8) {
            // Converte os primeiros 8 bits do buffer para um byte
            bitset<8> bits(buffer.substr(0, 8));
            // Escreve o byte no arquivo de saída
            saida_comprimida.put(static_cast<unsigned char>(bits.to_ulong()));
            // Remove os 8 bits já escritos do buffer
            buffer.erase(0, 8);
        }
    }

    // Se restarem bits no buffer que não completam 8 bits
    if (!buffer.empty()) {
        // Completa com zeros à direita e escreve o último byte
        bitset<8> bits(buffer);
        saida_comprimida.put(static_cast<unsigned char>(bits.to_ulong()));
    }

    saida_comprimida.close();
}

void construtor_huffman(wifstream &arquivo, wofstream &contagens_huffman_compressao) {
    if (!arquivo) return;
    wchar_t caractere;
    unordered_map<wchar_t, int> frequencia;

    // Contagem das frequências
    while (arquivo.get(caractere))
        frequencia[caractere]++;
    
    // Construção da árvore de Huffman
    priority_queue<nodo*, vector<nodo*>, prioridade> fila_prioritaria; 

    for (auto elemento : frequencia) {
        fila_prioritaria.push(gerar_no(elemento.first, elemento.second, nullptr, nullptr));
    }

    while (fila_prioritaria.size() != 1) {
        nodo *esquerdo = fila_prioritaria.top();
        fila_prioritaria.pop();
        nodo *direito = fila_prioritaria.top();
        fila_prioritaria.pop();  
        int soma = esquerdo->frequencia + direito->frequencia;
        fila_prioritaria.push(gerar_no(L'\0', soma, esquerdo, direito));
    }

    nodo *raiz = fila_prioritaria.top();
    unordered_map<wchar_t, wstring> codigo_huffman;
    codificar(raiz, L"", codigo_huffman);

    // Chamada para gerar o arquivo .dot e a imagem PNG da árvore de Huffman
    draw(raiz, codigo_huffman);

    // Impressão de caracteres, frequências e códigos binários
    contagens_huffman_compressao << L"======================================================\n";
    contagens_huffman_compressao << setw(30) << left << L"______ CARACTERE / FREQUÊNCIA / CÓDIGO BINÁRIO _______\n";
    contagens_huffman_compressao << L"======================================================\n\n";
    
    for (auto elemento : frequencia) {
        // Ajustando o caractere e a frequência
        if (elemento.first == L' ') {
            contagens_huffman_compressao << setw(5) << left << L"SPACE" << L"  ->  " << setw(10) << right << elemento.second << L"    ->    ";
        } else if (elemento.first == L'\n') {
            contagens_huffman_compressao << setw(5) << left << L"\\n" << L"  ->  " << setw(10) << right << elemento.second << L"    ->    ";
        } else {
            contagens_huffman_compressao << setw(5) << left << elemento.first << L"  ->  " << setw(10) << right << elemento.second << L"    ->    ";
        }

        // Verificando e imprimindo o código binário
        if (codigo_huffman.find(elemento.first) != codigo_huffman.end()) {
            contagens_huffman_compressao << codigo_huffman[elemento.first] << endl;
        } else {
            contagens_huffman_compressao << L"Sem código binário associado\n";
        }
    }

  
    contagens_huffman_compressao << L"\n\n_______________  Compressão Estimada   _______________\n";
    contagens_huffman_compressao << L"======================================================\n";

    // Cálculo de estatísticas de compressão
    arquivo.clear();
    arquivo.seekg(0, ios::beg);

    float total_bits = 0, comprimido = 0;
    while (arquivo.get(caractere)) {
        total_bits += 8;
        comprimido += codigo_huffman[caractere].length();
    }

    contagens_huffman_compressao << L"Total de bits sem compressão = " << total_bits << L" bits.\n";
    contagens_huffman_compressao << L"Total de bits comprimido = " << comprimido << L" bits.\n";
    contagens_huffman_compressao << L"------------------------------------------------------\n";
    contagens_huffman_compressao << L"Economia = " << 100 - (comprimido / total_bits) * 100 << "%\n";
    contagens_huffman_compressao << L"======================================================\n";

    // Geração do arquivo comprimido
    gerar_arquivo_comprimido(arquivo, codigo_huffman, "../arquivo_comprimido.bin");
}


int main() {
    wifstream entrada(L"../clarissa.txt");
    wofstream saida("../contagens_huffman_compressao.txt");

    entrada.imbue(locale(entrada.getloc(), new codecvt_utf8<wchar_t>)); 
    saida.imbue(locale(saida.getloc(), new codecvt_utf8<wchar_t>));

    if(!entrada.is_open()){
        wcerr << L"Não foi possível abrir o arquivo.\n"; return 1;
    }
    if (!saida.is_open()){
        wcerr << L"Não foi possível gerar arquivo de saída\n"; return 1;
    }
    construtor_huffman(entrada, saida);
    entrada.close();
    saida.close();
    return 0;
}
