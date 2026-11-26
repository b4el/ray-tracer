#include "Quadrilatero.h"

#include <cmath>
#include <stdexcept>
#include <utility>

Quadrilatero::Quadrilatero(const Ponto3& canto, const Vec3& u, const Vec3& v, std::shared_ptr<Material> material)
    : canto_(canto), u_(u), v_(v), material_(std::move(material)) {
    Vec3 n = cross(u, v);
    if (n.quaseZero()) throw std::invalid_argument("Quadrilatero com lados paralelos (area zero).");
    normal_ = normalizar(n);
    d_ = dot(normal_, canto);
    w_ = n / dot(n, n);

    // Caixa que envolve os 4 cantos
    caixa_ = Caixa(Caixa(canto, canto + u + v), Caixa(canto + u, canto + v));
}

bool Quadrilatero::atinge(const Raio& r, Intervalo t, Impacto& impacto) const {
    // 1) Onde o raio cruza o plano do quadrilatero?
    double denominador = dot(normal_, r.direcao);
    if (std::fabs(denominador) < 1e-8) return false;  // raio paralelo ao plano

    double distancia = (d_ - dot(normal_, r.origem)) / denominador;
    if (!t.contem(distancia)) return false;

    // 2) Esse ponto esta dentro do quadrilatero? Escrevemos o ponto como
    //    canto + alfa*u + beta*v; ele esta dentro se alfa e beta estao entre 0 e 1.
    Ponto3 ponto = r.em(distancia);
    Vec3 relativo = ponto - canto_;
    double alfa = dot(w_, cross(relativo, v_));
    double beta = dot(w_, cross(u_, relativo));
    if (alfa < 0 || alfa > 1 || beta < 0 || beta > 1) return false;

    impacto.t = distancia;
    impacto.ponto = ponto;
    impacto.u = alfa;  // alfa e beta ja sao as coordenadas da textura
    impacto.v = beta;
    impacto.material = material_.get();
    impacto.definirNormal(r, normal_);
    return true;
}
