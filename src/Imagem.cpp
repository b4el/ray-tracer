#include "Imagem.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <stdexcept>

Imagem::Imagem(int largura, int altura) : largura_(largura), altura_(altura) {
    if (largura <= 0 || altura <= 0) throw std::invalid_argument("Tamanho de imagem invalido.");
    pixels_.resize(static_cast<size_t>(largura) * altura);
}

unsigned char Imagem::paraByte(double c) {
    if (!(c > 0)) return 0;  // tambem trata NaN
    c = std::sqrt(c);        // correcao gama 2.0: o olho percebe brilho de forma nao linear
    if (c > 0.999) c = 0.999;
    return static_cast<unsigned char>(256 * c);
}

void Imagem::salvar(const std::string& caminho) const {
    auto terminaCom = [&](const std::string& fim) {
        return caminho.size() >= fim.size() && caminho.compare(caminho.size() - fim.size(), fim.size(), fim) == 0;
    };
    if (terminaCom(".ppm"))
        salvarPPM(caminho);
    else
        salvarBMP(caminho);
}

// PPM: o formato de imagem mais simples que existe (texto puro)
void Imagem::salvarPPM(const std::string& caminho) const {
    std::ofstream arq(caminho);
    if (!arq) throw std::runtime_error("Nao foi possivel criar " + caminho);
    arq << "P3\n" << largura_ << ' ' << altura_ << "\n255\n";
    for (int y = 0; y < altura_; y++)
        for (int x = 0; x < largura_; x++) {
            const Cor& c = obter(x, y);
            arq << int(paraByte(c.x)) << ' ' << int(paraByte(c.y)) << ' ' << int(paraByte(c.z)) << '\n';
        }
}

namespace {
void escrever16(std::ofstream& f, uint16_t v) {
    f.put(static_cast<char>(v & 0xFF));
    f.put(static_cast<char>((v >> 8) & 0xFF));
}
void escrever32(std::ofstream& f, uint32_t v) {
    for (int i = 0; i < 4; i++) f.put(static_cast<char>((v >> (8 * i)) & 0xFF));
}
}  // namespace

// BMP 24 bits: cabecalho de 54 bytes + pixels de baixo para cima, em ordem BGR,
// e cada linha completada ate um multiplo de 4 bytes.
void Imagem::salvarBMP(const std::string& caminho) const {
    std::ofstream f(caminho, std::ios::binary);
    if (!f) throw std::runtime_error("Nao foi possivel criar " + caminho);

    const uint32_t bytesPorLinha = (3u * static_cast<uint32_t>(largura_) + 3u) & ~3u;
    const uint32_t tamanhoPixels = bytesPorLinha * static_cast<uint32_t>(altura_);

    // Cabecalho do arquivo (14 bytes)
    f.put('B');
    f.put('M');
    escrever32(f, 54 + tamanhoPixels);  // tamanho total
    escrever32(f, 0);                   // reservado
    escrever32(f, 54);                  // onde comecam os pixels

    // Cabecalho da imagem (40 bytes)
    escrever32(f, 40);
    escrever32(f, static_cast<uint32_t>(largura_));
    escrever32(f, static_cast<uint32_t>(altura_));
    escrever16(f, 1);   // planos
    escrever16(f, 24);  // bits por pixel
    escrever32(f, 0);   // sem compressao
    escrever32(f, tamanhoPixels);
    escrever32(f, 2835);  // 72 DPI
    escrever32(f, 2835);
    escrever32(f, 0);
    escrever32(f, 0);

    const uint32_t preenchimento = bytesPorLinha - 3u * static_cast<uint32_t>(largura_);
    for (int y = altura_ - 1; y >= 0; y--) {
        for (int x = 0; x < largura_; x++) {
            const Cor& c = obter(x, y);
            f.put(static_cast<char>(paraByte(c.z)));
            f.put(static_cast<char>(paraByte(c.y)));
            f.put(static_cast<char>(paraByte(c.x)));
        }
        for (uint32_t i = 0; i < preenchimento; i++) f.put(0);
    }
}
