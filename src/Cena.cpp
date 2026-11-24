#include "Cena.h"

#include <utility>

void Cena::adicionar(std::shared_ptr<Objeto> objeto) {
    caixa_ = objetos_.empty() ? objeto->caixa() : Caixa(caixa_, objeto->caixa());
    objetos_.push_back(std::move(objeto));
}

bool Cena::atinge(const Raio& r, Intervalo t, Impacto& impacto) const {
    Impacto temp;
    bool atingiuAlgo = false;
    double maisProximo = t.max;

    for (const auto& objeto : objetos_) {
        // Diminui o limite a cada acerto: so aceita objetos MAIS perto
        if (objeto->atinge(r, Intervalo(t.min, maisProximo), temp)) {
            atingiuAlgo = true;
            maisProximo = temp.t;
            impacto = temp;
        }
    }
    return atingiuAlgo;
}
