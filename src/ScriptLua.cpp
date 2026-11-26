#include "ScriptLua.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>

// O Lua e escrito em C: os cabecalhos precisam de extern "C" quando usados em C++
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

// IMPORTANTE sobre erros: quando uma funcao chamada pelo Lua usa luaL_error, o Lua
// "pula" de volta (longjmp) sem rodar destrutores de objetos C++. Por isso, dentro
// das funcoes abaixo, so guardamos tipos simples (double, Vec3, const char*) ate
// terminar todas as validacoes, e so depois criamos std::string ou mexemos em vetores.

namespace {

// Limite de instrucoes: protege contra scripts com laco infinito
constexpr int INSTRUCOES_POR_VERIFICACAO = 100000;
constexpr int VERIFICACOES_MAXIMAS = 2000;  // ~200 milhoes de instrucoes

// Funcoes de material escritas no proprio Lua: so montam tabelas simples
// que o C++ le depois. Mais facil de entender (e de mudar) do que em C.
const char* PRELUDIO = R"LUA(
function difuso(cor)
  return { tipo = "difuso", cor = cor }
end

function metal(cor, rugosidade)
  return { tipo = "metal", cor = cor, rugosidade = rugosidade or 0 }
end

function vidro(indice)
  return { tipo = "vidro", indice = indice or 1.5 }
end

function luz(cor)
  return { tipo = "luz", cor = cor }
end

-- Texturas: podem ser usadas no lugar da cor de um difuso, ex.: difuso(xadrez(...))
function xadrez(cor1, cor2, escala)
  return { tipo = "xadrez", cor1 = cor1, cor2 = cor2, escala = escala or 1 }
end

function imagem(arquivo)
  return { tipo = "imagem", arquivo = arquivo }
end

-- Um chao enorme na altura indicada, feito com quad{}
function chao(t)
  local y = t.altura or 0
  quad { canto = {-1000, y, -1000}, u = {2000, 0, 0}, v = {0, 0, 2000}, material = t.material }
end
)LUA";

// A cena sendo montada fica guardada como "upvalue" de cada funcao registrada
CenaPronta& cenaAtual(lua_State* L) { return *static_cast<CenaPronta*>(lua_touserdata(L, lua_upvalueindex(1))); }

// Le um numero do campo t[campo]. Se nao existir, usa o padrao (ou da erro se obrigatorio).
double lerNumero(lua_State* L, int tabela, const char* funcao, const char* campo, bool obrigatorio, double padrao) {
    lua_getfield(L, tabela, campo);
    double valor = padrao;
    if (lua_isnil(L, -1)) {
        if (obrigatorio) luaL_error(L, "%s: falta o campo '%s'", funcao, campo);
    } else if (lua_isnumber(L, -1)) {
        valor = lua_tonumber(L, -1);
    } else {
        luaL_error(L, "%s: o campo '%s' precisa ser um numero", funcao, campo);
    }
    lua_pop(L, 1);
    return valor;
}

// Le {x, y, z} da tabela que esta na posicao 'indice' da pilha
Vec3 lerTripla(lua_State* L, int indice, const char* funcao, const char* campo) {
    if (!lua_istable(L, indice)) luaL_error(L, "%s: o campo '%s' precisa ser {x, y, z}", funcao, campo);
    double v[3];
    for (int i = 0; i < 3; i++) {
        lua_rawgeti(L, indice, i + 1);
        if (!lua_isnumber(L, -1))
            luaL_error(L, "%s: o campo '%s' precisa ter 3 numeros, como {1, 2, 3}", funcao, campo);
        v[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    return {v[0], v[1], v[2]};
}

Vec3 lerVetor(lua_State* L, int tabela, const char* funcao, const char* campo, bool obrigatorio, Vec3 padrao) {
    lua_getfield(L, tabela, campo);
    Vec3 valor = padrao;
    if (lua_isnil(L, -1)) {
        if (obrigatorio) luaL_error(L, "%s: falta o campo '%s'", funcao, campo);
    } else {
        valor = lerTripla(L, lua_gettop(L), funcao, campo);
    }
    lua_pop(L, 1);
    return valor;
}

// Material lido do Lua, so com tipos simples (seguro se o Lua der erro no meio da leitura).
// Vira um DescricaoMaterial (que tem std::string) apenas depois de tudo validado.
struct MaterialLido {
    DescricaoMaterial::Tipo tipo = DescricaoMaterial::Tipo::Difuso;
    Cor cor{0.5, 0.5, 0.5};
    double rugosidade = 0;
    double indice = 1.5;
    DescricaoTextura::Tipo textura = DescricaoTextura::Tipo::Nenhuma;
    Cor cor2{0, 0, 0};
    double escala = 1;
    const char* arquivo = nullptr;  // aponta para a string do Lua (continua viva na tabela do script)

    DescricaoMaterial paraDescricao() const {
        DescricaoMaterial d;
        d.tipo = tipo;
        d.cor = cor;
        d.rugosidade = rugosidade;
        d.indice = indice;
        d.textura.tipo = textura;
        d.textura.cor2 = cor2;
        d.textura.escala = escala;
        if (arquivo != nullptr) d.textura.arquivo = arquivo;
        return d;
    }
};

// So aceita um nome de arquivo simples, sem pastas: a imagem e sempre lida de texturas/
bool nomeDeArquivoSeguro(const char* nome) {
    std::string_view n(nome);
    return !n.empty() && n.find("..") == std::string_view::npos && n.find_first_of("/\\:") == std::string_view::npos;
}

// Le a "cor" de um difuso: uma cor {r, g, b} ou uma textura (xadrez(...) ou imagem("..."))
void lerCorOuTextura(lua_State* L, int material, const char* funcao, MaterialLido& m) {
    lua_getfield(L, material, "cor");
    int cor = lua_gettop(L);
    if (lua_istable(L, cor)) {
        lua_getfield(L, cor, "tipo");
        const char* tipo = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (tipo != nullptr && std::string_view(tipo) == "xadrez") {
            m.textura = DescricaoTextura::Tipo::Xadrez;
            m.cor = lerVetor(L, cor, "xadrez", "cor1", true, {});
            m.cor2 = lerVetor(L, cor, "xadrez", "cor2", true, {});
            m.escala = lerNumero(L, cor, "xadrez", "escala", false, 1);
            if (m.escala <= 0) luaL_error(L, "xadrez: a escala precisa ser maior que zero");
        } else if (tipo != nullptr && std::string_view(tipo) == "imagem") {
            lua_getfield(L, cor, "arquivo");
            const char* arquivo = lua_tostring(L, -1);
            lua_pop(L, 1);
            if (arquivo == nullptr || !nomeDeArquivoSeguro(arquivo))
                luaL_error(L, "imagem: use so o nome do arquivo que esta na pasta texturas/ (ex.: \"marmore.png\")");
            m.textura = DescricaoTextura::Tipo::Imagem;
            m.cor = {1, 1, 1};
            m.arquivo = arquivo;
        } else if (tipo != nullptr) {
            luaL_error(L, "%s: textura desconhecida: %s", funcao, tipo);
        } else {
            m.cor = lerTripla(L, cor, funcao, "cor");
        }
    } else {
        luaL_error(L, "%s: a cor precisa ser {r, g, b}, xadrez(...) ou imagem(\"...\")", funcao);
    }
    lua_pop(L, 1);
}

// Le t.material (uma tabela criada por difuso/metal/vidro/luz)
MaterialLido lerMaterial(lua_State* L, int tabela, const char* funcao) {
    lua_getfield(L, tabela, "material");
    if (!lua_istable(L, -1))
        luaL_error(L, "%s: falta o campo 'material' (use difuso{...}, metal(...), vidro(...) ou luz{...})", funcao);
    int m = lua_gettop(L);

    lua_getfield(L, m, "tipo");
    const char* tipo = lua_tostring(L, -1);
    if (tipo == nullptr) luaL_error(L, "%s: material invalido", funcao);

    MaterialLido d;
    using Tipo = DescricaoMaterial::Tipo;
    if (std::string_view(tipo) == "difuso")
        d.tipo = Tipo::Difuso;
    else if (std::string_view(tipo) == "metal")
        d.tipo = Tipo::Metal;
    else if (std::string_view(tipo) == "vidro")
        d.tipo = Tipo::Vidro;
    else if (std::string_view(tipo) == "luz")
        d.tipo = Tipo::Luz;
    else
        luaL_error(L, "%s: tipo de material desconhecido: %s", funcao, tipo);
    lua_pop(L, 1);

    if (d.tipo == Tipo::Difuso) lerCorOuTextura(L, m, funcao, d);
    if (d.tipo == Tipo::Metal || d.tipo == Tipo::Luz) d.cor = lerVetor(L, m, funcao, "cor", true, {});
    if (d.tipo == Tipo::Metal) d.rugosidade = lerNumero(L, m, funcao, "rugosidade", false, 0);
    if (d.tipo == Tipo::Vidro) {
        d.indice = lerNumero(L, m, funcao, "indice", false, 1.5);
        if (d.indice <= 0) luaL_error(L, "%s: o indice do vidro precisa ser positivo", funcao);
    }
    lua_pop(L, 1);  // tabela do material
    return d;
}

// esfera { centro = {x, y, z}, raio = r, material = ... }
int luaEsfera(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    Vec3 centro = lerVetor(L, 1, "esfera", "centro", true, {});
    double raio = lerNumero(L, 1, "esfera", "raio", true, 1);
    if (raio <= 0) luaL_error(L, "esfera: o raio precisa ser maior que zero");
    MaterialLido material = lerMaterial(L, 1, "esfera");

    // Tudo validado: agora e seguro criar objetos C++ e mexer no vetor
    cenaAtual(L).esferas.push_back({centro, raio, material.paraDescricao()});
    return 0;
}

// quad { canto = {x, y, z}, u = {x, y, z}, v = {x, y, z}, material = ... }
int luaQuad(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    Vec3 canto = lerVetor(L, 1, "quad", "canto", true, {});
    Vec3 u = lerVetor(L, 1, "quad", "u", true, {});
    Vec3 v = lerVetor(L, 1, "quad", "v", true, {});
    if (cross(u, v).quaseZero()) luaL_error(L, "quad: os lados 'u' e 'v' nao podem ser paralelos nem zero");
    MaterialLido material = lerMaterial(L, 1, "quad");

    cenaAtual(L).quads.push_back({canto, u, v, material.paraDescricao()});
    return 0;
}

// modelo { arquivo = "no_toroidal.obj", posicao = {x, y, z}, escala = s, material = ... }
int luaModelo(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "arquivo");
    const char* arquivo = lua_tostring(L, -1);
    if (arquivo == nullptr) luaL_error(L, "modelo: falta o campo 'arquivo' (ex.: \"no_toroidal.obj\")");
    Vec3 posicao = lerVetor(L, 1, "modelo", "posicao", false, {0, 0, 0});
    double escala = lerNumero(L, 1, "modelo", "escala", false, 1);
    if (escala <= 0) luaL_error(L, "modelo: a escala precisa ser maior que zero");
    MaterialLido material = lerMaterial(L, 1, "modelo");

    DescricaoMalha m;
    m.arquivo = arquivo;  // a string do Lua continua valida porque ainda esta na pilha
    m.deslocamento = posicao;
    m.escala = escala;
    m.material = material.paraDescricao();
    cenaAtual(L).malhas.push_back(std::move(m));
    lua_pop(L, 1);
    return 0;
}

// camera { posicao = {...}, alvo = {...}, cima = {...}, campo_de_visao = 40, abertura = 0, distancia_foco = 8 }
int luaCamera(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    ConfigCamera c = cenaAtual(L).camera;
    c.posicao = lerVetor(L, 1, "camera", "posicao", false, c.posicao);
    c.alvo = lerVetor(L, 1, "camera", "alvo", false, c.alvo);
    c.cima = lerVetor(L, 1, "camera", "cima", false, c.cima);
    c.campoDeVisao = lerNumero(L, 1, "camera", "campo_de_visao", false, c.campoDeVisao);
    c.abertura = lerNumero(L, 1, "camera", "abertura", false, c.abertura);
    c.distanciaFoco = lerNumero(L, 1, "camera", "distancia_foco", false, (c.posicao - c.alvo).comprimento());
    if (c.campoDeVisao <= 0 || c.campoDeVisao >= 180)
        luaL_error(L, "camera: campo_de_visao precisa estar entre 0 e 180 graus");
    if ((c.posicao - c.alvo).quaseZero()) luaL_error(L, "camera: posicao e alvo nao podem ser o mesmo ponto");
    cenaAtual(L).camera = c;
    return 0;
}

// ceu(true) ou ceu(false)
int luaCeu(lua_State* L) {
    luaL_checktype(L, 1, LUA_TBOOLEAN);
    cenaAtual(L).ceu = lua_toboolean(L, 1) != 0;
    return 0;
}

// Chamado de tempos em tempos pelo Lua: interrompe scripts que nunca terminam
void verificarTempo(lua_State* L, lua_Debug*) {
    lua_getfield(L, LUA_REGISTRYINDEX, "verificacoes");
    lua_Integer n = lua_tointeger(L, -1) + 1;
    lua_pop(L, 1);
    lua_pushinteger(L, n);
    lua_setfield(L, LUA_REGISTRYINDEX, "verificacoes");
    if (n > VERIFICACOES_MAXIMAS) luaL_error(L, "o script demorou demais (laco infinito?)");
}

// Abre so as bibliotecas seguras: nada de io, os, package ou debug
void abrirBibliotecasSeguras(lua_State* L) {
    const luaL_Reg seguras[] = {
        {LUA_GNAME, luaopen_base},       {LUA_MATHLIBNAME, luaopen_math}, {LUA_STRLIBNAME, luaopen_string},
        {LUA_TABLIBNAME, luaopen_table}, {LUA_UTF8LIBNAME, luaopen_utf8},
    };
    for (const luaL_Reg& lib : seguras) {
        luaL_requiref(L, lib.name, lib.func, 1);
        lua_pop(L, 1);
    }
    // A biblioteca base tem funcoes que leem arquivos do disco: removemos
    for (const char* proibida : {"dofile", "loadfile"}) {
        lua_pushnil(L);
        lua_setglobal(L, proibida);
    }
}

std::string mensagemDeErro(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    return msg ? msg : "erro desconhecido no script";
}

}  // namespace

CenaPronta lerCenaLua(const std::string& codigo, const std::string& nome, bool usarBVH) {
    // unique_ptr com lua_close: o Lua e fechado mesmo se uma excecao for lancada
    std::unique_ptr<lua_State, decltype(&lua_close)> estado(luaL_newstate(), &lua_close);
    if (!estado) throw std::runtime_error("Nao foi possivel iniciar o Lua.");
    lua_State* L = estado.get();

    CenaPronta cena;
    cena.nome = nome;
    cena.camera.posicao = {0, 2, 8};  // padroes pensados para cenas pequenas
    cena.camera.alvo = {0, 0.5, 0};
    cena.camera.campoDeVisao = 35;
    cena.camera.distanciaFoco = (cena.camera.posicao - cena.camera.alvo).comprimento();

    abrirBibliotecasSeguras(L);

    // Registra as funcoes da cena, cada uma com um ponteiro para 'cena' guardado como upvalue
    const luaL_Reg funcoes[] = {
        {"esfera", luaEsfera}, {"quad", luaQuad}, {"modelo", luaModelo}, {"camera", luaCamera}, {"ceu", luaCeu}};
    for (const luaL_Reg& f : funcoes) {
        lua_pushlightuserdata(L, &cena);
        lua_pushcclosure(L, f.func, 1);
        lua_setglobal(L, f.name);
    }

    if (luaL_dostring(L, PRELUDIO) != LUA_OK)
        throw std::runtime_error("Erro interno no preludio Lua: " + mensagemDeErro(L));

    // Protecao contra laco infinito
    lua_pushinteger(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, "verificacoes");
    lua_sethook(L, verificarTempo, LUA_MASKCOUNT, INSTRUCOES_POR_VERIFICACAO);

    // "t" = so aceita texto (nunca bytecode pre-compilado, que pode ser malicioso)
    std::string nomeDoTrecho = "@" + nome;
    if (luaL_loadbufferx(L, codigo.data(), codigo.size(), nomeDoTrecho.c_str(), "t") != LUA_OK ||
        lua_pcall(L, 0, 0, 0) != LUA_OK) {
        throw std::runtime_error("Erro na cena Lua: " + mensagemDeErro(L));
    }

    if (cena.esferas.empty() && cena.malhas.empty() && cena.quads.empty())
        throw std::runtime_error("A cena " + nome + " nao criou nenhum objeto (use esfera{...} ou modelo{...}).");

    montarMundo(cena, usarBVH);
    return cena;
}

CenaPronta carregarCenaLua(const std::string& caminho, bool usarBVH) {
    std::ifstream arq(caminho);
    if (!arq) throw std::runtime_error("Nao foi possivel abrir a cena " + caminho);
    std::stringstream conteudo;
    conteudo << arq.rdbuf();
    return lerCenaLua(conteudo.str(), caminho, usarBVH);
}
