#include "Malha.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "Triangulo.h"

namespace {

// Converte um indice do .obj (comeca em 1, ou negativo = contando do fim) para base 0
int converterIndice(const std::string& texto, size_t quantidade, int linha) {
    int i = std::stoi(texto);
    int convertido = i > 0 ? i - 1 : static_cast<int>(quantidade) + i;
    if (i == 0 || convertido < 0 || convertido >= static_cast<int>(quantidade))
        throw std::runtime_error("Indice invalido na linha " + std::to_string(linha) + ": " + texto);
    return convertido;
}

}  // namespace

DadosMalha lerOBJ(const std::string& conteudo) {
    DadosMalha m;
    std::istringstream entrada(conteudo);
    std::string linha;
    int numero = 0;

    while (std::getline(entrada, linha)) {
        numero++;
        std::istringstream ss(linha);
        std::string tipo;
        if (!(ss >> tipo) || tipo[0] == '#') continue;

        if (tipo == "v") {
            Ponto3 p;
            if (!(ss >> p.x >> p.y >> p.z)) throw std::runtime_error("Vertice invalido na linha " + std::to_string(numero));
            m.vertices.push_back(p);
        } else if (tipo == "vn") {
            Vec3 n;
            if (!(ss >> n.x >> n.y >> n.z)) throw std::runtime_error("Normal invalida na linha " + std::to_string(numero));
            m.normais.push_back(n);
        } else if (tipo == "f") {
            // Cada canto pode ser "v", "v/vt", "v//vn" ou "v/vt/vn"
            std::vector<int> vs, ns;
            std::string canto;
            while (ss >> canto) {
                size_t barra1 = canto.find('/');
                vs.push_back(converterIndice(canto.substr(0, barra1), m.vertices.size(), numero));
                int n = -1;
                if (barra1 != std::string::npos) {
                    size_t barra2 = canto.find('/', barra1 + 1);
                    if (barra2 != std::string::npos && barra2 + 1 < canto.size())
                        n = converterIndice(canto.substr(barra2 + 1), m.normais.size(), numero);
                }
                ns.push_back(n);
            }
            if (vs.size() < 3) throw std::runtime_error("Face com menos de 3 vertices na linha " + std::to_string(numero));

            // Divide poligonos em triangulos "em leque": (0,1,2), (0,2,3), ...
            for (size_t k = 1; k + 1 < vs.size(); k++) {
                DadosMalha::Face f;
                f.v[0] = vs[0]; f.v[1] = vs[k]; f.v[2] = vs[k + 1];
                f.n[0] = ns[0]; f.n[1] = ns[k]; f.n[2] = ns[k + 1];
                m.faces.push_back(f);
            }
        }
        // Outras linhas (vt, o, g, usemtl, s, ...) sao ignoradas
    }

    if (m.faces.empty()) throw std::runtime_error("O arquivo .obj nao tem nenhuma face.");
    return m;
}

DadosMalha carregarOBJ(const std::string& caminho) {
    std::ifstream arq(caminho);
    if (!arq) throw std::runtime_error("Nao foi possivel abrir o modelo " + caminho);
    std::stringstream conteudo;
    conteudo << arq.rdbuf();
    return lerOBJ(conteudo.str());
}

std::vector<std::shared_ptr<Objeto>> criarTriangulos(const DadosMalha& malha, const Vec3& deslocamento,
                                                     double escala, std::shared_ptr<Material> material) {
    std::vector<std::shared_ptr<Objeto>> triangulos;
    triangulos.reserve(malha.faces.size());

    for (const DadosMalha::Face& f : malha.faces) {
        Ponto3 a = escala * malha.vertices[f.v[0]] + deslocamento;
        Ponto3 b = escala * malha.vertices[f.v[1]] + deslocamento;
        Ponto3 c = escala * malha.vertices[f.v[2]] + deslocamento;

        // Ignora triangulos degenerados (area zero)
        if (cross(b - a, c - a).quaseZero()) continue;

        bool temNormais = f.n[0] >= 0 && f.n[1] >= 0 && f.n[2] >= 0;
        if (temNormais)
            triangulos.push_back(std::make_shared<Triangulo>(a, b, c, malha.normais[f.n[0]], malha.normais[f.n[1]],
                                                             malha.normais[f.n[2]], material));
        else
            triangulos.push_back(std::make_shared<Triangulo>(a, b, c, material));
    }
    return triangulos;
}
