#pragma once

#include "Camera.h"
#include "Imagem.h"
#include "Objeto.h"

struct ConfigRender {
    int largura = 800;
    int altura = 450;
    int amostrasPorPixel = 50;    // mais amostras = menos "chuvisco", mais lento
    int profundidadeMaxima = 20;  // quantas vezes um raio pode quicar
    int threads = 0;              // 0 = usar todos os nucleos do processador
    bool ceu = true;              // false = fundo preto (so luzes iluminam)
    bool mostrarProgresso = true;
};

// Gera a imagem disparando raios pela cena, dividindo as linhas entre threads.
Imagem renderizar(const Objeto& mundo, const Camera& camera, const ConfigRender& config);

// Cor que chega na camera vinda da direcao do raio (funcao recursiva)
Cor corDoRaio(const Raio& r, const Objeto& mundo, int profundidade, bool ceu);
