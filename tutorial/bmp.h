// bmp.h - salva uma imagem no formato BMP (abre direto no Windows).
// Usado por todas as etapas do tutorial. Nao precisa entender tudo agora:
// o importante e saber que cada pixel tem 3 valores (R, G, B) de 0 a 255.
#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// pixels: largura * altura * 3 bytes, na ordem R, G, B, linha por linha,
// comecando pelo canto SUPERIOR esquerdo.
inline bool salvarBMP(const std::string& nome, int largura, int altura, const std::vector<uint8_t>& pixels) {
    std::ofstream f(nome, std::ios::binary);
    if (!f) return false;

    auto u16 = [&](uint16_t v) { f.put(char(v & 0xFF)); f.put(char(v >> 8)); };
    auto u32 = [&](uint32_t v) { for (int i = 0; i < 4; i++) f.put(char((v >> (8 * i)) & 0xFF)); };

    uint32_t bytesPorLinha = (3u * largura + 3u) & ~3u;  // cada linha precisa ter multiplo de 4 bytes
    uint32_t tamanho = bytesPorLinha * altura;

    f.put('B'); f.put('M'); u32(54 + tamanho); u32(0); u32(54);           // cabecalho do arquivo
    u32(40); u32(largura); u32(altura); u16(1); u16(24); u32(0);          // cabecalho da imagem
    u32(tamanho); u32(2835); u32(2835); u32(0); u32(0);

    for (int y = altura - 1; y >= 0; y--) {  // BMP guarda de baixo para cima
        for (int x = 0; x < largura; x++) {
            size_t i = (size_t(y) * largura + x) * 3;
            f.put(char(pixels[i + 2])); f.put(char(pixels[i + 1])); f.put(char(pixels[i]));  // ordem B, G, R
        }
        for (uint32_t p = 3u * largura; p < bytesPorLinha; p++) f.put(0);
    }
    return true;
}
