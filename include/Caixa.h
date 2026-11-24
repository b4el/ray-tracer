#pragma once

#include "Intervalo.h"
#include "Raio.h"

// Caixa alinhada aos eixos (AABB). Envolve um objeto e serve para descartar
// rapidamente raios que nem passam perto dele (base da BVH).
struct Caixa {
    Intervalo x, y, z;

    Caixa() = default;
    Caixa(const Intervalo& x, const Intervalo& y, const Intervalo& z) : x(x), y(y), z(z) { garantirEspessura(); }

    // Caixa definida por dois cantos opostos
    Caixa(const Ponto3& a, const Ponto3& b)
        : x(a.x <= b.x ? Intervalo(a.x, b.x) : Intervalo(b.x, a.x)),
          y(a.y <= b.y ? Intervalo(a.y, b.y) : Intervalo(b.y, a.y)),
          z(a.z <= b.z ? Intervalo(a.z, b.z) : Intervalo(b.z, a.z)) {
        garantirEspessura();
    }

    // Menor caixa que contem as duas
    Caixa(const Caixa& a, const Caixa& b) : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}

    const Intervalo& eixo(int n) const { return n == 0 ? x : (n == 1 ? y : z); }

    // Eixo em que a caixa e mais comprida (0 = x, 1 = y, 2 = z)
    int eixoMaisLongo() const {
        if (x.tamanho() > y.tamanho()) return x.tamanho() > z.tamanho() ? 0 : 2;
        return y.tamanho() > z.tamanho() ? 1 : 2;
    }

    // Metodo das "fatias" (slab): o raio precisa estar dentro da caixa
    // nos tres eixos ao mesmo tempo.
    bool atinge(const Raio& r, Intervalo t) const {
        const double origem[3] = {r.origem.x, r.origem.y, r.origem.z};
        const double direcao[3] = {r.direcao.x, r.direcao.y, r.direcao.z};

        for (int a = 0; a < 3; a++) {
            const Intervalo& ax = eixo(a);
            const double inv = 1.0 / direcao[a];
            double t0 = (ax.min - origem[a]) * inv;
            double t1 = (ax.max - origem[a]) * inv;
            if (t0 > t1) { double tmp = t0; t0 = t1; t1 = tmp; }
            if (t0 > t.min) t.min = t0;
            if (t1 < t.max) t.max = t1;
            if (t.max <= t.min) return false;
        }
        return true;
    }

private:
    // Evita caixas com espessura zero (ex.: um plano), que dariam erro numerico
    void garantirEspessura() {
        constexpr double minimo = 0.0001;
        if (x.tamanho() < minimo) x = x.expandir(minimo);
        if (y.tamanho() < minimo) y = y.expandir(minimo);
        if (z.tamanho() < minimo) z = z.expandir(minimo);
    }
};
