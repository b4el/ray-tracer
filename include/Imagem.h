#pragma once

#include <string>
#include <vector>

#include "Vec3.h"

// Guarda os pixels da imagem (em cor linear) e salva em arquivo.
class Imagem {
public:
    Imagem(int largura, int altura);

    int largura() const { return largura_; }
    int altura() const { return altura_; }

    // (0, 0) e o canto SUPERIOR esquerdo
    void definir(int x, int y, const Cor& cor) { pixels_[static_cast<size_t>(y) * largura_ + x] = cor; }
    const Cor& obter(int x, int y) const { return pixels_[static_cast<size_t>(y) * largura_ + x]; }

    // Escolhe o formato pela extensao: .bmp (abre direto no Windows) ou .ppm
    void salvar(const std::string& caminho) const;
    void salvarBMP(const std::string& caminho) const;
    void salvarPPM(const std::string& caminho) const;

    // Converte uma cor linear (0..1+) em um byte 0..255 com correcao gama
    static unsigned char paraByte(double componente);

private:
    int largura_, altura_;
    std::vector<Cor> pixels_;
};
