#pragma once

#include <memory>
#include <utility>

#include "Objeto.h"
#include "Raio.h"
#include "Textura.h"

// Um material decide o que acontece quando a luz bate nele:
// para onde o raio vai depois (espalhado) e quanto de cada cor sobra (atenuacao).
class Material {
public:
    virtual ~Material() = default;

    // Devolve false se o raio foi absorvido
    virtual bool espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const = 0;

    // Luz que o proprio material emite (so a Luz emite algo)
    virtual Cor emitida() const { return {0, 0, 0}; }
};

// Superficie fosca: espalha a luz em direcoes aleatorias (giz, papel, parede)
class Difuso : public Material {
public:
    explicit Difuso(const Cor& cor) : textura_(std::make_shared<CorSolida>(cor)) {}
    explicit Difuso(std::shared_ptr<Textura> textura) : textura_(std::move(textura)) {}
    bool espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const override;

private:
    std::shared_ptr<Textura> textura_;  // a cor pode variar ponto a ponto (xadrez, imagem)
};

// Metal: reflete como espelho. 'rugosidade' (0 a 1) deixa o reflexo borrado.
class Metal : public Material {
public:
    Metal(const Cor& cor, double rugosidade) : cor_(cor), rugosidade_(rugosidade < 1 ? rugosidade : 1) {}
    bool espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const override;

private:
    Cor cor_;
    double rugosidade_;
};

// Vidro, agua, diamante: refrata (atravessa entortando) ou reflete.
// indice: 1.0 = ar, 1.33 = agua, 1.5 = vidro, 2.4 = diamante
class Vidro : public Material {
public:
    explicit Vidro(double indiceDeRefracao) : indice_(indiceDeRefracao) {}
    bool espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const override;

private:
    double indice_;
};

// Material que emite luz propria (uma lampada)
class Luz : public Material {
public:
    explicit Luz(const Cor& cor) : cor_(cor) {}
    bool espalhar(const Raio&, const Impacto&, Cor&, Raio&) const override { return false; }
    Cor emitida() const override { return cor_; }

private:
    Cor cor_;
};
