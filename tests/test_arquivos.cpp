// Testes de arquivos: leitura de .obj e escrita de imagens BMP

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <stdexcept>

#include "Imagem.h"
#include "Malha.h"
#include "Material.h"

TEST(LeituraOBJ, PoligonosNormaisEIndicesNegativos) {
    DadosMalha m = lerOBJ(
        "# comentario\n"
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
        "vn 0 0 1\n"
        "o qualquer\n"
        "f 1//1 2//1 3//1 4//1\n"  // quadrado: vira 2 triangulos
        "f -4 -3 -2\n");           // indices negativos contam do fim
    EXPECT_EQ(m.vertices.size(), 4u);
    EXPECT_EQ(m.normais.size(), 1u);
    ASSERT_EQ(m.faces.size(), 3u);

    // Segundo triangulo do quadrado: (0, 2, 3)
    EXPECT_EQ(m.faces[1].v[0], 0);
    EXPECT_EQ(m.faces[1].v[1], 2);
    EXPECT_EQ(m.faces[1].v[2], 3);
    EXPECT_EQ(m.faces[0].n[0], 0);

    // Face com indices negativos e sem normal
    EXPECT_EQ(m.faces[2].v[0], 0);
    EXPECT_EQ(m.faces[2].n[0], -1);
}

TEST(LeituraOBJ, FormatoComCoordenadasDeTextura) {
    DadosMalha m = lerOBJ("v 0 0 0\nv 1 0 0\nv 0 1 0\nvt 0 0\nvn 0 0 1\nf 1/1/1 2/1/1 3/1/1\n");
    ASSERT_EQ(m.faces.size(), 1u);
    EXPECT_EQ(m.faces[0].n[2], 0);
}

TEST(LeituraOBJ, ErrosClaros) {
    EXPECT_THROW(lerOBJ("v 0 0 0\nf 1 2 3\n"), std::runtime_error);  // indice que nao existe
    EXPECT_THROW(lerOBJ("v 0 0\n"), std::runtime_error);             // vertice incompleto
    EXPECT_THROW(lerOBJ("v 0 0 0\n"), std::runtime_error);           // sem nenhuma face
    EXPECT_THROW(carregarOBJ("arquivo_que_nao_existe.obj"), std::runtime_error);
}

TEST(LeituraOBJ, CriarTriangulosIgnoraDegenerados) {
    DadosMalha m = lerOBJ("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 2 0 0\nf 1 2 3\nf 1 2 4\n");  // a 2a face tem area zero
    auto tris = criarTriangulos(m, Vec3(0, 0, -5), 2.0, std::make_shared<Difuso>(Cor(1, 1, 1)));
    EXPECT_EQ(tris.size(), 1u);
}

TEST(Imagem, CorParaByteComCorrecaoGama) {
    EXPECT_EQ(Imagem::paraByte(0), 0);
    EXPECT_EQ(Imagem::paraByte(1), 255);
    EXPECT_EQ(Imagem::paraByte(-5), 0);
    EXPECT_EQ(Imagem::paraByte(0.25), 128);  // sqrt(0.25) = 0.5
    EXPECT_EQ(Imagem::paraByte(50), 255);    // valores acima de 1 sao limitados
}

TEST(Imagem, TamanhoDoArquivoBMP) {
    Imagem img(3, 2);  // largura 3 -> linha com 9 bytes + 3 de preenchimento = 12
    const char* nome = "teste_tmp.bmp";
    img.salvarBMP(nome);
    std::ifstream f(nome, std::ios::binary | std::ios::ate);
    ASSERT_TRUE(f.is_open());
    EXPECT_EQ(static_cast<long>(f.tellg()), 54 + 12 * 2);
    f.close();
    std::remove(nome);
}

TEST(Imagem, TamanhoInvalido) { EXPECT_THROW(Imagem(0, 10), std::invalid_argument); }
