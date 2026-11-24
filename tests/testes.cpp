// Testes automaticos (sem bibliotecas externas). Rode com: ctest

#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "BVH.h"
#include "Camera.h"
#include "Cena.h"
#include "Esfera.h"
#include "Imagem.h"
#include "Malha.h"
#include "Material.h"
#include "ScriptLua.h"
#include <string>
#include "Triangulo.h"

static int falhas = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            std::cerr << "FALHOU: " #cond " (linha " << __LINE__ << ")\n";   \
            falhas++;                                                        \
        }                                                                    \
    } while (0)

static bool perto(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }

void testaVetores() {
    Vec3 a{1, 2, 3}, b{4, 5, 6};
    CHECK(perto(dot(a, b), 32));
    Vec3 c = cross(Vec3(1, 0, 0), Vec3(0, 1, 0));
    CHECK(perto(c.x, 0) && perto(c.y, 0) && perto(c.z, 1));
    CHECK(perto(normalizar(Vec3(3, 4, 0)).comprimento(), 1));

    Vec3 r = refletir(Vec3(1, -1, 0), Vec3(0, 1, 0));
    CHECK(perto(r.x, 1) && perto(r.y, 1));
}

void testaRefracaoSemMudarDeMeio() {
    // eta = 1: a luz atravessa sem entortar
    Vec3 v = normalizar(Vec3(1, -1, 0));
    Vec3 r = refratar(v, Vec3(0, 1, 0), 1.0);
    CHECK(perto(r.x, v.x, 1e-9) && perto(r.y, v.y, 1e-9));
}

void testaEsfera() {
    auto mat = std::make_shared<Difuso>(Cor(0.5, 0.5, 0.5));
    Esfera esfera(Ponto3(0, 0, -5), 1, mat);
    Impacto imp;

    // Raio direto no centro: bate a 4 unidades (5 - raio 1)
    CHECK(esfera.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));
    CHECK(perto(imp.t, 4));
    CHECK(perto(imp.normal.z, 1));  // normal aponta para a camera
    CHECK(imp.faceDeFora);
    CHECK(imp.material == mat.get());

    // Raio para o outro lado: erra
    CHECK(!esfera.atinge(Raio{{0, 0, 0}, {0, 0, 1}}, Intervalo(0.001, 1e9), imp));
    // Raio que passa por cima: erra
    CHECK(!esfera.atinge(Raio{{0, 2, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));
}

void testaCenaPegaOMaisProximo() {
    auto mat = std::make_shared<Difuso>(Cor(1, 1, 1));
    Cena cena;
    cena.adicionar(std::make_shared<Esfera>(Ponto3(0, 0, -10), 1, mat));
    cena.adicionar(std::make_shared<Esfera>(Ponto3(0, 0, -5), 1, mat));
    Impacto imp;
    CHECK(cena.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));
    CHECK(perto(imp.t, 4));
}

void testaBVHIgualALista() {
    // A BVH tem que dar exatamente o mesmo resultado que testar tudo na forca bruta
    auto mat = std::make_shared<Difuso>(Cor(1, 1, 1));
    std::vector<std::shared_ptr<Objeto>> objs;
    Cena lista;
    for (int i = 0; i < 50; i++) {
        auto e = std::make_shared<Esfera>(Ponto3(i % 7 - 3.0, i / 7 - 3.0, -10.0 - i * 0.3), 0.4, mat);
        objs.push_back(e);
        lista.adicionar(e);
    }
    NoBVH bvh(objs);

    int divergencias = 0;
    for (int y = -20; y <= 20; y++) {
        for (int x = -20; x <= 20; x++) {
            Raio r{{0, 0, 0}, {x * 0.02, y * 0.02, -1}};
            Impacto a, b;
            bool ha = lista.atinge(r, Intervalo(0.001, 1e9), a);
            bool hb = bvh.atinge(r, Intervalo(0.001, 1e9), b);
            if (ha != hb || (ha && !perto(a.t, b.t))) divergencias++;
        }
    }
    CHECK(divergencias == 0);
}

void testaCaixa() {
    Caixa c(Ponto3(-1, -1, -1), Ponto3(1, 1, 1));
    CHECK(c.atinge(Raio{{0, 0, 5}, {0, 0, -1}}, Intervalo(0, 100)));
    CHECK(!c.atinge(Raio{{3, 0, 5}, {0, 0, -1}}, Intervalo(0, 100)));
}

void testaCorParaByte() {
    CHECK(Imagem::paraByte(0) == 0);
    CHECK(Imagem::paraByte(1) == 255);
    CHECK(Imagem::paraByte(-5) == 0);
    CHECK(Imagem::paraByte(0.25) == 128);  // gama: sqrt(0.25) = 0.5
}

void testaBMP() {
    Imagem img(3, 2);  // largura 3 -> linha com 9 bytes + 3 de preenchimento = 12
    img.salvarBMP("teste_tmp.bmp");
    std::ifstream f("teste_tmp.bmp", std::ios::binary | std::ios::ate);
    CHECK(f.is_open());
    CHECK(static_cast<long>(f.tellg()) == 54 + 12 * 2);
    f.close();
    std::remove("teste_tmp.bmp");
}

void testaTriangulo() {
    auto mat = std::make_shared<Difuso>(Cor(1, 1, 1));
    // Triangulo no plano z = -2, cobrindo a origem
    Triangulo tri(Ponto3(-1, -1, -2), Ponto3(1, -1, -2), Ponto3(0, 1, -2), mat);
    Impacto imp;
    CHECK(tri.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));
    CHECK(perto(imp.t, 2));
    CHECK(perto(imp.normal.z, 1));               // normal virada para a camera
    CHECK(!tri.atinge(Raio{{5, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));  // passa ao lado
    CHECK(!tri.atinge(Raio{{0, 0, 0}, {1, 0, 0}}, Intervalo(0.001, 1e9), imp));   // paralelo
    CHECK(!tri.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1.5), imp));  // alem do limite
}

void testaTrianguloSuave() {
    auto mat = std::make_shared<Difuso>(Cor(1, 1, 1));
    // Normais dos vertices inclinadas: no centro, a normal interpolada aponta reto
    Triangulo tri(Ponto3(-1, -1, -2), Ponto3(1, -1, -2), Ponto3(0, 1, -2),
                  Vec3(-1, -1, 2), Vec3(1, -1, 2), Vec3(0, 1.4142, 2), mat);
    Impacto imp;
    CHECK(tri.atinge(Raio{{0, -1.0 / 3, 0}, {0, 0, -1}}, Intervalo(0.001, 1e9), imp));
    CHECK(imp.normal.z > 0.9);
}

void testaLeituraOBJ() {
    // Um quadrado (4 vertices, 1 face de 4 lados) vira 2 triangulos
    DadosMalha m = lerOBJ(
        "# comentario\n"
        "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\n"
        "vn 0 0 1\n"
        "o qualquer\n"
        "f 1//1 2//1 3//1 4//1\n"
        "f -4 -3 -2\n");  // indices negativos contam do fim
    CHECK(m.vertices.size() == 4);
    CHECK(m.normais.size() == 1);
    CHECK(m.faces.size() == 3);
    CHECK(m.faces[1].v[0] == 0 && m.faces[1].v[1] == 2 && m.faces[1].v[2] == 3);
    CHECK(m.faces[0].n[0] == 0);
    CHECK(m.faces[2].v[0] == 0 && m.faces[2].n[0] == -1);

    auto tris = criarTriangulos(m, Vec3(0, 0, -5), 2.0, std::make_shared<Difuso>(Cor(1, 1, 1)));
    CHECK(tris.size() == 3);

    bool lancou = false;
    try { lerOBJ("v 0 0 0\nf 1 2 3\n"); } catch (const std::exception&) { lancou = true; }
    CHECK(lancou);  // indice que nao existe
}

// Roda um script que deveria falhar e devolve a mensagem de erro ("" se nao falhou)
static std::string erroDoScript(const std::string& codigo) {
    try {
        lerCenaLua(codigo, "teste.lua");
    } catch (const std::exception& e) {
        return e.what();
    }
    return "";
}

static bool contem(const std::string& texto, const std::string& trecho) { return texto.find(trecho) != std::string::npos; }

void testaCenaLua() {
    CenaPronta c = lerCenaLua(
        "esfera { centro = {0, -100, 0}, raio = 100, material = difuso {0.5, 0.5, 0.5} }\n"
        "for i = 1, 3 do\n"
        "  esfera { centro = {i, 1, 0}, raio = 0.5, material = metal({1, 1, 1}, 0.2) }\n"
        "end\n"
        "esfera { centro = {0, 1, 2}, raio = 0.5, material = vidro(1.33) }\n"
        "camera { posicao = {0, 2, 10}, alvo = {0, 1, 0}, campo_de_visao = 30 }\n"
        "ceu(false)\n",
        "teste.lua");
    CHECK(c.esferas.size() == 5);
    CHECK(c.esferas[1].material.tipo == DescricaoMaterial::Tipo::Metal);
    CHECK(perto(c.esferas[1].material.rugosidade, 0.2));
    CHECK(perto(c.esferas[4].material.indice, 1.33));
    CHECK(perto(c.camera.campoDeVisao, 30));
    CHECK(perto(c.camera.posicao.z, 10));
    CHECK(!c.ceu);
    CHECK(c.mundo != nullptr);

    // A cena montada funciona: um raio para baixo bate no chao
    Impacto imp;
    CHECK(c.mundo->atinge(Raio{{0, 5, 5}, {0, -1, 0}}, Intervalo(0.001, 1e9), imp));
}

void testaErrosLua() {
    // Erros mostram o arquivo e a linha
    std::string e = erroDoScript("esfera { centro = {0, 0, 0}, raio = 1, material = difuso {1, 1, 1} }\n"
                                 "esfera { centro = {0, 0, 0}, material = difuso {1, 1, 1} }\n");
    CHECK(contem(e, "teste.lua:2:"));
    CHECK(contem(e, "raio"));

    CHECK(contem(erroDoScript("esfera { centro = {0, 0}, raio = 1, material = vidro() }"), "3 numeros"));
    CHECK(contem(erroDoScript("esfera { centro = {0, 0, 0}, raio = 1 }"), "material"));
    CHECK(contem(erroDoScript("esfera { centro = {0, 0, 0}, raio = -1, material = vidro() }"), "maior que zero"));
    CHECK(contem(erroDoScript("camera { posicao = {0, 0, 0} }"), "nenhum objeto"));
    CHECK(contem(erroDoScript("isso nao e lua"), "teste.lua:1:"));  // erro de sintaxe
}

void testaSandboxLua() {
    // Sem acesso a arquivos ou ao sistema
    CHECK(contem(erroDoScript("os.execute('echo oi')"), "os"));
    CHECK(contem(erroDoScript("io.open('x.txt', 'w')"), "io"));
    CHECK(contem(erroDoScript("dofile('outro.lua')"), "dofile"));
    CHECK(contem(erroDoScript("require('socket')"), "require"));
    // Laco infinito e interrompido
    CHECK(contem(erroDoScript("while true do end"), "demorou demais"));
}

int main() {
    testaVetores();
    testaRefracaoSemMudarDeMeio();
    testaEsfera();
    testaCenaPegaOMaisProximo();
    testaBVHIgualALista();
    testaCaixa();
    testaCorParaByte();
    testaBMP();
    testaTriangulo();
    testaTrianguloSuave();
    testaLeituraOBJ();
    testaCenaLua();
    testaErrosLua();
    testaSandboxLua();

    if (falhas == 0) {
        std::cout << "Todos os testes passaram!\n";
        return 0;
    }
    std::cerr << falhas << " verificacao(oes) falharam.\n";
    return 1;
}
