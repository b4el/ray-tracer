#pragma once

#include <limits>

// Intervalo [min, max] de numeros reais
struct Intervalo {
    double min = +std::numeric_limits<double>::infinity();  // vazio por padrao
    double max = -std::numeric_limits<double>::infinity();

    constexpr Intervalo() = default;
    constexpr Intervalo(double min, double max) : min(min), max(max) {}

    // Menor intervalo que contem os dois
    constexpr Intervalo(const Intervalo& a, const Intervalo& b)
        : min(a.min < b.min ? a.min : b.min), max(a.max > b.max ? a.max : b.max) {}

    constexpr double tamanho() const { return max - min; }
    constexpr bool contem(double x) const { return min <= x && x <= max; }
    constexpr bool envolve(double x) const { return min < x && x < max; }
    constexpr double limitar(double x) const { return x < min ? min : (x > max ? max : x); }
    constexpr Intervalo expandir(double delta) const { return {min - delta / 2, max + delta / 2}; }
};
