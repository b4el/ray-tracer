#pragma once

#include <string>

#include "Cenas.h"

// Cenas descritas em scripts Lua.
//
// O script roda numa "caixa de areia" (sandbox): so tem acesso a matematica,
// texto e tabelas. Nao consegue ler arquivos nem executar comandos.
//
// Funcoes disponiveis no script (veja cenas/exemplo.lua):
//   esfera { centro = {x, y, z}, raio = r, material = ... }
//   modelo { arquivo = "no_toroidal.obj", posicao = {x, y, z}, escala = s, material = ... }
//   camera { posicao = {...}, alvo = {...}, campo_de_visao = graus, abertura = a, distancia_foco = d }
//   ceu(true | false)
//   materiais: difuso {r, g, b} | metal({r, g, b}, rugosidade) | vidro(indice) | luz {r, g, b}

// Executa o arquivo e monta a cena. Lanca std::runtime_error com arquivo e linha se algo der errado.
CenaPronta carregarCenaLua(const std::string& caminho, bool usarBVH = true);

// Mesma coisa, a partir do codigo em texto ("nome" aparece nas mensagens de erro). Usado nos testes.
CenaPronta lerCenaLua(const std::string& codigo, const std::string& nome, bool usarBVH = true);
