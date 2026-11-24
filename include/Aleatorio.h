#pragma once

#include <random>

#include "Vec3.h"

// Numeros aleatorios rapidos e seguros para usar em varias threads:
// cada thread tem o seu proprio gerador (thread_local).
namespace aleatorio {

inline std::mt19937& gerador() {
    thread_local std::mt19937 g{std::random_device{}()};
    return g;
}

// Define a semente da thread atual (usado para gerar cenas sempre iguais)
inline void semente(unsigned s) { gerador().seed(s); }

// Numero em [0, 1)
inline double real() {
    thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gerador());
}

// Numero em [min, max)
inline double real(double min, double max) { return min + (max - min) * real(); }

inline Vec3 vetor(double min, double max) { return {real(min, max), real(min, max), real(min, max)}; }

// Direcao aleatoria com comprimento 1 (ponto sorteado na superficie de uma esfera)
inline Vec3 vetorUnitario() {
    while (true) {
        Vec3 p = vetor(-1, 1);
        double lenSq = p.comprimentoAoQuadrado();
        if (lenSq > 1e-160 && lenSq <= 1.0) return p / std::sqrt(lenSq);
    }
}

// Ponto aleatorio dentro de um disco de raio 1 (usado no desfoque da lente)
inline Vec3 noDisco() {
    while (true) {
        Vec3 p{real(-1, 1), real(-1, 1), 0};
        if (p.comprimentoAoQuadrado() < 1.0) return p;
    }
}

}  // namespace aleatorio
