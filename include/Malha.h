#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Material.h"
#include "Objeto.h"

// Dados lidos de um arquivo .obj (formato de modelos 3D em texto)
struct DadosMalha {
    struct Face {
        int v[3];                 // indices dos vertices
        int n[3] = {-1, -1, -1};  // indices das normais (-1 = sem normal)
    };
    std::vector<Ponto3> vertices;
    std::vector<Vec3> normais;
    std::vector<Face> faces;  // sempre triangulos (poligonos sao divididos)
};

// Le um .obj. Entende as linhas "v", "vn" e "f" (nos formatos i, i/j, i//k, i/j/k
// e indices negativos); o resto e ignorado. Lanca excecao se nao conseguir ler.
DadosMalha carregarOBJ(const std::string& caminho);

// Mesma leitura, a partir de um texto (usado nos testes)
DadosMalha lerOBJ(const std::string& conteudo);

// Transforma a malha em triangulos prontos para a cena:
// escala, depois desloca, e usa normais suaves quando o arquivo as tiver.
std::vector<std::shared_ptr<Objeto>> criarTriangulos(const DadosMalha& malha, const Vec3& deslocamento, double escala,
                                                     const std::shared_ptr<Material>& material);
