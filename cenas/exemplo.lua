-- exemplo.lua: a menor cena possivel, comentada linha a linha.
-- Rode com:  raytracer --cena cenas/exemplo.lua --saida exemplo.bmp
--
-- Uma cena e so um programa Lua que chama algumas funcoes:
--   esfera{...}   adiciona uma esfera
--   modelo{...}   adiciona um modelo 3D (.obj da pasta modelos/)
--   camera{...}   posiciona a camera
--   ceu(...)      liga ou desliga o ceu azul
-- e os materiais: difuso{r, g, b}, metal({r, g, b}, rugosidade), vidro(indice), luz{r, g, b}.
-- Cores vao de 0 a 1: {1, 0, 0} e vermelho, {1, 1, 1} e branco.

-- O chao: uma esfera gigante, com o topo em y = 0
esfera { centro = {0, -1000, 0}, raio = 1000, material = difuso {0.5, 0.5, 0.5} }

-- Tres esferas lado a lado
esfera { centro = {-1.6, 0.6, 0}, raio = 0.6, material = vidro(1.5) }
esfera { centro = { 0.0, 0.6, 0}, raio = 0.6, material = difuso {0.9, 0.3, 0.2} }
esfera { centro = { 1.6, 0.6, 0}, raio = 0.6, material = metal({0.9, 0.8, 0.5}, 0.05) }

-- Um modelo 3D pequeno na frente
modelo { arquivo = "gema.obj", posicao = {0, 0, 1.6}, escala = 0.5, material = metal({0.6, 0.8, 1.0}, 0.0) }

-- A camera: de onde ela olha e para onde
camera { posicao = {0, 1.6, 6}, alvo = {0, 0.5, 0}, campo_de_visao = 35 }
