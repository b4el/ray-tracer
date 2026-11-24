// ETAPA 3 - A primeira esfera
// Objetivo: descobrir se um raio bate numa esfera.
//
// A matematica (com calma):
//   Um ponto P esta na esfera de centro C e raio R quando |P - C| = R.
//   O raio e P(t) = O + t*D. Substituindo, vira uma equacao do 2o grau em t:
//       (D.D) t^2  -  2 D.(C-O) t  +  (C-O).(C-O) - R^2  =  0
//   Se o "delta" (b^2 - 4ac) for negativo, nao ha solucao: o raio erra a esfera.
//   ("." e o produto escalar, a funcao dot abaixo)
//
// Resultado: etapa3.bmp, uma bolinha vermelha no ceu.

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

#include "bmp.h"

struct Vec3 {
    double x, y, z;
};
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(double t, Vec3 v) { return {t * v.x, t * v.y, t * v.z}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }  // NOVO: produto escalar
double comprimento(Vec3 v) { return std::sqrt(dot(v, v)); }
Vec3 normalizar(Vec3 v) { return (1.0 / comprimento(v)) * v; }

struct Raio {
    Vec3 origem;
    Vec3 direcao;
};

// NOVO: o raio bate na esfera?
bool atingeEsfera(Vec3 centro, double raio, const Raio& r) {
    Vec3 oc = centro - r.origem;
    double a = dot(r.direcao, r.direcao);
    double b = -2.0 * dot(r.direcao, oc);
    double c = dot(oc, oc) - raio * raio;
    double delta = b * b - 4 * a * c;
    return delta >= 0;
}

Vec3 corDoRaio(const Raio& r) {
    if (atingeEsfera({0, 0, -1}, 0.5, r)) return {1, 0, 0};  // vermelho

    Vec3 d = normalizar(r.direcao);
    double a = 0.5 * (d.y + 1.0);
    return (1.0 - a) * Vec3{1, 1, 1} + a * Vec3{0.5, 0.7, 1.0};
}

int main() {
    const int largura = 400, altura = 225;
    double alturaTela = 2.0, larguraTela = alturaTela * double(largura) / altura;
    Vec3 camera{0, 0, 0}, horizontal{larguraTela, 0, 0}, vertical{0, alturaTela, 0};
    Vec3 canto = camera - 0.5 * horizontal + 0.5 * vertical - Vec3{0, 0, 1};

    std::vector<uint8_t> pixels(largura * altura * 3);
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            double u = double(x) / (largura - 1), v = double(y) / (altura - 1);
            Raio r{camera, canto + u * horizontal - v * vertical - camera};
            Vec3 cor = corDoRaio(r);
            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = uint8_t(255.999 * cor.x);
            pixels[i + 1] = uint8_t(255.999 * cor.y);
            pixels[i + 2] = uint8_t(255.999 * cor.z);
        }
    }
    salvarBMP("etapa3.bmp", largura, altura, pixels);
    std::cout << "Imagem salva em etapa3.bmp\n";
}

// DESAFIOS
// 1. Coloque uma segunda esfera, verde, ao lado da vermelha.
// 2. Mude o centro para {0, 0, +1} (atras da camera). Ela ainda aparece! Por que?
//    Dica: a equacao tambem acha solucoes com t negativo. Isso e corrigido na etapa 4.
