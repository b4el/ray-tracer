#pragma once

#include <memory>

#include "Material.h"
#include "Objeto.h"

class Esfera : public Objeto {
public:
    Esfera(const Ponto3& centro, double raio, std::shared_ptr<Material> material);

    bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const override;
    Caixa caixa() const override { return caixa_; }

private:
    Ponto3 centro_;
    double raio_;
    std::shared_ptr<Material> material_;  // varias esferas podem dividir o mesmo material
    Caixa caixa_;
};
