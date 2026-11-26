// Testes de texturas, quadrilateros e saida PNG

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <stdexcept>

#include "Esfera.h"
#include "Imagem.h"
#include "Material.h"
#include "Quadrilatero.h"
#include "Textura.h"

namespace {

const Intervalo kTudo(0.001, 1e9);

bool mesmaCor(const Cor& a, const Cor& b, double tolerancia = 1e-9) {
    return std::abs(a.x - b.x) < tolerancia && std::abs(a.y - b.y) < tolerancia && std::abs(a.z - b.z) < tolerancia;
}

}  // namespace

TEST(Xadrez, AlternaAsCoresACadaQuadrado) {
    Xadrez x(Cor(1, 1, 1), Cor(0, 0, 0), 1.0);
    Cor a = x.valor(0, 0, Ponto3(0.5, 0, 0.5));
    Cor b = x.valor(0, 0, Ponto3(1.5, 0, 0.5));  // um quadrado para o lado
    Cor c = x.valor(0, 0, Ponto3(1.5, 0, 1.5));  // diagonal: volta para a primeira cor
    EXPECT_FALSE(mesmaCor(a, b));
    EXPECT_TRUE(mesmaCor(a, c));
}

TEST(Xadrez, ChaoEmYZeroNaoTemListras) {
    // Um ponto do chao pode sair como y = -1e-12 por arredondamento: tem que dar a mesma cor
    Xadrez x(Cor(1, 1, 1), Cor(0, 0, 0), 1.0);
    EXPECT_TRUE(mesmaCor(x.valor(0, 0, Ponto3(0.5, 0.0, 0.5)), x.valor(0, 0, Ponto3(0.5, -1e-12, 0.5))));
}

TEST(Esfera, CoordenadasDaTextura) {
    Esfera e(Ponto3(0, 0, 0), 1, std::make_shared<Difuso>(Cor(1, 1, 1)));
    Impacto imp;
    ASSERT_TRUE(e.atinge(Raio{{0, 5, 0}, {0, -1, 0}}, kTudo, imp));  // polo de cima
    EXPECT_NEAR(imp.v, 1.0, 1e-9);
    ASSERT_TRUE(e.atinge(Raio{{0, -5, 0}, {0, 1, 0}}, kTudo, imp));  // polo de baixo
    EXPECT_NEAR(imp.v, 0.0, 1e-9);
    ASSERT_TRUE(e.atinge(Raio{{5, 0, 0}, {-1, 0, 0}}, kTudo, imp));  // equador
    EXPECT_NEAR(imp.v, 0.5, 1e-9);
    EXPECT_GE(imp.u, 0.0);
    EXPECT_LE(imp.u, 1.0);
}

TEST(Quadrilatero, AtingeDentroEErraFora) {
    // Quadrado de 2x2 no chao (y = 0), de (-1, 0, -1) a (1, 0, 1)
    Quadrilatero q(Ponto3(-1, 0, -1), Vec3(2, 0, 0), Vec3(0, 0, 2), std::make_shared<Difuso>(Cor(1, 1, 1)));
    Impacto imp;
    ASSERT_TRUE(q.atinge(Raio{{0, 3, 0}, {0, -1, 0}}, kTudo, imp));
    EXPECT_NEAR(imp.t, 3, 1e-9);
    EXPECT_NEAR(imp.normal.y, 1, 1e-9);  // virada para quem olha de cima
    EXPECT_NEAR(imp.u, 0.5, 1e-9);       // centro do quadrado
    EXPECT_NEAR(imp.v, 0.5, 1e-9);

    EXPECT_FALSE(q.atinge(Raio{{5, 3, 0}, {0, -1, 0}}, kTudo, imp));  // fora do quadrado
    EXPECT_FALSE(q.atinge(Raio{{0, 3, 0}, {1, 0, 0}}, kTudo, imp));   // paralelo ao plano
}

TEST(Quadrilatero, LadosParalelosLancamExcecao) {
    auto mat = std::make_shared<Difuso>(Cor(1, 1, 1));
    EXPECT_THROW(Quadrilatero(Ponto3(0, 0, 0), Vec3(1, 0, 0), Vec3(2, 0, 0), mat), std::invalid_argument);
}

TEST(ImagemPNG, SalvaEUmPNGDeVerdade) {
    Imagem img(4, 3);
    const char* nome = "teste_tmp.png";
    img.salvarPNG(nome);
    std::ifstream f(nome, std::ios::binary);
    ASSERT_TRUE(f.is_open());
    unsigned char assinatura[8] = {};
    f.read(reinterpret_cast<char*>(assinatura), 8);
    f.close();
    std::remove(nome);
    const unsigned char esperado[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
    for (int i = 0; i < 8; i++) EXPECT_EQ(assinatura[i], esperado[i]);
}

TEST(ImagemPNG, FormatoDesconhecidoLancaExcecao) { EXPECT_THROW(Imagem(2, 2).salvar("x.jpg"), std::invalid_argument); }

TEST(TexturaImagem, LeOQueFoiSalvo) {
    // Salva uma imagem 2x1 (vermelho, azul) em PNG e le de volta como textura
    Imagem img(2, 1);
    img.definir(0, 0, Cor(1, 0, 0));
    img.definir(1, 0, Cor(0, 0, 1));
    const char* nome = "teste_textura_tmp.png";
    img.salvarPNG(nome);

    TexturaImagem t(nome);
    std::remove(nome);
    EXPECT_EQ(t.largura(), 2);
    EXPECT_EQ(t.altura(), 1);
    Cor esquerda = t.valor(0.25, 0.5, Ponto3());
    Cor direita = t.valor(0.75, 0.5, Ponto3());
    EXPECT_GT(esquerda.x, 0.9);
    EXPECT_LT(esquerda.z, 0.1);
    EXPECT_GT(direita.z, 0.9);
    EXPECT_LT(direita.x, 0.1);
}

TEST(TexturaImagem, ArquivoQueNaoExisteLancaExcecao) {
    EXPECT_THROW(TexturaImagem("nao_existe.png"), std::runtime_error);
}
