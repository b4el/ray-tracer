// ETAPA 6 - Materiais: fosco, metal e vidro (com polimorfismo!)
// Objetivo: cada esfera ter um MATERIAL diferente. Usamos uma classe base
// abstrata "Material" e classes filhas que sobrescrevem "espalhar".
//   - Fosco: quica em direcao aleatoria (etapa 5) e tem uma cor.
//   - Metal: reflete como espelho. "rugosidade" deixa o reflexo borrado.
//   - Vidro: a luz ATRAVESSA e entorta (refracao, Lei de Snell). Em angulos
//     rasantes ela reflete (aproximacao de Schlick).
//
// Resultado: etapa6.bmp, tres esferas: vidro, fosca e dourada.

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <vector>

#include "bmp.h"

struct Vec3 {
    double x, y, z;
};
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator-(Vec3 a) { return {-a.x, -a.y, -a.z}; }
Vec3 operator*(double t, Vec3 v) { return {t * v.x, t * v.y, t * v.z}; }
Vec3 operator*(Vec3 a, Vec3 b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }  // NOVO: cor * cor
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
double comprimento(Vec3 v) { return std::sqrt(dot(v, v)); }
Vec3 normalizar(Vec3 v) { return (1.0 / comprimento(v)) * v; }

// NOVO: reflexao e refracao
Vec3 refletir(Vec3 v, Vec3 n) { return v - 2 * dot(v, n) * n; }
Vec3 refratar(Vec3 v, Vec3 n, double eta) {
    double cosTheta = std::fmin(dot(-v, n), 1.0);
    Vec3 perp = eta * (v + cosTheta * n);
    Vec3 paralelo = -std::sqrt(std::fabs(1.0 - dot(perp, perp))) * n;
    return perp + paralelo;
}

std::mt19937 gerador(42);
double aleatorio() { return std::uniform_real_distribution<double>(0.0, 1.0)(gerador); }
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

class Material;  // declarado antes porque Impacto aponta para ele

struct Impacto {
    double t;
    Vec3 ponto, normal;
    bool faceDeFora;  // NOVO: o vidro precisa saber se o raio esta entrando ou saindo
    const Material* material;
};

// ---------- NOVO: hierarquia de materiais ----------
class Material {
public:
    virtual ~Material() = default;
    // Devolve false se a luz foi absorvida
    virtual bool espalhar(const Raio& entrada, const Impacto& imp, Vec3& atenuacao, Raio& saida) const = 0;
};

class Fosco : public Material {
    Vec3 cor;

public:
    explicit Fosco(Vec3 c) : cor(c) {}
    bool espalhar(const Raio&, const Impacto& imp, Vec3& atenuacao, Raio& saida) const override {
        saida = {imp.ponto, imp.normal + direcaoAleatoria()};
        atenuacao = cor;
        return true;
    }
};

class Metal : public Material {
    Vec3 cor;
    double rugosidade;

public:
    Metal(Vec3 c, double r) : cor(c), rugosidade(r) {}
    bool espalhar(const Raio& entrada, const Impacto& imp, Vec3& atenuacao, Raio& saida) const override {
        Vec3 refletido = normalizar(refletir(entrada.direcao, imp.normal));
        saida = {imp.ponto, refletido + rugosidade * direcaoAleatoria()};
        atenuacao = cor;
        return dot(saida.direcao, imp.normal) > 0;
    }
};

class Vidro : public Material {
    double indice;  // 1.5 para vidro
public:
    explicit Vidro(double i) : indice(i) {}
    bool espalhar(const Raio& entrada, const Impacto& imp, Vec3& atenuacao, Raio& saida) const override {
        atenuacao = {1, 1, 1};
        double eta = imp.faceDeFora ? 1.0 / indice : indice;
        Vec3 d = normalizar(entrada.direcao);
        double cosTheta = std::fmin(dot(-d, imp.normal), 1.0);
        double senTheta = std::sqrt(1.0 - cosTheta * cosTheta);

        double r0 = (1 - eta) / (1 + eta);
        r0 = r0 * r0;
        double schlick = r0 + (1 - r0) * std::pow(1 - cosTheta, 5);

        bool reflete = eta * senTheta > 1.0 || schlick > aleatorio();
        saida = {imp.ponto, reflete ? refletir(d, imp.normal) : refratar(d, imp.normal, eta)};
        return true;
    }
};

struct Esfera {
    Vec3 centro;
    double raio;
    std::shared_ptr<Material> material;  // NOVO

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
        Vec3 normalParaFora = (1.0 / raio) * (imp.ponto - centro);
        imp.faceDeFora = dot(r.direcao, normalParaFora) < 0;
        imp.normal = imp.faceDeFora ? normalParaFora : -normalParaFora;  // sempre contra o raio
        imp.material = material.get();
        return true;
    }
};

bool atingeCena(const std::vector<Esfera>& cena, const Raio& r, Impacto& imp) {
    bool acertou = false;
    double maisProximo = std::numeric_limits<double>::infinity();
    for (const Esfera& e : cena) {
        Impacto temp{};
        if (e.atinge(r, 0.001, maisProximo, temp)) {
            acertou = true;
            maisProximo = temp.t;
            imp = temp;
        }
    }
    return acertou;
}

Vec3 corDoRaio(const Raio& r, const std::vector<Esfera>& cena, int profundidade) {
    if (profundidade <= 0) return {0, 0, 0};
    Impacto imp{};
    if (atingeCena(cena, r, imp)) {
        Raio saida;
        Vec3 atenuacao;
        // POLIMORFISMO: cada material decide do seu jeito
        if (imp.material->espalhar(r, imp, atenuacao, saida))
            return atenuacao * corDoRaio(saida, cena, profundidade - 1);
        return {0, 0, 0};
    }
    Vec3 d = normalizar(r.direcao);
    double a = 0.5 * (d.y + 1.0);
    return (1.0 - a) * Vec3{1, 1, 1} + a * Vec3{0.5, 0.7, 1.0};
}

uint8_t paraByte(double c) {
    c = std::sqrt(c);
    if (c > 0.999) c = 0.999;
    return uint8_t(256 * c);
}

int main() {
    std::vector<Esfera> cena = {
        {{0, -100.5, -1}, 100, std::make_shared<Fosco>(Vec3{0.8, 0.8, 0.0})},
        {{0, 0, -1.2}, 0.5, std::make_shared<Fosco>(Vec3{0.1, 0.2, 0.5})},
        {{-1, 0, -1}, 0.5, std::make_shared<Vidro>(1.5)},
        {{1, 0, -1}, 0.5, std::make_shared<Metal>(Vec3{0.8, 0.6, 0.2}, 0.1)},
    };

    const int largura = 400, altura = 225, amostras = 60, profundidade = 15;
    double alturaTela = 2.0, larguraTela = alturaTela * double(largura) / altura;
    Vec3 camera{0, 0, 0.8}, horizontal{larguraTela, 0, 0}, vertical{0, alturaTela, 0};  // um pouco mais para tras
    Vec3 canto = camera - 0.5 * horizontal + 0.5 * vertical - Vec3{0, 0, 1};

    std::vector<uint8_t> pixels(largura * altura * 3);
    for (int y = 0; y < altura; y++) {
        std::cout << "\rLinhas restantes: " << (altura - y) << "   " << std::flush;
        for (int x = 0; x < largura; x++) {
            Vec3 soma{0, 0, 0};
            for (int s = 0; s < amostras; s++) {
                double u = (x + aleatorio()) / (largura - 1), v = (y + aleatorio()) / (altura - 1);
                Raio r{camera, canto + u * horizontal - v * vertical - camera};
                soma = soma + corDoRaio(r, cena, profundidade);
            }
            Vec3 cor = (1.0 / amostras) * soma;
            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = paraByte(cor.x);
            pixels[i + 1] = paraByte(cor.y);
            pixels[i + 2] = paraByte(cor.z);
        }
    }
    salvarBMP("etapa6.bmp", largura, altura, pixels);
    std::cout << "\nImagem salva em etapa6.bmp\n";
}

// DESAFIOS
// 1. Mude a rugosidade do metal para 0 (espelho perfeito) e para 1 (bem fosco).
// 2. Crie uma bolha: uma esfera de Vidro(1.0 / 1.5) com raio 0.4 dentro da de vidro.
// 3. Crie um material novo, "Espelho Colorido", que reflete mas tinge de vermelho.
//
// PROXIMO PASSO: abra o projeto principal (pasta src/). Ele tem tudo isso
// organizado em varios arquivos, mais camera movel, desfoque, luzes,
// varias threads e a BVH, que deixa centenas de objetos rapidos.
