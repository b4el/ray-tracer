// ETAPA 1 - Sua primeira imagem
// Objetivo: entender que uma imagem e so uma grade de pixels, e cada pixel
// e uma cor com 3 numeros: vermelho (R), verde (G) e azul (B).
//
// Resultado: etapa1.bmp, um degrade colorido.

#include <cstdint>
#include <iostream>
#include <vector>

#include "bmp.h"

int main() {
    const int largura = 256;
    const int altura = 256;
    std::vector<uint8_t> pixels(largura * altura * 3);

    for (int y = 0; y < altura; y++) {
        for (int x = 0; x < largura; x++) {
            // Valores de 0.0 a 1.0 conforme a posicao do pixel
            double r = double(x) / (largura - 1);  // mais vermelho para a direita
            double g = double(y) / (altura - 1);   // mais verde para baixo
            double b = 0.25;

            size_t i = (size_t(y) * largura + x) * 3;
            pixels[i] = uint8_t(255.999 * r);
            pixels[i + 1] = uint8_t(255.999 * g);
            pixels[i + 2] = uint8_t(255.999 * b);
        }
    }

    salvarBMP("etapa1.bmp", largura, altura, pixels);
    std::cout << "Imagem salva em etapa1.bmp\n";
}

// DESAFIOS
// 1. Deixe o canto inferior direito branco.
// 2. Faca um degrade so em tons de cinza (r = g = b).
// 3. Desenhe um quadrado preto no meio da imagem.
