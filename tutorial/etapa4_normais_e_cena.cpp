// ETAPA 4 - Normais e varios objetos
// Objetivos:
//   - Calcular ONDE o raio bateu (o valor de t), nao so SE bateu.
//   - Calcular a NORMAL: a direcao "para fora" da superficie naquele ponto.
//     Ela e a base de toda a iluminacao das proximas etapas.
//   - Ter varios objetos e mostrar sempre o MAIS PROXIMO da camera.
//
// Resultado: etapa4.bmp, uma esfera colorida pela normal em cima de um "chao"
// (que e so uma esfera gigante).

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
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

struct Raio {
    Vec3 origem, direcao;
    Vec3 em(double t) const { return origem + t * direcao; }  // NOVO: ponto ao longo do raio
};

// NOVO: o que sabemos sobre o ponto atingido
struct Impacto {
    double t;
    Vec3 ponto;
    Vec3 normal;
};

struct Esfera {
    Vec3 centro;
    double raio;

    // So aceita impactos com t entre tMin e tMax (assim ignoramos o que esta atras)
    bool atinge(const Raio& r, double tMin, double tMax, Impacto& imp) const {
        Vec3 oc = centro - r.origem;
        double a = dot(r.direcao, r.direcao);
        double h = dot(r.direcao, oc);  // versao simplificada: b = -2h
        double c = dot(oc, oc) - raio * raio;
        double delta = h * h - a * c;
        if (delta < 0) return false;

        double raiz = (h - std::sqrt(delta)) / a;  // primeiro a raiz mais proxima
        if (raiz <= tMin || raiz >= tMax) {
            raiz = (h + std::sqrt(delta)) / a;
            if (raiz <= tMin || raiz >= tMax) return false;
        }
        imp.t = raiz;
        imp.ponto = r.em(raiz);
        imp.normal = (1.0 / raio) * (imp.ponto - centro);  // aponta do centro para fora
        return true;
    }
};

// NOVO: testa todas as esferas e fica com a mais proxima
bool atingeCena(const std::vector<Esfera>& cena, const Raio& r, Impacto& imp) {
    bool acertou = false;
    double maisProximo = std::numeric_limits<double>::infinity();
    for (const Esfera& e : cena) {
        Impacto temp{};
        if (e.atinge(r, 0.0, maisProximo, temp)) {
            acertou = true;
            maisProximo = temp.t;
            imp = temp;
        }
    }
    return acertou;
}

Vec3 corDoRaio(const Raio& r, const std::vector<Esfera>& cena) {
    Impacto imp{};
    if (atingeCena(cena, r, imp)) {
        // Normal vai de -1 a 1; transformamos em cor de 0 a 1 para "ver" a normal
        return 0.5 * (imp.normal + Vec3{1, 1, 1});
    }
    Vec3 d = normalizar(r.direcao);
    double a = 0.5 * (d.y + 1.0);
    return (1.0 - a) * Vec3{1, 1, 1} + a * Vec3{0.5, 0.7, 1.0};
}

int main() {
    std::vector<Esfera> cena = {
        {{0, 0, -1}, 0.5}, {{0, -100.5, -1}, 100},  // o chao: uma esfera enorme
    };

    const int largura = 400, altura = 225;
    double alturaTela = 2.0, larguraTela = alturaTela * double(largura) / altura;
    Vec3 camera{0, 0, 0}, horizontal{larguraTela, 0, 0}, vertical{0, alturaTela, 0};
    Vec3 canto = camera - 0.5 * horizontal + 0.5 * vertical - Vec3{0, 0, 1};

    std::vector<uint8_t> pixels(largura * altura * 3);
    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            double u = double(x) / (largura - 1), v = double(y) / (altura - 1);
            Raio r{camera, canto + u * horizontal - v * vertical - camera};
            Vec3 cor = corDoRaio(r, cena);
            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = uint8_t(255.999 * cor.x);
            pixels[i + 1] = uint8_t(255.999 * cor.y);
            pixels[i + 2] = uint8_t(255.999 * cor.z);
        }
    }
    salvarBMP("etapa4.bmp", largura, altura, pixels);
    std::cout << "Imagem salva em etapa4.bmp\n";
}

// DESAFIOS
// 1. Adicione mais 2 esferas e confira que a mais proxima sempre "tampa" a de tras.
// 2. Pinte cada esfera de uma cor fixa, mas mais escura quando a normal aponta
//    para baixo (dica: multiplique a cor por 0.5 * (normal.y + 1)).
