#pragma once

#include <cmath>

// Vetor de 3 dimensoes. Usado para posicoes (Ponto3), direcoes e cores (Cor).
struct Vec3 {
    double x = 0, y = 0, z = 0;

    constexpr Vec3() = default;
    constexpr Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    constexpr Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator*=(double t) { x *= t; y *= t; z *= t; return *this; }
    Vec3& operator/=(double t) { return *this *= 1.0 / t; }

    double comprimentoAoQuadrado() const { return x * x + y * y + z * z; }
    double comprimento() const { return std::sqrt(comprimentoAoQuadrado()); }

    // true se o vetor e praticamente zero (evita direcoes degeneradas)
    bool quaseZero() const {
        constexpr double e = 1e-8;
        return std::fabs(x) < e && std::fabs(y) < e && std::fabs(z) < e;
    }
};

using Ponto3 = Vec3;
using Cor = Vec3;

constexpr Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
constexpr Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
constexpr Vec3 operator*(const Vec3& a, const Vec3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
constexpr Vec3 operator*(double t, const Vec3& v) { return {t * v.x, t * v.y, t * v.z}; }
constexpr Vec3 operator*(const Vec3& v, double t) { return t * v; }
constexpr Vec3 operator/(const Vec3& v, double t) { return (1.0 / t) * v; }

// Produto escalar: mede o quanto dois vetores "apontam para o mesmo lado"
constexpr double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

// Produto vetorial: devolve um vetor perpendicular aos dois
constexpr Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline Vec3 normalizar(const Vec3& v) { return v / v.comprimento(); }

// Reflexao de um raio numa superficie com normal n (como um espelho)
constexpr Vec3 refletir(const Vec3& v, const Vec3& n) { return v - 2 * dot(v, n) * n; }

// Refracao pela Lei de Snell (luz entrando/saindo do vidro).
// eta = indice do meio de origem / indice do meio de destino
inline Vec3 refratar(const Vec3& v, const Vec3& n, double eta) {
    double cosTheta = std::fmin(dot(-v, n), 1.0);
    Vec3 perpendicular = eta * (v + cosTheta * n);
    Vec3 paralelo = -std::sqrt(std::fabs(1.0 - perpendicular.comprimentoAoQuadrado())) * n;
    return perpendicular + paralelo;
}
