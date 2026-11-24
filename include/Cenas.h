#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Camera.h"
#include "Objeto.h"

// Descricao "em dados" de um material e de uma esfera.
// A cena e montada a partir dessas descricoes, e elas tambem podem ser
// exportadas em JSON (usado pela pagina de comparacao com Three.js).
struct DescricaoMaterial {
    enum class Tipo { Difuso, Metal, Vidro, Luz };
    Tipo tipo = Tipo::Difuso;
    Cor cor{0.5, 0.5, 0.5};   // Difuso, Metal e Luz
    double rugosidade = 0;    // Metal
    double indice = 1.5;      // Vidro
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

// Uma cena pronta: objetos + posicao da camera + se tem ceu
struct CenaPronta {
    std::string nome;
    std::vector<DescricaoEsfera> esferas;
    std::vector<DescricaoMalha> malhas;
    std::shared_ptr<Objeto> mundo;
    ConfigCamera camera;
    bool ceu = true;
};

// Nomes aceitos: "capa", "simples", "noite", "modelos".
// usarBVH = false monta uma lista simples (para medir o ganho da BVH).
CenaPronta montarCena(const std::string& nome, bool usarBVH = true);
std::vector<std::string> nomesDasCenas();

// Salva todas as cenas em um arquivo JSON
void exportarCenasJSON(const std::string& caminho);
