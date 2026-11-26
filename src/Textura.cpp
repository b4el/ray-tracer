#include "Textura.h"

#include <cmath>
#include <stdexcept>

#include "stb_image.h"

Xadrez::Xadrez(const Cor& par, const Cor& impar, double escala)
    : par_(par), impar_(impar), inverso_(1.0 / (escala > 0 ? escala : 1.0)) {}

Cor Xadrez::valor(double, double, const Ponto3& p) const {
    // Em qual "cubo" do xadrez o ponto cai? Soma dos indices par = uma cor, impar = outra.
    // O pequeno deslocamento evita listras num chao em y = 0: por arredondamento, um ponto
    // do chao pode sair como -0.0000000001 e cair no "cubo" de baixo.
    constexpr double desloc = 1e-6;
    const auto x = static_cast<long long>(std::floor(inverso_ * p.x + desloc));
    const auto y = static_cast<long long>(std::floor(inverso_ * p.y + desloc));
    const auto z = static_cast<long long>(std::floor(inverso_ * p.z + desloc));
    return ((x + y + z) % 2 == 0) ? par_ : impar_;
}

TexturaImagem::TexturaImagem(const std::string& caminho) {
    int canais = 0;
    unsigned char* dados = stbi_load(caminho.c_str(), &largura_, &altura_, &canais, 3);
    if (dados == nullptr) throw std::runtime_error("Nao foi possivel ler a imagem " + caminho);

    // Imagens comuns estao em sRGB (com gama). Convertemos para cor linear, que e o que o
    // ray tracer usa nas contas; a correcao gama e reaplicada ao salvar a imagem final.
    const size_t total = static_cast<size_t>(largura_) * static_cast<size_t>(altura_);
    pixels_.resize(total);
    for (size_t i = 0; i < total; i++) {
        auto linear = [&](int c) {
            double v = dados[i * 3 + static_cast<size_t>(c)] / 255.0;
            return v * v;  // inverso do sqrt usado em Imagem::paraByte
        };
        pixels_[i] = {linear(0), linear(1), linear(2)};
    }
    stbi_image_free(dados);
}

Cor TexturaImagem::valor(double u, double v, const Ponto3&) const {
    // Repete a imagem fora de [0, 1] e inverte v (o topo da imagem e v = 1)
    u -= std::floor(u);
    v = 1.0 - (v - std::floor(v));
    int i = static_cast<int>(u * largura_);
    int j = static_cast<int>(v * altura_);
    if (i >= largura_) i = largura_ - 1;
    if (j >= altura_) j = altura_ - 1;
    return pixels_[static_cast<size_t>(j) * static_cast<size_t>(largura_) + static_cast<size_t>(i)];
}
