"""Gera os modelos 3D (.obj) usados na cena "modelos".

Os modelos sao criados por codigo (nenhum arquivo de terceiros):
  - no_toroidal.obj : um "torus knot" (tubo que da voltas em torno de um toro), com normais suaves
  - gema.obj        : uma pedra lapidada, com faces planas (sem normais)

Cada modelo e centralizado em x/z, apoiado no chao (menor y = 0) e escalado
para que a maior dimensao seja 1.

Uso:  python ferramentas/gerar_modelos.py
Os arquivos sao salvos em modelos/ e copiados para docs/modelos/ (pagina web).
"""

import math
import os
import shutil

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def normalizar(v):
    n = math.sqrt(sum(c * c for c in v))
    return tuple(c / n for c in v)


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def enquadrar(vertices):
    """Centraliza em x/z, apoia no chao e escala para maior dimensao = 1."""
    mins = [min(v[i] for v in vertices) for i in range(3)]
    maxs = [max(v[i] for v in vertices) for i in range(3)]
    escala = 1.0 / max(maxs[i] - mins[i] for i in range(3))
    cx, cz = (mins[0] + maxs[0]) / 2, (mins[2] + maxs[2]) / 2
    return [((v[0] - cx) * escala, (v[1] - mins[1]) * escala, (v[2] - cz) * escala) for v in vertices]


def salvar(nome, comentario, vertices, faces, normais=None):
    linhas = [f"# {comentario}", f"# {len(vertices)} vertices, {len(faces)} faces"]
    linhas += [f"v {x:.6f} {y:.6f} {z:.6f}" for x, y, z in vertices]
    if normais:
        linhas += [f"vn {x:.6f} {y:.6f} {z:.6f}" for x, y, z in normais]
        linhas += ["f " + " ".join(f"{i + 1}//{i + 1}" for i in f) for f in faces]
    else:
        linhas += ["f " + " ".join(str(i + 1) for i in f) for f in faces]
    for pasta in ("modelos", os.path.join("docs", "modelos")):
        destino = os.path.join(RAIZ, pasta)
        os.makedirs(destino, exist_ok=True)
        with open(os.path.join(destino, nome), "w", newline="\n") as f:
            f.write("\n".join(linhas) + "\n")
    print(f"{nome}: {len(vertices)} vertices, {len(faces)} faces")


def girar(v, eixo_y_graus=0.0):
    """Gira um ponto em torno do eixo y."""
    a = math.radians(eixo_y_graus)
    return (v[0] * math.cos(a) + v[2] * math.sin(a), v[1], -v[0] * math.sin(a) + v[2] * math.cos(a))


def no_toroidal(p=2, q=3, segmentos=240, lados=20, raio_tubo=0.5, giro=25.0):
    """Curva (p, q) sobre um toro, envolvida por um tubo. Fica "em pe", de frente para a camera."""
    def curva(t):
        r = 2 + math.cos(q * t)
        return girar((r * math.cos(p * t), r * math.sin(p * t), math.sin(q * t)), giro)

    vertices, normais = [], []
    for i in range(segmentos):
        t = 2 * math.pi * i / segmentos
        centro = curva(t)
        tangente = normalizar(sub(curva(t + 1e-4), curva(t - 1e-4)))
        # Base que acompanha o tubo: usa a direcao para o eixo do toro como referencia
        eixo = girar((0.0, 0.0, 1.0), giro)
        dentro = sub((0.0, 0.0, 0.0), centro)
        proj = sum(dentro[k] * eixo[k] for k in range(3))
        referencia = normalizar(tuple(dentro[k] - proj * eixo[k] for k in range(3)))
        b = normalizar(cross(tangente, referencia))
        n = cross(b, tangente)
        for j in range(lados):
            a = 2 * math.pi * j / lados
            normal = tuple(math.cos(a) * n[k] + math.sin(a) * b[k] for k in range(3))
            vertices.append(tuple(centro[k] + raio_tubo * normal[k] for k in range(3)))
            normais.append(normal)

    faces = []
    for i in range(segmentos):
        for j in range(lados):
            a = i * lados + j
            b = ((i + 1) % segmentos) * lados + j
            c = ((i + 1) % segmentos) * lados + (j + 1) % lados
            d = i * lados + (j + 1) % lados
            faces += [(a, b, c), (a, c, d)]

    # Garante que as normais apontam para fora do tubo
    v0, n0 = vertices[0], normais[0]
    if sum((v0[k] - curva(0)[k]) * n0[k] for k in range(3)) < 0:
        normais = [tuple(-c for c in n) for n in normais]
    return enquadrar(vertices), faces, normais


def gema(lados=12):
    """Pedra lapidada: mesa (topo plano), coroa, cinta e pavilhao (ponta embaixo)."""
    vertices = []
    def anel(raio, y, giro=0.0):
        inicio = len(vertices)
        for i in range(lados):
            a = 2 * math.pi * (i + giro) / lados
            vertices.append((raio * math.cos(a), y, raio * math.sin(a)))
        return inicio

    mesa = anel(0.55, 0.45)
    cinta = anel(1.0, 0.0, 0.5)
    ponta = len(vertices)
    vertices.append((0.0, -1.1, 0.0))
    centro_mesa = len(vertices)
    vertices.append((0.0, 0.45, 0.0))

    faces = []
    for i in range(lados):
        j = (i + 1) % lados
        faces.append((centro_mesa, mesa + j, mesa + i))           # mesa (topo)
        faces.append((mesa + i, mesa + j, cinta + i))             # coroa
        faces.append((mesa + j, cinta + j, cinta + i))
        faces.append((cinta + i, cinta + j, ponta))               # pavilhao
    return enquadrar(vertices), faces


if __name__ == "__main__":
    v, f, n = no_toroidal()
    salvar("no_toroidal.obj", "No toroidal (2,3) gerado por ferramentas/gerar_modelos.py", v, f, n)
    v, f = gema()
    salvar("gema.obj", "Gema lapidada gerada por ferramentas/gerar_modelos.py", v, f)
