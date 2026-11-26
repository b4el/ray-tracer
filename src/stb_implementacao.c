/* Implementacao das bibliotecas stb (dominio publico, github.com/nothings/stb).
 * Elas vem num unico arquivo .h; este .c "liga" o codigo delas uma vez so no projeto. */

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
