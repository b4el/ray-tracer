#pragma once

#include <memory>
#include <vector>

#include "Objeto.h"

// Bounding Volume Hierarchy: uma arvore de caixas.
// Em vez de testar o raio contra TODOS os objetos (O(n)), descemos pela arvore
// e descartamos grupos inteiros cujas caixas o raio nem toca (~O(log n)).
class NoBVH : public Objeto {
public:
    explicit NoBVH(std::vector<std::shared_ptr<Objeto>> objetos);

    bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const override;
    Caixa caixa() const override { return caixa_; }

private:
    NoBVH(std::vector<std::shared_ptr<Objeto>>& objetos, size_t inicio, size_t fim);

    std::shared_ptr<Objeto> esquerda_;
    std::shared_ptr<Objeto> direita_;
    Caixa caixa_;
};
