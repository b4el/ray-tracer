-- espiral.lua: uma dupla helice (como um DNA) gerada com lacos e trigonometria.
-- Mostra a vantagem de descrever cenas com codigo: 120 objetos em poucas linhas.
-- Rode com:  raytracer --cena cenas/espiral.lua --saida espiral.bmp

-- Converte matiz (0 a 1) em cor RGB, para fazer um arco-iris ao longo da helice
local function arcoIris(h)
  local function canal(deslocamento)
    local x = math.abs(((h * 6 + deslocamento) % 6) - 3) - 1
    return math.max(0, math.min(1, x))
  end
  return { canal(0), canal(4), canal(2) }
end

esfera { centro = {0, -1000, 0}, raio = 1000, material = difuso {0.35, 0.37, 0.42} }

local voltas = 2.5
local passos = 48
local altura = 4.2
local raio = 1.2

for i = 0, passos - 1 do
  local t = i / (passos - 1)              -- vai de 0 a 1 ao longo da helice
  local angulo = t * voltas * 2 * math.pi
  local y = 0.35 + t * altura

  -- As duas fitas, uma de frente para a outra (meia volta de diferenca)
  local a = { raio * math.cos(angulo), y, raio * math.sin(angulo) }
  local b = { -a[1], y, -a[3] }

  esfera { centro = a, raio = 0.22, material = difuso(arcoIris(t)) }
  esfera { centro = b, raio = 0.22, material = metal(arcoIris(1 - t), 0.1) }

  -- A cada 4 passos, uma "ponte" de vidro ligando as fitas
  if i % 4 == 0 then
    for k = 1, 3 do
      local f = k / 4
      esfera {
        centro = { a[1] + (b[1] - a[1]) * f, y, a[3] + (b[3] - a[3]) * f },
        raio = 0.09,
        material = vidro(1.5),
      }
    end
  end
end

camera { posicao = {0, 3.2, 9}, alvo = {0, 2.3, 0}, campo_de_visao = 38 }
