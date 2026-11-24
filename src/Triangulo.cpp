#include "Triangulo.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {
Caixa caixaDe(const Ponto3& a, const Ponto3& b, const Ponto3& c) {
    Ponto3 menor{std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y}), std::min({a.z, b.z, c.z})};
    Ponto3 maior{std::max({a.x, b.x, c.x}), std::max({a.y, b.y, c.y}), std::max({a.z, b.z, c.z})};
    return Caixa(menor, maior);
}
}  // namespace

Triangulo::Triangulo(const Ponto3& a, const Ponto3& b, const Ponto3& c, std::shared_ptr<Material> material)
    : a_(a), b_(b), c_(c), suave_(false), material_(std::move(material)), caixa_(caixaDe(a, b, c)) {
    Vec3 n = normalizar(cross(b - a, c - a));
    na_ = nb_ = nc_ = n;
}

Triangulo::Triangulo(const Ponto3& a, const Ponto3& b, const Ponto3& c,
                     const Vec3& na, const Vec3& nb, const Vec3& nc, std::shared_ptr<Material> material)
    : a_(a), b_(b), c_(c), na_(normalizar(na)), nb_(normalizar(nb)), nc_(normalizar(nc)), suave_(true),
      material_(std::move(material)), caixa_(caixaDe(a, b, c)) {}

// Algoritmo de Moller-Trumbore: resolve de uma vez a distancia t e as
// coordenadas baricentricas (u, v) do ponto dentro do triangulo.
bool Triangulo::atinge(const Raio& r, Intervalo t, Impacto& impacto) const {
    const Vec3 aresta1 = b_ - a_;
    const Vec3 aresta2 = c_ - a_;
    const Vec3 p = cross(r.direcao, aresta2);
    const double det = dot(aresta1, p);
    if (std::fabs(det) < 1e-12) return false;  // raio paralelo ao triangulo

    const double invDet = 1.0 / det;
    const Vec3 s = r.origem - a_;
    const double u = dot(s, p) * invDet;
    if (u < 0.0 || u > 1.0) return false;

    const Vec3 q = cross(s, aresta1);
    const double v = dot(r.direcao, q) * invDet;
    if (v < 0.0 || u + v > 1.0) return false;

    const double distancia = dot(aresta2, q) * invDet;
    if (!t.envolve(distancia)) return false;

    impacto.t = distancia;
    impacto.ponto = r.em(distancia);
    // Faces planas: na_ == nb_ == nc_, entao a interpolacao devolve a propria normal
    Vec3 normal = suave_ ? normalizar((1.0 - u - v) * na_ + u * nb_ + v * nc_) : na_;
    impacto.definirNormal(r, normal);
    impacto.material = material_.get();
    return true;
}
