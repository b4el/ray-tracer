#pragma once

#include <string>
#include <vector>

#include "Vec3.h"

// Uma textura decide a cor de cada ponto de uma superficie.
//   (u, v): coordenadas da superficie, de 0 a 1 (usadas por imagens)
//   p: o ponto no espaco 3D (usado pelo xadrez)
class Textura {
public:
    virtual ~Textura() = default;
    virtual Cor valor(double u, double v, const Ponto3& p) const = 0;
};

// Uma cor so, igual em todo lugar
class CorSolida : public Textura {
public:
    explicit CorSolida(const Cor& cor) : cor_(cor) {}
    Cor valor(double, double, const Ponto3&) const override { return cor_; }

private:
    Cor cor_;
};

// Xadrez 3D: o espaco e dividido em cubos de lado 'escala', alternando duas cores.
// Como usa a posicao 3D, funciona em qualquer objeto (esfera, chao, modelo .obj).
class Xadrez : public Textura {
public:
    Xadrez(const Cor& par, const Cor& impar, double escala);
    Cor valor(double u, double v, const Ponto3& p) const override;

private:
    Cor par_, impar_;
    double inverso_;  // 1 / escala
};

// Imagem (PNG, JPG, BMP...) mapeada pelas coordenadas (u, v) da superficie
class TexturaImagem : public Textura {
public:
    // Lanca std::runtime_error se o arquivo nao puder ser lido
    explicit TexturaImagem(const std::string& caminho);
    Cor valor(double u, double v, const Ponto3& p) const override;

    int largura() const { return largura_; }
    int altura() const { return altura_; }

private:
    int largura_ = 0, altura_ = 0;
    std::vector<Cor> pixels_;  // ja convertidos para cor linear
};
