#pragma once

#include "Vec3.h"

// Um raio e uma semirreta: P(t) = origem + t * direcao
struct Raio {
    Ponto3 origem;
    Vec3 direcao;

    Ponto3 em(double t) const { return origem + t * direcao; }
};
