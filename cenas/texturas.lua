-- texturas.lua: chao xadrez, paineis (quad) e texturas de imagem.
-- Rode com:  raytracer --cena cenas/texturas.lua --saida texturas.png
--
-- Novidades usadas aqui:
--   chao { altura = y, material = ... }            um chao enorme
--   quad { canto = ..., u = ..., v = ..., ... }    um retangulo (canto + dois lados)
--   xadrez(cor1, cor2, escala)                     textura de xadrez
--   imagem("marmore.png")                          textura lida da pasta texturas/
-- As texturas entram no lugar da cor de um material difuso: difuso(xadrez(...)).

-- Chao xadrez: quadrados de 0.5 de lado
chao { altura = 0, material = difuso(xadrez({0.85, 0.85, 0.82}, {0.12, 0.13, 0.16}, 0.5)) }

-- Parede de madeira ao fundo: canto inferior esquerdo + lado horizontal (u) + lado vertical (v)
quad {
  canto = {-4, 0, -2.5},
  u = {8, 0, 0},
  v = {0, 3.2, 0},
  material = difuso(imagem("madeira.png")),
}

-- Esferas: marmore, espelho, vidro e uma bola com xadrez (o xadrez e 3D, funciona em qualquer forma)
esfera { centro = {-1.9, 0.8, 0}, raio = 0.8, material = difuso(imagem("marmore.png")) }
esfera { centro = { 0.0, 0.8, -0.4}, raio = 0.8, material = metal({0.92, 0.92, 0.95}, 0.0) }
esfera { centro = { 1.9, 0.8, 0}, raio = 0.8, material = vidro(1.5) }
esfera { centro = { 0.9, 0.35, 1.5}, raio = 0.35, material = difuso(xadrez({0.9, 0.35, 0.1}, {0.95, 0.9, 0.85}, 0.15)) }

-- Um painel de luz no alto, de frente para a cena
quad { canto = {-1.5, 3.6, 0.5}, u = {3, 0, 0}, v = {0, 0, 1.2}, material = luz {3, 2.9, 2.7} }

camera { posicao = {0, 1.7, 6.5}, alvo = {0, 0.8, 0}, campo_de_visao = 40 }
