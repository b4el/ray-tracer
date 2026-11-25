// Testes de matematica: vetores, reflexao, refracao e caixas (AABB)

#include <gtest/gtest.h>

#include "Caixa.h"
#include "Vec3.h"

TEST(Vetores, ProdutoEscalar) {
    EXPECT_DOUBLE_EQ(dot(Vec3(1, 2, 3), Vec3(4, 5, 6)), 32);
    EXPECT_DOUBLE_EQ(dot(Vec3(1, 0, 0), Vec3(0, 1, 0)), 0);  // perpendiculares
}

TEST(Vetores, ProdutoVetorialEPerpendicular) {
    Vec3 c = cross(Vec3(1, 0, 0), Vec3(0, 1, 0));
    EXPECT_DOUBLE_EQ(c.x, 0);
    EXPECT_DOUBLE_EQ(c.y, 0);
    EXPECT_DOUBLE_EQ(c.z, 1);
}

TEST(Vetores, NormalizarDeixaComprimentoUm) {
    EXPECT_NEAR(normalizar(Vec3(3, 4, 0)).comprimento(), 1.0, 1e-12);
    EXPECT_NEAR(normalizar(Vec3(-2, 7, 0.5)).comprimento(), 1.0, 1e-12);
}

TEST(Vetores, QuaseZero) {
    EXPECT_TRUE(Vec3(1e-10, 0, -1e-10).quaseZero());
    EXPECT_FALSE(Vec3(0, 0.01, 0).quaseZero());
}

TEST(Optica, ReflexaoComoEspelho) {
    Vec3 r = refletir(Vec3(1, -1, 0), Vec3(0, 1, 0));
    EXPECT_DOUBLE_EQ(r.x, 1);
    EXPECT_DOUBLE_EQ(r.y, 1);
}

TEST(Optica, RefracaoSemMudarDeMeioNaoEntorta) {
    // eta = 1: a luz atravessa em linha reta
    Vec3 v = normalizar(Vec3(1, -1, 0));
    Vec3 r = refratar(v, Vec3(0, 1, 0), 1.0);
    EXPECT_NEAR(r.x, v.x, 1e-9);
    EXPECT_NEAR(r.y, v.y, 1e-9);
}

TEST(Optica, RefracaoNoVidroAproximaDaNormal) {
    // Entrando no vidro (eta < 1) o raio "endireita", ficando mais perto da normal
    Vec3 v = normalizar(Vec3(1, -1, 0));
    Vec3 r = normalizar(refratar(v, Vec3(0, 1, 0), 1.0 / 1.5));
    EXPECT_LT(r.x, v.x);
    EXPECT_LT(r.y, 0);  // continua descendo
}

TEST(Caixa, RaioQueAtravessaEQueErra) {
    Caixa c(Ponto3(-1, -1, -1), Ponto3(1, 1, 1));
    EXPECT_TRUE(c.atinge(Raio{{0, 0, 5}, {0, 0, -1}}, Intervalo(0, 100)));
    EXPECT_FALSE(c.atinge(Raio{{3, 0, 5}, {0, 0, -1}}, Intervalo(0, 100)));
    EXPECT_FALSE(c.atinge(Raio{{0, 0, 5}, {0, 0, -1}}, Intervalo(0, 3)));  // limite antes da caixa
}

TEST(Caixa, UniaoEnvolveAsDuas) {
    Caixa u(Caixa(Ponto3(0, 0, 0), Ponto3(1, 1, 1)), Caixa(Ponto3(2, -1, 0), Ponto3(3, 0, 1)));
    EXPECT_DOUBLE_EQ(u.x.min, 0);
    EXPECT_DOUBLE_EQ(u.x.max, 3);
    EXPECT_DOUBLE_EQ(u.y.min, -1);
    EXPECT_EQ(u.eixoMaisLongo(), 0);
}
