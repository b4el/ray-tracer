// ETAPA 2 - Vetores, raios e um ceu
// Objetivo: criar a struct Vec3 (vetor 3D) e disparar um RAIO da camera
// para cada pixel. Por enquanto o raio nao bate em nada: a cor depende so
// da direcao dele (mais para cima = mais azul).
//
// Resultado: etapa2.bmp, um ceu em degrade.

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

#include "bmp.h"

// ---------- Vec3: o tipo mais importante do ray tracer ----------
// Serve para pontos (x, y, z), direcoes e ate cores (r, g, b).
struct Vec3 {
    double x, y, z;
};
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(double t, Vec3 v) { return {t * v.x, t * v.y, t * v.z}; }
double comprimento(Vec3 v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
Vec3 normalizar(Vec3 v) { return (1.0 / comprimento(v)) * v; }

// ---------- Raio: P(t) = origem + t * direcao ----------
struct Raio {
    Vec3 origem;
    Vec3 direcao;
};

// Cor que a camera ve na direcao do raio
Vec3 corDoRaio(const Raio& r) {
    Vec3 d = normalizar(r.direcao);
    double a = 0.5 * (d.y + 1.0);  // d.y vai de -1 a 1 -> a vai de 0 a 1
    Vec3 branco{1, 1, 1}, azul{0.5, 0.7, 1.0};
    return (1.0 - a) * branco + a * azul;  // "interpolacao linear" (lerp)
}

int main() {
    const int largura = 400;
    const int altura = 225;  // proporcao 16:9

    // A "tela" fica 1 unidade a frente da camera (no eixo -z)
    double alturaTela = 2.0;
    double larguraTela = alturaTela * double(largura) / altura;
    Vec3 camera{0, 0, 0};
    Vec3 horizontal{larguraTela, 0, 0};
    Vec3 vertical{0, alturaTela, 0};
    Vec3 cantoSuperiorEsquerdo = camera - 0.5 * horizontal + 0.5 * vertical - Vec3{0, 0, 1};

    std::vector<uint8_t> pixels(largura * altura * 3);
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            double u = double(x) / (largura - 1);
            double v = double(y) / (altura - 1);
            Vec3 pontoNaTela = cantoSuperiorEsquerdo + u * horizontal - v * vertical;
            Raio r{camera, pontoNaTela - camera};

            Vec3 cor = corDoRaio(r);
            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = uint8_t(255.999 * cor.x);
            pixels[i + 1] = uint8_t(255.999 * cor.y);
            pixels[i + 2] = uint8_t(255.999 * cor.z);
        }
    }

    salvarBMP("etapa2.bmp", largura, altura, pixels);
    std::cout << "Imagem salva em etapa2.bmp\n";
}

// DESAFIOS
// 1. Troque o azul por um ceu de por do sol (laranja e roxo).
// 2. O que acontece se a camera ficar em {0, 0, 0} mas a tela em z = -0.5? Por que?
