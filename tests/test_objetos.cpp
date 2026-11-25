// Testes de intersecao: esfera, triangulo, cena e BVH

#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <vector>

#include "BVH.h"
#include "Cena.h"
#include "Esfera.h"
#include "Material.h"
#include "Triangulo.h"

namespace {

const Intervalo kTudo(0.001, 1e9);

std::shared_ptr<Material> materialQualquer() { return std::make_shared<Difuso>(Cor(0.5, 0.5, 0.5)); }

}  // namespace

TEST(Esfera, RaioNoCentroBateNaFrente) {
    auto mat = materialQualquer();
    Esfera esfera(Ponto3(0, 0, -5), 1, mat);
    Impacto imp;
    ASSERT_TRUE(esfera.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, kTudo, imp));
    EXPECT_NEAR(imp.t, 4, 1e-9);         // 5 de distancia - 1 de raio
    EXPECT_NEAR(imp.normal.z, 1, 1e-9);  // normal virada para a camera
    EXPECT_TRUE(imp.faceDeFora);
    EXPECT_EQ(imp.material, mat.get());
}

TEST(Esfera, RaioQueErra) {
    Esfera esfera(Ponto3(0, 0, -5), 1, materialQualquer());
    Impacto imp;
    EXPECT_FALSE(esfera.atinge(Raio{{0, 0, 0}, {0, 0, 1}}, kTudo, imp));   // para o outro lado
    EXPECT_FALSE(esfera.atinge(Raio{{0, 2, 0}, {0, 0, -1}}, kTudo, imp));  // passa por cima
}

TEST(Esfera, RaioSaindoDeDentroVeFaceDeDentro) {
    Esfera esfera(Ponto3(0, 0, 0), 1, materialQualquer());
    Impacto imp;
    ASSERT_TRUE(esfera.atinge(Raio{{0, 0, 0}, {1, 0, 0}}, kTudo, imp));
    EXPECT_FALSE(imp.faceDeFora);
    EXPECT_NEAR(imp.normal.x, -1, 1e-9);  // a normal sempre aponta contra o raio
}

TEST(Triangulo, AtingeNoMeio) {
    Triangulo tri(Ponto3(-1, -1, -2), Ponto3(1, -1, -2), Ponto3(0, 1, -2), materialQualquer());
    Impacto imp;
    ASSERT_TRUE(tri.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, kTudo, imp));
    EXPECT_NEAR(imp.t, 2, 1e-9);
    EXPECT_NEAR(imp.normal.z, 1, 1e-9);
}

TEST(Triangulo, CasosQueErram) {
    Triangulo tri(Ponto3(-1, -1, -2), Ponto3(1, -1, -2), Ponto3(0, 1, -2), materialQualquer());
    Impacto imp;
    EXPECT_FALSE(tri.atinge(Raio{{5, 0, 0}, {0, 0, -1}}, kTudo, imp));                  // passa ao lado
    EXPECT_FALSE(tri.atinge(Raio{{0, 0, 0}, {1, 0, 0}}, kTudo, imp));                   // paralelo
    EXPECT_FALSE(tri.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, Intervalo(0.001, 1.5), imp));  // alem do limite
}

TEST(Triangulo, NormaisSuavesSaoInterpoladas) {
    // Normais dos vertices inclinadas para fora: no centro, a media aponta reto
    Triangulo tri(Ponto3(-1, -1, -2), Ponto3(1, -1, -2), Ponto3(0, 1, -2), Vec3(-1, -1, 2), Vec3(1, -1, 2),
                  Vec3(0, 1.4142, 2), materialQualquer());
    Impacto imp;
    ASSERT_TRUE(tri.atinge(Raio{{0, -1.0 / 3, 0}, {0, 0, -1}}, kTudo, imp));
    EXPECT_GT(imp.normal.z, 0.9);
}

TEST(Cena, PegaOObjetoMaisProximo) {
    auto mat = materialQualquer();
    Cena cena;
    cena.adicionar(std::make_shared<Esfera>(Ponto3(0, 0, -10), 1, mat));
    cena.adicionar(std::make_shared<Esfera>(Ponto3(0, 0, -5), 1, mat));
    Impacto imp;
    ASSERT_TRUE(cena.atinge(Raio{{0, 0, 0}, {0, 0, -1}}, kTudo, imp));
    EXPECT_NEAR(imp.t, 4, 1e-9);
}

TEST(BVH, DaOMesmoResultadoQueForcaBruta) {
    // A BVH so pode acelerar: o resultado tem que ser identico a testar tudo
    auto mat = materialQualquer();
    std::vector<std::shared_ptr<Objeto>> objetos;
    Cena lista;
    for (int i = 0; i < 50; i++) {
        auto e = std::make_shared<Esfera>(Ponto3(i % 7 - 3.0, i / 7 - 3.0, -10.0 - i * 0.3), 0.4, mat);
        objetos.push_back(e);
        lista.adicionar(e);
    }
    NoBVH bvh(objetos);

    int divergencias = 0;
    for (int y = -20; y <= 20; y++) {
        for (int x = -20; x <= 20; x++) {
            Raio r{{0, 0, 0}, {x * 0.02, y * 0.02, -1}};
            Impacto a, b;
            bool acertouLista = lista.atinge(r, kTudo, a);
            bool acertouBVH = bvh.atinge(r, kTudo, b);
            if (acertouLista != acertouBVH || (acertouLista && std::abs(a.t - b.t) > 1e-9)) divergencias++;
        }
    }
    EXPECT_EQ(divergencias, 0);
}

TEST(BVH, ListaVaziaELancaExcecao) {
    EXPECT_THROW(NoBVH(std::vector<std::shared_ptr<Objeto>>{}), std::invalid_argument);
}
