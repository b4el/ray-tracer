-- materiais.lua: uma galeria para comparar os parametros dos materiais.
--   fileira de tras:  metal, da rugosidade 0 (espelho) ate 1 (fosco)
--   fileira do meio:  vidro, do indice 1.0 (ar) ate 2.4 (diamante)
--   fileira da frente: difuso, passando pelas cores
-- Rode com:  raytracer --cena cenas/materiais.lua --saida materiais.bmp

esfera { centro = {0, -1000, 0}, raio = 1000, material = difuso {0.42, 0.44, 0.48} }

local quantidade = 6
local espaco = 1.05
local raio = 0.42

-- Converte matiz (0 a 1) em cor RGB
local function arcoIris(h)
  local function canal(deslocamento)
    local v = math.abs(((h * 6 + deslocamento) % 6) - 3) - 1
    return math.max(0, math.min(1, v))
  end
  return { canal(0), canal(4), canal(2) }
end

-- Posicao x da esfera numero i (de 1 a quantidade), centralizada
local function x(i)
  return (i - (quantidade + 1) / 2) * espaco
end

for i = 1, quantidade do
  local t = (i - 1) / (quantidade - 1)   -- de 0 a 1

  esfera { centro = {x(i), raio, -1.2}, raio = raio, material = metal({0.85, 0.85, 0.88}, t) }
  esfera { centro = {x(i), raio,  0.0}, raio = raio, material = vidro(1.0 + 1.4 * t) }
  esfera {
    centro = {x(i), raio, 1.2}, raio = raio,
    material = difuso(arcoIris(t * 0.83)),   -- 0.83: para nao voltar ao vermelho no fim
  }
end

camera { posicao = {0, 3.6, 5.4}, alvo = {0, 0.2, -0.3}, campo_de_visao = 42 }
