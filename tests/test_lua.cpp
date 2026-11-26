// Testes das cenas em Lua: API, mensagens de erro e sandbox

#include <gtest/gtest.h>

#include <string>

#include "ScriptLua.h"

namespace {

// Roda um script que deveria falhar e devolve a mensagem de erro ("" se nao falhou)
std::string erroDoScript(const std::string& codigo) {
    try {
        lerCenaLua(codigo, "teste.lua");
    } catch (const std::exception& e) {
        return e.what();
    }
    return "";
}

}  // namespace

TEST(CenaLua, MontaACenaDescrita) {
    CenaPronta c = lerCenaLua(
        "esfera { centro = {0, -100, 0}, raio = 100, material = difuso {0.5, 0.5, 0.5} }\n"
        "for i = 1, 3 do\n"
        "  esfera { centro = {i, 1, 0}, raio = 0.5, material = metal({1, 1, 1}, 0.2) }\n"
        "end\n"
        "esfera { centro = {0, 1, 2}, raio = 0.5, material = vidro(1.33) }\n"
        "camera { posicao = {0, 2, 10}, alvo = {0, 1, 0}, campo_de_visao = 30 }\n"
        "ceu(false)\n",
        "teste.lua");

    ASSERT_EQ(c.esferas.size(), 5u);
    EXPECT_EQ(c.esferas[1].material.tipo, DescricaoMaterial::Tipo::Metal);
    EXPECT_DOUBLE_EQ(c.esferas[1].material.rugosidade, 0.2);
    EXPECT_DOUBLE_EQ(c.esferas[4].material.indice, 1.33);
    EXPECT_DOUBLE_EQ(c.camera.campoDeVisao, 30);
    EXPECT_DOUBLE_EQ(c.camera.posicao.z, 10);
    EXPECT_FALSE(c.ceu);
    ASSERT_NE(c.mundo, nullptr);

    // A cena montada funciona: um raio para baixo bate no chao
    Impacto imp;
    EXPECT_TRUE(c.mundo->atinge(Raio{{0, 5, 5}, {0, -1, 0}}, Intervalo(0.001, 1e9), imp));
}

TEST(CenaLua, ErroMostraArquivoELinha) {
    std::string e = erroDoScript(
        "esfera { centro = {0, 0, 0}, raio = 1, material = difuso {1, 1, 1} }\n"
        "esfera { centro = {0, 0, 0}, material = difuso {1, 1, 1} }\n");
    EXPECT_NE(e.find("teste.lua:2:"), std::string::npos) << e;
    EXPECT_NE(e.find("raio"), std::string::npos) << e;
}

TEST(CenaLua, ValidaOsCampos) {
    EXPECT_NE(erroDoScript("esfera { centro = {0, 0}, raio = 1, material = vidro() }").find("3 numeros"),
              std::string::npos);
    EXPECT_NE(erroDoScript("esfera { centro = {0, 0, 0}, raio = 1 }").find("material"), std::string::npos);
    EXPECT_NE(erroDoScript("esfera { centro = {0, 0, 0}, raio = -1, material = vidro() }").find("maior que zero"),
              std::string::npos);
    EXPECT_NE(erroDoScript("camera { posicao = {0, 0, 0} }").find("nenhum objeto"), std::string::npos);
    EXPECT_NE(erroDoScript("isso nao e lua").find("teste.lua:1:"), std::string::npos);  // erro de sintaxe
}

TEST(CenaLua, SandboxBloqueiaArquivosESistema) {
    EXPECT_NE(erroDoScript("os.execute('echo oi')").find("os"), std::string::npos);
    EXPECT_NE(erroDoScript("io.open('x.txt', 'w')").find("io"), std::string::npos);
    EXPECT_NE(erroDoScript("dofile('outro.lua')").find("dofile"), std::string::npos);
    EXPECT_NE(erroDoScript("require('socket')").find("require"), std::string::npos);
}

TEST(CenaLua, LacoInfinitoEInterrompido) {
    EXPECT_NE(erroDoScript("while true do end").find("demorou demais"), std::string::npos);
}

TEST(CenaLua, QuadChaoEXadrez) {
    CenaPronta c = lerCenaLua(
        "chao { altura = 0, material = difuso(xadrez({1, 1, 1}, {0, 0, 0}, 0.5)) }\n"
        "quad { canto = {0, 0, -2}, u = {2, 0, 0}, v = {0, 2, 0}, material = metal({1, 1, 1}, 0) }\n",
        "teste.lua");
    ASSERT_EQ(c.quads.size(), 2u);
    EXPECT_EQ(c.quads[0].material.textura.tipo, DescricaoTextura::Tipo::Xadrez);
    EXPECT_DOUBLE_EQ(c.quads[0].material.textura.escala, 0.5);
    EXPECT_DOUBLE_EQ(c.quads[0].material.textura.cor2.x, 0);
    EXPECT_EQ(c.quads[1].material.tipo, DescricaoMaterial::Tipo::Metal);
}

TEST(CenaLua, ImagemSoDaPastaDeTexturas) {
    // Nomes com pastas ou ".." sao recusados antes de tentar abrir qualquer arquivo
    for (const char* nome : {"../segredo.png", "C:/Windows/x.png", "sub/pasta.png", ""}) {
        std::string codigo =
            std::string("esfera { centro = {0, 0, 0}, raio = 1, material = difuso(imagem(\"") + nome + "\")) }";
        EXPECT_NE(erroDoScript(codigo).find("texturas/"), std::string::npos) << nome;
    }
}

TEST(CenaLua, QuadInvalido) {
    std::string e = erroDoScript("quad { canto = {0, 0, 0}, u = {1, 0, 0}, v = {2, 0, 0}, material = vidro() }");
    EXPECT_NE(e.find("paralelos"), std::string::npos) << e;
}
