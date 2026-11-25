#include "Renderizador.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include "Aleatorio.h"
#include "Material.h"

Cor corDoRaio(const Raio& r, const Objeto& mundo, int profundidade, bool ceu) {
    if (profundidade <= 0) return {0, 0, 0};  // quicou demais: nao chega luz

    Impacto impacto;
    // 0.001 em vez de 0 evita que o raio bata na propria superficie de onde saiu
    // (o famoso "shadow acne")
    if (!mundo.atinge(r, Intervalo(0.001, std::numeric_limits<double>::infinity()), impacto)) {
        if (!ceu) return {0, 0, 0};
        // Ceu: degrade de branco (horizonte) para azul (alto)
        double a = 0.5 * (normalizar(r.direcao).y + 1.0);
        return (1.0 - a) * Cor(1.0, 1.0, 1.0) + a * Cor(0.5, 0.7, 1.0);
    }

    Cor emitida = impacto.material->emitida();
    Raio espalhado;
    Cor atenuacao;
    if (!impacto.material->espalhar(r, impacto, atenuacao, espalhado)) return emitida;

    return emitida + atenuacao * corDoRaio(espalhado, mundo, profundidade - 1, ceu);
}

Imagem renderizar(const Objeto& mundo, const Camera& camera, const ConfigRender& cfg) {
    Imagem imagem(cfg.largura, cfg.altura);

    int numThreads = cfg.threads > 0 ? cfg.threads : static_cast<int>(std::thread::hardware_concurrency());
    numThreads = std::max(1, numThreads);

    // Cada thread pega a proxima linha livre. atomic garante que duas threads
    // nunca pegam a mesma linha, sem precisar de mutex.
    std::atomic<int> proximaLinha{0};
    std::atomic<int> linhasProntas{0};
    std::mutex mutexSaida;

    auto trabalhador = [&]() {
        while (true) {
            int y = proximaLinha.fetch_add(1);
            if (y >= cfg.altura) break;

            for (int x = 0; x < cfg.largura; x++) {
                Cor soma{0, 0, 0};
                for (int s = 0; s < cfg.amostrasPorPixel; s++) {
                    // Antialiasing: cada amostra cai num ponto aleatorio dentro do pixel
                    double u = (x + aleatorio::real()) / (cfg.largura - 1);
                    double v = (cfg.altura - 1 - y + aleatorio::real()) / (cfg.altura - 1);
                    soma += corDoRaio(camera.gerarRaio(u, v), mundo, cfg.profundidadeMaxima, cfg.ceu);
                }
                imagem.definir(x, y, soma / cfg.amostrasPorPixel);
            }

            int prontas = ++linhasProntas;
            if (cfg.mostrarProgresso) {
                std::lock_guard<std::mutex> trava(mutexSaida);
                std::cout << "\rRenderizando: " << (100 * prontas / cfg.altura) << "% " << std::flush;
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(numThreads));
    for (int i = 0; i < numThreads; i++) threads.emplace_back(trabalhador);
    for (auto& t : threads) t.join();  // espera todas terminarem

    if (cfg.mostrarProgresso) std::cout << "\rRenderizando: 100%  \n";
    return imagem;
}
