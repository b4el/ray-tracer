#include "Cenas.h"

#include <fstream>
#include <iomanip>
#include <random>
#include <stdexcept>

#include "Aleatorio.h"
#include "BVH.h"
#include "Cena.h"
#include "Esfera.h"
#include "Malha.h"
#include "Material.h"

namespace {

using Tipo = DescricaoMaterial::Tipo;

DescricaoMaterial difuso(const Cor& cor) { return {Tipo::Difuso, cor, 0, 1.5}; }
DescricaoMaterial metal(const Cor& cor, double rugosidade) { return {Tipo::Metal, cor, rugosidade, 1.5}; }
DescricaoMaterial vidro(double indice) { return {Tipo::Vidro, {1, 1, 1}, 0, indice}; }
DescricaoMaterial luz(const Cor& cor) { return {Tipo::Luz, cor, 0, 1.5}; }

std::shared_ptr<Material> criarMaterial(const DescricaoMaterial& d) {
    switch (d.tipo) {
        case Tipo::Difuso: return std::make_shared<Difuso>(d.cor);
        case Tipo::Metal: return std::make_shared<Metal>(d.cor, d.rugosidade);
        case Tipo::Vidro: return std::make_shared<Vidro>(d.indice);
        case Tipo::Luz: return std::make_shared<Luz>(d.cor);
    }
    throw std::logic_error("Tipo de material desconhecido.");
}

// Transforma as descricoes em objetos de verdade.
// Com muitos objetos, usa a BVH; com poucos, uma lista simples basta.
void construirMundo(CenaPronta& cena, bool usarBVH) {
    std::vector<std::shared_ptr<Objeto>> objetos;
    for (const DescricaoEsfera& e : cena.esferas)
        objetos.push_back(std::make_shared<Esfera>(e.centro, e.raio, criarMaterial(e.material)));

    for (const DescricaoMalha& m : cena.malhas) {
        DadosMalha dados = carregarOBJ("modelos/" + m.arquivo);
        auto triangulos = criarTriangulos(dados, m.deslocamento, m.escala, criarMaterial(m.material));
        objetos.insert(objetos.end(), triangulos.begin(), triangulos.end());
    }

    if (usarBVH && objetos.size() > 16) {
        cena.mundo = std::make_shared<NoBVH>(objetos);
    } else {
        auto lista = std::make_shared<Cena>();
        for (auto& o : objetos) lista->adicionar(o);
        cena.mundo = lista;
    }
}

// Tres esferas lado a lado: vidro oco, fosca e metal
CenaPronta cenaSimples() {
    CenaPronta c;
    c.esferas = {
        {{0, -100.5, -1}, 100, difuso({0.8, 0.8, 0.0})},
        {{0, 0, -1.2}, 0.5, difuso({0.1, 0.2, 0.5})},
        {{-1, 0, -1}, 0.5, vidro(1.5)},
        {{-1, 0, -1}, 0.4, vidro(1.0 / 1.5)},  // bolha de ar dentro do vidro = vidro oco
        {{1, 0, -1}, 0.5, metal({0.8, 0.6, 0.2}, 0.1)},
    };
    c.camera.posicao = {0, 0.6, 1.4};
    c.camera.alvo = {0, 0, -1};
    c.camera.campoDeVisao = 50;
    return c;
}

// A capa do projeto: muitas bolinhas aleatorias + tres esferas grandes
CenaPronta cenaCapa() {
    aleatorio::semente(2026);  // mesma semente = mesma cena sempre (no mesmo compilador)

    CenaPronta c;
    c.esferas.push_back({{0, -1000, 0}, 1000, difuso({0.5, 0.5, 0.5})});

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            double sorteio = aleatorio::real();
            double dx = aleatorio::real();
            double dz = aleatorio::real();
            Ponto3 centro(a + 0.9 * dx, 0.2, b + 0.9 * dz);
            if ((centro - Ponto3(4, 0.2, 0)).comprimento() <= 0.9) continue;  // nao encostar na esfera grande

            DescricaoMaterial material;
            if (sorteio < 0.75) {
                Cor cor = aleatorio::vetor(0, 1);
                cor = cor * aleatorio::vetor(0, 1);
                material = difuso(cor);
            } else if (sorteio < 0.92) {
                Cor cor = aleatorio::vetor(0.5, 1);
                material = metal(cor, aleatorio::real(0, 0.4));
            } else {
                material = vidro(1.5);
            }
            c.esferas.push_back({centro, 0.2, material});
        }
    }

    c.esferas.push_back({{0, 1, 0}, 1.0, vidro(1.5)});
    c.esferas.push_back({{-4, 1, 0}, 1.0, difuso({1.0, 0.54, 0.12})});
    c.esferas.push_back({{4, 1, 0}, 1.0, metal({0.7, 0.6, 0.5}, 0.0)});

    aleatorio::semente(std::random_device{}());  // volta ao aleatorio normal

    c.camera.posicao = {13, 2, 3};
    c.camera.alvo = {0, 0, 0};
    c.camera.campoDeVisao = 20;
    c.camera.abertura = 0.1;
    c.camera.distanciaFoco = 10;
    return c;
}

// Sem ceu: so lampadas iluminam a cena
CenaPronta cenaNoite() {
    CenaPronta c;
    c.esferas = {
        {{0, -1000, 0}, 1000, difuso({0.4, 0.4, 0.45})},
        {{0, 1, 0}, 1, metal({0.9, 0.9, 0.9}, 0.05)},
        {{-2.2, 0.7, 0.8}, 0.7, vidro(1.5)},
        {{2.2, 0.7, -0.6}, 0.7, difuso({0.2, 0.6, 0.9})},
        {{0, 5, 3}, 1.6, luz({5, 3.6, 2.0})},         // luz quente
        {{-3.5, 1.2, -2.5}, 0.6, luz({1.5, 3, 7})},   // luz azul
    };
    c.camera.posicao = {0, 2.2, 8};
    c.camera.alvo = {0, 0.8, 0};
    c.camera.campoDeVisao = 35;
    c.ceu = false;
    return c;
}

// Modelos 3D (.obj): no toroidal de vidro, gema dourada e esferas
CenaPronta cenaModelos() {
    CenaPronta c;
    c.esferas = {
        {{0, -1000, 0}, 1000, difuso({0.45, 0.47, 0.5})},
        {{2.6, 0.6, 0.9}, 0.6, difuso({0.75, 0.15, 0.2})},
        {{-1.4, 0.3, 1.9}, 0.3, metal({0.85, 0.85, 0.9}, 0.0)},
        {{1.3, 0.25, 2.1}, 0.25, vidro(1.5)},
    };
    c.malhas = {
        {"no_toroidal.obj", {0, 0, 0}, 2.4, vidro(1.5)},
        {"gema.obj", {-2.6, 0, 0.4}, 1.5, metal({1.0, 0.72, 0.3}, 0.02)},
    };
    c.camera.posicao = {0, 2.6, 8};
    c.camera.alvo = {0, 0.8, 0};
    c.camera.campoDeVisao = 35;
    c.camera.abertura = 0.04;
    c.camera.distanciaFoco = 8;
    return c;
}

const char* nomeDoTipo(Tipo t) {
    switch (t) {
        case Tipo::Difuso: return "difuso";
        case Tipo::Metal: return "metal";
        case Tipo::Vidro: return "vidro";
        case Tipo::Luz: return "luz";
    }
    return "?";
}

void escreverVetor(std::ostream& os, const Vec3& v) { os << '[' << v.x << ", " << v.y << ", " << v.z << ']'; }

}  // namespace

void montarMundo(CenaPronta& cena, bool usarBVH) { construirMundo(cena, usarBVH); }

std::vector<std::string> nomesDasCenas() { return {"capa", "simples", "noite", "modelos"}; }

CenaPronta montarCena(const std::string& nome, bool usarBVH) {
    CenaPronta c;
    if (nome == "capa") c = cenaCapa();
    else if (nome == "simples") c = cenaSimples();
    else if (nome == "noite") c = cenaNoite();
    else if (nome == "modelos") c = cenaModelos();
    else throw std::invalid_argument("Cena desconhecida: " + nome);

    c.nome = nome;
    construirMundo(c, usarBVH);
    return c;
}

// Exporta todas as cenas em JSON (lido pela pagina docs/index.html)
void exportarCenasJSON(const std::string& caminho) {
    std::ofstream os(caminho);
    if (!os) throw std::runtime_error("Nao foi possivel criar " + caminho);
    os << std::setprecision(6);

    os << "{\n";
    auto nomes = nomesDasCenas();
    for (size_t i = 0; i < nomes.size(); i++) {
        CenaPronta c = montarCena(nomes[i]);
        os << "  \"" << c.nome << "\": {\n";
        os << "    \"ceu\": " << (c.ceu ? "true" : "false") << ",\n";
        os << "    \"camera\": { \"posicao\": ";
        escreverVetor(os, c.camera.posicao);
        os << ", \"alvo\": ";
        escreverVetor(os, c.camera.alvo);
        os << ", \"campoDeVisao\": " << c.camera.campoDeVisao << " },\n";
        os << "    \"esferas\": [\n";
        for (size_t j = 0; j < c.esferas.size(); j++) {
            const DescricaoEsfera& e = c.esferas[j];
            os << "      { \"centro\": ";
            escreverVetor(os, e.centro);
            os << ", \"raio\": " << e.raio << ", \"material\": \"" << nomeDoTipo(e.material.tipo) << "\", \"cor\": ";
            escreverVetor(os, e.material.cor);
            os << ", \"rugosidade\": " << e.material.rugosidade << ", \"indice\": " << e.material.indice << " }"
               << (j + 1 < c.esferas.size() ? "," : "") << "\n";
        }
        os << "    ],\n    \"malhas\": [\n";
        for (size_t j = 0; j < c.malhas.size(); j++) {
            const DescricaoMalha& m = c.malhas[j];
            os << "      { \"arquivo\": \"modelos/" << m.arquivo << "\", \"deslocamento\": ";
            escreverVetor(os, m.deslocamento);
            os << ", \"escala\": " << m.escala << ", \"material\": \"" << nomeDoTipo(m.material.tipo) << "\", \"cor\": ";
            escreverVetor(os, m.material.cor);
            os << ", \"rugosidade\": " << m.material.rugosidade << ", \"indice\": " << m.material.indice << " }"
               << (j + 1 < c.malhas.size() ? "," : "") << "\n";
        }
        os << "    ]\n  }" << (i + 1 < nomes.size() ? "," : "") << "\n";
    }
    os << "}\n";
}
