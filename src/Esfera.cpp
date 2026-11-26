#include "Esfera.h"

#include <cmath>
#include <utility>

namespace {
// Converte um ponto da esfera de raio 1 em (u, v), como latitude e longitude:
// u da a volta em torno do eixo y; v vai de 0 (polo de baixo) a 1 (polo de cima).
void coordenadasDaEsfera(const Vec3& p, double& u, double& v) {
    constexpr double PI = 3.14159265358979323846;
    double theta = std::acos(-p.y);
    double phi = std::atan2(-p.z, p.x) + PI;
    u = phi / (2 * PI);
    v = theta / PI;
}
}  // namespace

Esfera::Esfera(const Ponto3& centro, double raio, std::shared_ptr<Material> material)
    : centro_(centro), raio_(std::fmax(0.0, raio)), material_(std::move(material)) {
    Vec3 r{raio_, raio_, raio_};
    caixa_ = Caixa(centro_ - r, centro_ + r);
}

// Um ponto P esta na esfera quando |P - centro|^2 = raio^2.
// Trocando P pelo raio (origem + t * direcao) sai uma equacao do 2o grau em t:
//   a*t^2 + b*t + c = 0
// Aqui usamos a versao simplificada com h = -b/2.
bool Esfera::atinge(const Raio& r, Intervalo t, Impacto& impacto) const {
    Vec3 oc = centro_ - r.origem;
    double a = r.direcao.comprimentoAoQuadrado();
    double h = dot(r.direcao, oc);
    double c = oc.comprimentoAoQuadrado() - raio_ * raio_;

    double delta = h * h - a * c;
    if (delta < 0) return false;  // o raio nao encosta na esfera

    double raizDelta = std::sqrt(delta);

    // Pega a raiz mais proxima que esteja dentro do intervalo aceito
    double raiz = (h - raizDelta) / a;
    if (!t.envolve(raiz)) {
        raiz = (h + raizDelta) / a;
        if (!t.envolve(raiz)) return false;
    }

    impacto.t = raiz;
    impacto.ponto = r.em(raiz);
    Vec3 normalParaFora = (impacto.ponto - centro_) / raio_;
    impacto.definirNormal(r, normalParaFora);
    coordenadasDaEsfera(normalParaFora, impacto.u, impacto.v);
    impacto.material = material_.get();
    return true;
}
