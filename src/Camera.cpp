#include "Camera.h"

#include <cmath>

#include "Aleatorio.h"

Camera::Camera(const ConfigCamera& c, double proporcao) {
    constexpr double PI = 3.14159265358979323846;
    double theta = c.campoDeVisao * PI / 180.0;
    double alturaTela = 2.0 * std::tan(theta / 2);
    double larguraTela = proporcao * alturaTela;

    // Monta um sistema de eixos proprio da camera
    Vec3 w = normalizar(c.posicao - c.alvo);  // para tras
    u_ = normalizar(cross(c.cima, w));        // para a direita
    v_ = cross(w, u_);                        // para cima

    origem_ = c.posicao;
    horizontal_ = c.distanciaFoco * larguraTela * u_;
    vertical_ = c.distanciaFoco * alturaTela * v_;
    cantoInferiorEsquerdo_ = origem_ - horizontal_ / 2 - vertical_ / 2 - c.distanciaFoco * w;
    raioDaLente_ = c.abertura / 2;
}

Raio Camera::gerarRaio(double s, double t) const {
    // Desfoque: o raio sai de um ponto aleatorio da lente, nao do centro exato
    Vec3 rd = raioDaLente_ * aleatorio::noDisco();
    Vec3 deslocamento = u_ * rd.x + v_ * rd.y;

    Ponto3 inicio = origem_ + deslocamento;
    return {inicio, cantoInferiorEsquerdo_ + s * horizontal_ + t * vertical_ - inicio};
}
