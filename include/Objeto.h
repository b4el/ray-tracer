#pragma once

#include <memory>

#include "Caixa.h"
#include "Intervalo.h"
#include "Raio.h"

class Material;

// Informacoes sobre o ponto onde um raio bateu num objeto
struct Impacto {
    Ponto3 ponto;
    Vec3 normal;             // sempre aponta CONTRA o raio
    double t = 0;            // distancia ao longo do raio
    bool faceDeFora = true;  // o raio veio de fora do objeto?
    double u = 0, v = 0;     // coordenadas na superficie, de 0 a 1 (usadas pelas texturas de imagem)
    const Material* material = nullptr;

    // normalParaFora deve ter comprimento 1
    void definirNormal(const Raio& r, const Vec3& normalParaFora) {
        faceDeFora = dot(r.direcao, normalParaFora) < 0;
        normal = faceDeFora ? normalParaFora : -normalParaFora;
    }
};

// Interface de qualquer coisa que um raio pode atingir (classe abstrata)
class Objeto {
public:
    virtual ~Objeto() = default;

    // Devolve true se o raio atinge o objeto com t dentro do intervalo.
    // Nesse caso, preenche 'impacto'.
    virtual bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const = 0;

    // Caixa que envolve o objeto inteiro (usada pela BVH)
    virtual Caixa caixa() const = 0;
};
