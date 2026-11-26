#pragma once

#include <memory>

#include "Material.h"
#include "Objeto.h"

// Quadrilatero (paralelogramo) definido por um canto e dois lados:
//
//        canto + v -------- canto + u + v
//           |                    |
//         canto ------------- canto + u
//
// Serve para chao, paredes e paineis. Um plano infinito nao teria uma caixa
// envolvente finita (e a BVH precisa de uma), entao usamos quadrilateros grandes.
class Quadrilatero : public Objeto {
public:
    Quadrilatero(const Ponto3& canto, const Vec3& u, const Vec3& v, std::shared_ptr<Material> material);

    bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const override;
    Caixa caixa() const override { return caixa_; }

private:
    Ponto3 canto_;
    Vec3 u_, v_;
    Vec3 normal_;  // perpendicular ao quadrilatero
    double d_;     // o plano e: dot(normal, p) = d
    Vec3 w_;       // ajuda a calcular as coordenadas (alfa, beta) do ponto atingido
    std::shared_ptr<Material> material_;
    Caixa caixa_;
};
