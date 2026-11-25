#include "BVH.h"

#include <algorithm>
#include <stdexcept>

NoBVH::NoBVH(std::vector<std::shared_ptr<Objeto>> objetos) : NoBVH(objetos, 0, objetos.size()) {}

NoBVH::NoBVH(std::vector<std::shared_ptr<Objeto>>& objetos, size_t inicio, size_t fim) {
    if (inicio >= fim) throw std::invalid_argument("BVH precisa de pelo menos um objeto.");

    // Caixa que envolve todos os objetos deste no
    caixa_ = objetos[inicio]->caixa();
    for (size_t i = inicio + 1; i < fim; i++) caixa_ = Caixa(caixa_, objetos[i]->caixa());

    size_t quantidade = fim - inicio;
    if (quantidade == 1) {
        esquerda_ = direita_ = objetos[inicio];
        return;
    }
    if (quantidade == 2) {
        esquerda_ = objetos[inicio];
        direita_ = objetos[inicio + 1];
        return;
    }

    // Ordena pelo eixo mais comprido e divide ao meio
    int eixo = caixa_.eixoMaisLongo();
    std::sort(objetos.begin() + static_cast<long>(inicio), objetos.begin() + static_cast<long>(fim),
              [eixo](const std::shared_ptr<Objeto>& a, const std::shared_ptr<Objeto>& b) {
                  return a->caixa().eixo(eixo).min < b->caixa().eixo(eixo).min;
              });

    size_t meio = inicio + quantidade / 2;
    esquerda_ = std::shared_ptr<NoBVH>(new NoBVH(objetos, inicio, meio));
    direita_ = std::shared_ptr<NoBVH>(new NoBVH(objetos, meio, fim));
}

bool NoBVH::atinge(const Raio& r, Intervalo t, Impacto& impacto) const {
    if (!caixa_.atinge(r, t)) return false;  // o raio nem passa perto: descarta tudo

    bool acertouEsquerda = esquerda_->atinge(r, t, impacto);
    // Se acertou a esquerda, a direita so interessa se estiver mais perto
    bool acertouDireita = direita_->atinge(r, Intervalo(t.min, acertouEsquerda ? impacto.t : t.max), impacto);
    return acertouEsquerda || acertouDireita;
}
