#pragma once

#include <memory>

#include "Material.h"
#include "Objeto.h"

// Triangulo: a forma basica de qualquer modelo 3D.
// Pode ter normais por vertice (sombreamento suave) ou uma normal so (faces planas).
class Triangulo : public Objeto {
public:
    // Faces planas: a normal sai do proprio triangulo
    Triangulo(const Ponto3& a, const Ponto3& b, const Ponto3& c, std::shared_ptr<Material> material);

    // Sombreamento suave: uma normal em cada vertice, interpolada no ponto atingido
    Triangulo(const Ponto3& a, const Ponto3& b, const Ponto3& c,
              const Vec3& na, const Vec3& nb, const Vec3& nc, std::shared_ptr<Material> material);

    bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const override;
    Caixa caixa() const override { return caixa_; }

private:
    Ponto3 a_, b_, c_;
    Vec3 na_, nb_, nc_;
    bool suave_;
    std::shared_ptr<Material> material_;
    Caixa caixa_;
};
