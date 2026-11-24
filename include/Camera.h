#pragma once

#include "Raio.h"

struct ConfigCamera {
    Ponto3 posicao{0, 0, 0};    // onde a camera esta
    Ponto3 alvo{0, 0, -1};      // para onde ela olha
    Vec3 cima{0, 1, 0};         // qual direcao e "para cima"
    double campoDeVisao = 90;   // angulo vertical, em graus (menor = mais zoom)
    double abertura = 0;        // tamanho da lente: 0 = tudo em foco
    double distanciaFoco = 1;   // distancia do plano que fica nitido
};

class Camera {
public:
    Camera(const ConfigCamera& config, double proporcao);

    // Raio que sai da camera passando pelo ponto (s, t) da imagem.
    // s e t vao de 0 a 1 (canto inferior esquerdo ate o superior direito).
    Raio gerarRaio(double s, double t) const;

private:
    Ponto3 origem_;
    Ponto3 cantoInferiorEsquerdo_;
    Vec3 horizontal_, vertical_;
    Vec3 u_, v_;  // eixos da camera (direita e cima)
    double raioDaLente_;
};
