#include "Material.h"

#include <cmath>

#include "Aleatorio.h"

bool Difuso::espalhar(const Raio&, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const {
    // Distribuicao "lambertiana": normal + direcao aleatoria
    Vec3 direcao = impacto.normal + aleatorio::vetorUnitario();
    if (direcao.quaseZero()) direcao = impacto.normal;  // caso raro: os dois se anularam

    espalhado = {impacto.ponto, direcao};
    atenuacao = textura_->valor(impacto.u, impacto.v, impacto.ponto);
    return true;
}

bool Metal::espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const {
    Vec3 refletido = normalizar(refletir(entrada.direcao, impacto.normal));
    refletido = refletido + rugosidade_ * aleatorio::vetorUnitario();

    espalhado = {impacto.ponto, refletido};
    atenuacao = cor_;
    return dot(espalhado.direcao, impacto.normal) > 0;  // nao pode ir "para dentro" do metal
}

namespace {
// Aproximacao de Schlick: quanto mais rasante o angulo, mais o vidro reflete
double refletancia(double cosseno, double indice) {
    double r0 = (1 - indice) / (1 + indice);
    r0 = r0 * r0;
    return r0 + (1 - r0) * std::pow(1 - cosseno, 5);
}
}  // namespace

bool Vidro::espalhar(const Raio& entrada, const Impacto& impacto, Cor& atenuacao, Raio& espalhado) const {
    atenuacao = {1.0, 1.0, 1.0};  // vidro nao absorve cor
    double eta = impacto.faceDeFora ? (1.0 / indice_) : indice_;

    Vec3 direcao = normalizar(entrada.direcao);
    double cosTheta = std::fmin(dot(-direcao, impacto.normal), 1.0);
    double senTheta = std::sqrt(1.0 - cosTheta * cosTheta);

    // Reflexao interna total: o raio nao consegue sair, entao reflete
    bool naoPodeRefratar = eta * senTheta > 1.0;

    Vec3 saida = (naoPodeRefratar || refletancia(cosTheta, eta) > aleatorio::real())
                     ? refletir(direcao, impacto.normal)
                     : refratar(direcao, impacto.normal, eta);

    espalhado = {impacto.ponto, saida};
    return true;
}
