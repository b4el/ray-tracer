#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Camera.h"
#include "Objeto.h"

// Descricao "em dados" de materiais, texturas e objetos.
// A cena e montada a partir dessas descricoes, e elas tambem podem ser
// exportadas em JSON (usado pela pagina de comparacao com Three.js).

// Textura opcional de um material difuso
struct DescricaoTextura {
    enum class Tipo { Nenhuma, Xadrez, Imagem };
    Tipo tipo = Tipo::Nenhuma;
    Cor cor2{0, 0, 0};    // Xadrez: segunda cor (a primeira e a cor do material)
    double escala = 1;    // Xadrez: tamanho de cada quadrado
    std::string arquivo;  // Imagem: nome do arquivo dentro de texturas/
};

struct DescricaoMaterial {
    enum class Tipo { Difuso, Metal, Vidro, Luz };
    Tipo tipo = Tipo::Difuso;
    Cor cor{0.5, 0.5, 0.5};    // Difuso, Metal e Luz
    double rugosidade = 0;     // Metal
    double indice = 1.5;       // Vidro
    DescricaoTextura textura;  // so para Difuso
};

struct DescricaoEsfera {
    Ponto3 centro;
    double raio = 1;
    DescricaoMaterial material;
};

// Um modelo 3D lido de modelos/<arquivo> (formato .obj)
struct DescricaoMalha {
    std::string arquivo;
    Vec3 deslocamento;  // onde o modelo fica na cena
    double escala = 1;
    DescricaoMaterial material;
};

// Quadrilatero: canto + dois lados (veja Quadrilatero.h)
struct DescricaoQuad {
    Ponto3 canto;
    Vec3 u, v;
    DescricaoMaterial material;
};

// Uma cena pronta: objetos + posicao da camera + se tem ceu
struct CenaPronta {
    std::string nome;
    std::vector<DescricaoEsfera> esferas;
    std::vector<DescricaoMalha> malhas;
    std::vector<DescricaoQuad> quads;
    std::shared_ptr<Objeto> mundo;
    ConfigCamera camera;
    bool ceu = true;
};

// Nomes aceitos: "capa", "simples", "noite", "modelos".
// usarBVH = false monta uma lista simples (para medir o ganho da BVH).
CenaPronta montarCena(const std::string& nome, bool usarBVH = true);
std::vector<std::string> nomesDasCenas();

// Cria os objetos de verdade (esferas, triangulos, BVH) a partir das descricoes.
// Usado pelas cenas prontas e pelas cenas escritas em Lua.
void montarMundo(CenaPronta& cena, bool usarBVH = true);

// Salva todas as cenas em um arquivo JSON
void exportarCenasJSON(const std::string& caminho);
