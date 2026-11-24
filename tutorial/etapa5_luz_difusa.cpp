// ETAPA 5 - Luz de verdade: antialiasing, superficies foscas e recursao
// Objetivos:
//   - ANTIALIASING: varios raios por pixel, em posicoes aleatorias, e tiramos a media.
//     Isso suaviza os "serrilhados" das bordas.
//   - MATERIAL FOSCO: quando a luz bate, ela quica numa direcao aleatoria.
//     O raio novo continua viajando (RECURSAO) ate chegar no ceu ou desistir.
//     Cada quique absorve metade da luz -> aparecem sombras sozinhas!
//   - CORRECAO GAMA: tirar a raiz quadrada da cor deixa o brilho natural ao olho.
//
// Resultado: etapa5.bmp, esfera fosca com sombra suave. Demora alguns segundos.

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

#include "bmp.h"

struct Vec3 {
    double x, y, z;
};
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(double t, Vec3 v) { return {t * v.x, t * v.y, t * v.z}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
double comprimento(Vec3 v) { return std::sqrt(dot(v, v)); }
Vec3 normalizar(Vec3 v) { return (1.0 / comprimento(v)) * v; }

// NOVO: numeros aleatorios
std::mt19937 gerador(42);
double aleatorio() { return std::uniform_real_distribution<double>(0.0, 1.0)(gerador); }

// NOVO: direcao aleatoria de comprimento 1 (sorteia ate cair dentro da esfera unitaria)
Vec3 direcaoAleatoria() {
    while (true) {
        Vec3 p{2 * aleatorio() - 1, 2 * aleatorio() - 1, 2 * aleatorio() - 1};
        double l = dot(p, p);
        if (l > 1e-12 && l <= 1) return (1.0 / std::sqrt(l)) * p;
    }
}

struct Raio {
    Vec3 origem, direcao;
    Vec3 em(double t) const { return origem + t * direcao; }
};
struct Impacto {
    double t;
    Vec3 ponto, normal;
};

struct Esfera {
    Vec3 centro;
    double raio;
    bool atinge(const Raio& r, double tMin, double tMax, Impacto& imp) const {
        Vec3 oc = centro - r.origem;
        double a = dot(r.direcao, r.direcao), h = dot(r.direcao, oc), c = dot(oc, oc) - raio * raio;
        double delta = h * h - a * c;
        if (delta < 0) return false;
        double raiz = (h - std::sqrt(delta)) / a;
        if (raiz <= tMin || raiz >= tMax) {
            raiz = (h + std::sqrt(delta)) / a;
            if (raiz <= tMin || raiz >= tMax) return false;
        }
        imp.t = raiz;
        imp.ponto = r.em(raiz);
        imp.normal = (1.0 / raio) * (imp.ponto - centro);
        return true;
    }
};

bool atingeCena(const std::vector<Esfera>& cena, const Raio& r, Impacto& imp) {
    bool acertou = false;
    double maisProximo = std::numeric_limits<double>::infinity();
    for (const Esfera& e : cena) {
        Impacto temp{};
        // 0.001 em vez de 0: evita o raio bater de novo no ponto de onde saiu
        if (e.atinge(r, 0.001, maisProximo, temp)) {
            acertou = true;
            maisProximo = temp.t;
            imp = temp;
        }
    }
    return acertou;
}

// NOVO: funcao RECURSIVA (ela chama a si mesma)
Vec3 corDoRaio(const Raio& r, const std::vector<Esfera>& cena, int profundidade) {
    if (profundidade <= 0) return {0, 0, 0};  // quicou demais: consideramos que nao chega luz

    Impacto imp{};
    if (atingeCena(cena, r, imp)) {
        Vec3 novaDirecao = imp.normal + direcaoAleatoria();  // quique "lambertiano"
        Raio quique{imp.ponto, novaDirecao};
        return 0.5 * corDoRaio(quique, cena, profundidade - 1);  // superficie absorve 50%
    }
    Vec3 d = normalizar(r.direcao);
    double a = 0.5 * (d.y + 1.0);
    return (1.0 - a) * Vec3{1, 1, 1} + a * Vec3{0.5, 0.7, 1.0};
}

uint8_t paraByte(double c) {
    c = std::sqrt(c);  // NOVO: correcao gama
    if (c > 0.999) c = 0.999;
    return uint8_t(256 * c);
}

int main() {
    std::vector<Esfera> cena = {{{0, 0, -1}, 0.5}, {{0, -100.5, -1}, 100}};

    const int largura = 400, altura = 225;
    const int amostras = 50;      // raios por pixel
    const int profundidade = 10;  // quiques maximos

    double alturaTela = 2.0, larguraTela = alturaTela * double(largura) / altura;
    Vec3 camera{0, 0, 0}, horizontal{larguraTela, 0, 0}, vertical{0, alturaTela, 0};
    Vec3 canto = camera - 0.5 * horizontal + 0.5 * vertical - Vec3{0, 0, 1};

    std::vector<uint8_t> pixels(largura * altura * 3);
    for (int y = 0; y < altura; y++) {
        std::cout << "\rLinhas restantes: " << (altura - y) << "   " << std::flush;
        for (int x = 0; x < largura; x++) {
            Vec3 soma{0, 0, 0};
            for (int s = 0; s < amostras; s++) {
                double u = (x + aleatorio()) / (largura - 1);  // NOVO: posicao aleatoria dentro do pixel
                double v = (y + aleatorio()) / (altura - 1);
                Raio r{camera, canto + u * horizontal - v * vertical - camera};
                soma = soma + corDoRaio(r, cena, profundidade);
            }
            Vec3 cor = (1.0 / amostras) * soma;  // media das amostras
            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = paraByte(cor.x);
            pixels[i + 1] = paraByte(cor.y);
            pixels[i + 2] = paraByte(cor.z);
        }
    }
    salvarBMP("etapa5.bmp", largura, altura, pixels);
    std::cout << "\nImagem salva em etapa5.bmp\n";
}

// DESAFIOS
// 1. Troque amostras para 1 e depois para 200. Compare o "chuvisco" e o tempo.
// 2. Troque o 0.5 do quique por 0.9 e por 0.1. O que muda?
// 3. Tire a correcao gama (comente o sqrt). A imagem fica mais escura. Por que?
