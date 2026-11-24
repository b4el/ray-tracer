// Ray tracer em C++17 - ponto de entrada.
// Exemplo: raytracer --cena capa --largura 1200 --amostras 100 --saida capa.bmp

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Cenas.h"
#include "Renderizador.h"

namespace {

struct Opcoes {
    std::string cena = "capa";
    std::string saida = "imagem.bmp";
    int largura = 800;
    double proporcao = 16.0 / 9.0;
    int amostras = 50;
    int profundidade = 20;
    int threads = 0;
    std::string exportar;  // se preenchido, so exporta as cenas em JSON
    bool semBVH = false;   // desliga a BVH (para comparar o desempenho)
};

void mostrarAjuda() {
    std::cout << "Uso: raytracer [opcoes]\n\n"
              << "  --cena NOME         capa | simples | noite | modelos  (padrao: capa)\n"
              << "  --saida ARQUIVO     .bmp ou .ppm            (padrao: imagem.bmp)\n"
              << "  --largura N         largura em pixels       (padrao: 800)\n"
              << "  --amostras N        amostras por pixel      (padrao: 50)\n"
              << "  --profundidade N    reflexoes por raio      (padrao: 20)\n"
              << "  --threads N         0 = todos os nucleos    (padrao: 0)\n"
              << "  --exportar ARQUIVO  salva as cenas em JSON (para a pagina com Three.js)\n"
              << "  --sem-bvh           desliga a BVH (para medir o ganho de desempenho)\n"
              << "  --ajuda             mostra esta mensagem\n";
}

int lerInteiro(const std::string& texto, const std::string& opcao) {
    try {
        size_t pos = 0;
        int valor = std::stoi(texto, &pos);
        if (pos != texto.size() || valor < 0) throw std::invalid_argument("");
        return valor;
    } catch (...) {
        throw std::invalid_argument("Valor invalido para " + opcao + ": " + texto);
    }
}

Opcoes lerOpcoes(int argc, char* argv[]) {
    Opcoes op;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--ajuda" || arg == "-h") {
            mostrarAjuda();
            std::exit(0);
        }
        if (arg == "--sem-bvh") {  // opcao sem valor
            op.semBVH = true;
            continue;
        }
        if (i + 1 >= argc) throw std::invalid_argument("Faltou o valor de " + arg);
        std::string valor = argv[++i];

        if (arg == "--cena") op.cena = valor;
        else if (arg == "--saida") op.saida = valor;
        else if (arg == "--largura") op.largura = lerInteiro(valor, arg);
        else if (arg == "--amostras") op.amostras = lerInteiro(valor, arg);
        else if (arg == "--profundidade") op.profundidade = lerInteiro(valor, arg);
        else if (arg == "--threads") op.threads = lerInteiro(valor, arg);
        else if (arg == "--exportar") op.exportar = valor;
        else throw std::invalid_argument("Opcao desconhecida: " + arg);
    }
    if (op.largura < 16) throw std::invalid_argument("A largura minima e 16 pixels.");
    if (op.amostras < 1) throw std::invalid_argument("Use pelo menos 1 amostra.");
    return op;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        Opcoes op = lerOpcoes(argc, argv);

        if (!op.exportar.empty()) {
            exportarCenasJSON(op.exportar);
            std::cout << "Cenas exportadas para " << op.exportar << "\n";
            return 0;
        }

        ConfigRender cfg;
        cfg.largura = op.largura;
        cfg.altura = static_cast<int>(op.largura / op.proporcao);
        cfg.amostrasPorPixel = op.amostras;
        cfg.profundidadeMaxima = op.profundidade;
        cfg.threads = op.threads;

        CenaPronta cena = montarCena(op.cena, !op.semBVH);
        cfg.ceu = cena.ceu;
        Camera camera(cena.camera, static_cast<double>(cfg.largura) / cfg.altura);

        std::cout << "Cena: " << op.cena << " | " << cfg.largura << "x" << cfg.altura << " | " << cfg.amostrasPorPixel
                  << " amostras" << (op.semBVH ? " | sem BVH" : "") << "\n";

        auto inicio = std::chrono::steady_clock::now();
        Imagem imagem = renderizar(*cena.mundo, camera, cfg);
        auto fim = std::chrono::steady_clock::now();

        imagem.salvar(op.saida);
        double segundos = std::chrono::duration<double>(fim - inicio).count();
        std::cout << "Pronto em " << segundos << " s -> " << op.saida << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n\n";
        mostrarAjuda();
        return 1;
    }
}
