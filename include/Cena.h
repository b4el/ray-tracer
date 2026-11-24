#pragma once

#include <memory>
#include <vector>

#include "Objeto.h"

// Uma lista de objetos que tambem e um Objeto (padrao "Composite").
// Testa o raio contra todos e guarda o impacto mais proximo.
class Cena : public Objeto {
public:
    void adicionar(std::shared_ptr<Objeto> objeto);

    bool atinge(const Raio& r, Intervalo t, Impacto& impacto) const override;
    Caixa caixa() const override { return caixa_; }

    const std::vector<std::shared_ptr<Objeto>>& objetos() const { return objetos_; }
    size_t tamanho() const { return objetos_.size(); }

private:
    std::vector<std::shared_ptr<Objeto>> objetos_;
    Caixa caixa_;
};
