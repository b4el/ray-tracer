"""Gera as texturas de exemplo (marmore e madeira) por codigo, sem imagens de terceiros.

Uso:  python ferramentas/gerar_texturas.py   (precisa do Pillow: pip install pillow)
Os arquivos vao para texturas/ e sao copiados para docs/texturas/ (pagina web).
"""

import math
import os
import random

from PIL import Image

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
random.seed(7)


def ruido_suave(tamanho, oitavas=5):
    """Ruido de valor com varias oitavas (bem simples), devolve uma funcao f(x, y) em [0, 1]."""
    grades = []
    for o in range(oitavas):
        n = 4 * 2 ** o
        grades.append((n, [[random.random() for _ in range(n + 1)] for _ in range(n + 1)]))

    def f(x, y):
        total, peso, soma_pesos = 0.0, 1.0, 0.0
        for n, g in grades:
            gx, gy = x / tamanho * n, y / tamanho * n
            i, j = int(gx) % n, int(gy) % n
            fx, fy = gx - int(gx), gy - int(gy)
            sx, sy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
            a = g[j][i] + (g[j][i + 1] - g[j][i]) * sx
            b = g[j + 1][i] + (g[j + 1][i + 1] - g[j + 1][i]) * sx
            total += (a + (b - a) * sy) * peso
            soma_pesos += peso
            peso *= 0.5
        return total / soma_pesos

    return f


def salvar(nome, img):
    for pasta in ("texturas", os.path.join("docs", "texturas")):
        destino = os.path.join(RAIZ, pasta)
        os.makedirs(destino, exist_ok=True)
        img.save(os.path.join(destino, nome), optimize=True)
    print(nome, img.size)


def marmore(tamanho=512):
    f = ruido_suave(tamanho)
    img = Image.new("RGB", (tamanho, tamanho))
    px = img.load()
    for y in range(tamanho):
        for x in range(tamanho):
            veio = abs(math.sin((x + y * 0.4) / tamanho * 9 + f(x, y) * 9))
            v = 0.55 + 0.45 * veio ** 0.35
            px[x, y] = (int(235 * v), int(232 * v), int(228 * v))
    return img


def madeira(tamanho=512):
    f = ruido_suave(tamanho, 4)
    img = Image.new("RGB", (tamanho, tamanho))
    px = img.load()
    cx, cy = tamanho * 0.3, -tamanho * 0.8
    for y in range(tamanho):
        for x in range(tamanho):
            d = math.hypot(x - cx, (y - cy) * 0.15) / tamanho * 28 + f(x, y) * 4
            anel = 0.5 + 0.5 * math.sin(d * 2 * math.pi)
            v = 0.65 + 0.35 * anel
            px[x, y] = (int(150 * v), int(95 * v), int(55 * v))
    return img


if __name__ == "__main__":
    salvar("marmore.png", marmore())
    salvar("madeira.png", madeira())
