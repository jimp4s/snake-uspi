//
// graphics.c - Módulo de gráficos para Snake Game
// Abstração para operações de desenho e framebuffer
//

#include "graphics.h"
#include "game_config.h"
#include <stdint.h>
#include <string.h>

// Ponteiro para framebuffer
static volatile uint16_t *framebuffer = NULL;

// Buffer duplo para reduzir flickering (opcional)
static uint16_t back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static bool double_buffer_enabled = false;

// Inicializar sistema gráfico
bool graphics_init(void) {
    // Configurar ponteiro do framebuffer
    // Em implementação real, isso viria de inicialização do hardware
    framebuffer = (volatile uint16_t*)FRAMEBUFFER_ADDR;
    
    if (!framebuffer) {
        return false;
    }
    
    // Limpar tela inicial
    graphics_clear_screen(BACKGROUND_COLOR);
    
    return true;
}

// Habilitar/desabilitar double buffering
void graphics_set_double_buffer(bool enabled) {
    double_buffer_enabled = enabled;
    if (enabled) {
        // Inicializar back buffer
        memset(back_buffer, 0, sizeof(back_buffer));
    }
}

// Obter ponteiro do buffer atual
uint16_t* graphics_get_current_buffer(void) {
    return double_buffer_enabled ? back_buffer : (uint16_t*)framebuffer;
}

// Trocar buffers (apresentar back buffer)
void graphics_swap_buffers(void) {
    if (double_buffer_enabled && framebuffer) {
        // Copiar back buffer para framebuffer
        memcpy((void*)framebuffer, back_buffer, sizeof(back_buffer));
    }
}

// Desenhar pixel individual
void graphics_draw_pixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return; // Fora dos limites
    }
    
    uint16_t *buffer = graphics_get_current_buffer();
    buffer[y * SCREEN_WIDTH + x] = color;
}

// Desenhar retângulo preenchido
void graphics_draw_rect(int x, int y, int width, int height, uint16_t color) {
    // Clippping
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
    if (x + width < 0 || y + height < 0) return;
    
    int start_x = (x < 0) ? 0 : x;
    int start_y = (y < 0) ? 0 : y;
    int end_x = (x + width > SCREEN_WIDTH) ? SCREEN_WIDTH : x + width;
    int end_y = (y + height > SCREEN_HEIGHT) ? SCREEN_HEIGHT : y + height;
    
    uint16_t *buffer = graphics_get_current_buffer();
    
    for (int dy = start_y; dy < end_y; dy++) {
        for (int dx = start_x; dx < end_x; dx++) {
            buffer[dy * SCREEN_WIDTH + dx] = color;
        }
    }
}

// Desenhar borda de retângulo
void graphics_draw_rect_outline(int x, int y, int width, int height, uint16_t color) {
    // Linhas horizontais
    graphics_draw_rect(x, y, width, 1, color);                    // Top
    graphics_draw_rect(x, y + height - 1, width, 1, color);       // Bottom
    
    // Linhas verticais  
    graphics_draw_rect(x, y, 1, height, color);                   // Left
    graphics_draw_rect(x + width - 1, y, 1, height, color);       // Right
}

// Limpar tela inteira
void graphics_clear_screen(uint16_t color) {
    uint16_t *buffer = graphics_get_current_buffer();
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        buffer[i] = color;
    }
}

// Desenhar linha horizontal
void graphics_draw_hline(int x, int y, int width, uint16_t color) {
    graphics_draw_rect(x, y, width, 1, color);
}

// Desenhar linha vertical
void graphics_draw_vline(int x, int y, int height, uint16_t color) {
    graphics_draw_rect(x, y, 1, height, color);
}

// Desenhar círculo (algoritmo de Bresenham)
void graphics_draw_circle(int center_x, int center_y, int radius, uint16_t color) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (x <= y) {
        // Desenhar 8 pontos simétricos
        graphics_draw_pixel(center_x + x, center_y + y, color);
        graphics_draw_pixel(center_x - x, center_y + y, color);
        graphics_draw_pixel(center_x + x, center_y - y, color);
        graphics_draw_pixel(center_x - x, center_y - y, color);
        graphics_draw_pixel(center_x + y, center_y + x, color);
        graphics_draw_pixel(center_x - y, center_y + x, color);
        graphics_draw_pixel(center_x + y, center_y - x, color);
        graphics_draw_pixel(center_x - y, center_y - x, color);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Desenhar círculo preenchido
void graphics_draw_filled_circle(int center_x, int center_y, int radius, uint16_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                graphics_draw_pixel(center_x + x, center_y + y, color);
            }
        }
    }
}

// Converter RGB888 para RGB565
uint16_t graphics_rgb_to_565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

// Extrair componentes RGB de cor 565
void graphics_565_to_rgb(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = (color >> 11) << 3;
    *g = ((color >> 5) & 0x3F) << 2;
    *b = (color & 0x1F) << 3;
}

// Desenhar grade do jogo
void graphics_draw_grid(uint16_t color) {
    // Linhas verticais
    for (int x = 0; x <= GAME_WIDTH; x++) {
        int px = x * CELL_SIZE;
        graphics_draw_vline(px, 0, SCREEN_HEIGHT, color);
    }
    
    // Linhas horizontais
    for (int y = 0; y <= GAME_HEIGHT; y++) {
        int py = y * CELL_SIZE;
        graphics_draw_hline(0, py, SCREEN_WIDTH, color);
    }
}

// Desenhar célula do jogo (com coordenadas lógicas)
void graphics_draw_game_cell(int game_x, int game_y, uint16_t color) {
    int pixel_x = game_x * CELL_SIZE;
    int pixel_y = game_y * CELL_SIZE;
    
    graphics_draw_rect(pixel_x + 1, pixel_y + 1, 
                      CELL_SIZE - 2, CELL_SIZE - 2, color);
}

// Desenhar célula com borda
void graphics_draw_game_cell_bordered(int game_x, int game_y, 
                                    uint16_t fill_color, uint16_t border_color) {
    int pixel_x = game_x * CELL_SIZE;
    int pixel_y = game_y * CELL_SIZE;
    
    // Borda
    graphics_draw_rect_outline(pixel_x, pixel_y, CELL_SIZE, CELL_SIZE, border_color);
    
    // Preenchimento
    graphics_draw_rect(pixel_x + 1, pixel_y + 1, 
                      CELL_SIZE - 2, CELL_SIZE - 2, fill_color);
}

// Função simples para desenhar texto (bitmap font 8x8)
// Esta é uma implementação muito básica - em projeto real usaria fonte verdadeira
void graphics_draw_char(int x, int y, char c, uint16_t color) {
    // Fonte bitmap 8x8 muito simples para alguns caracteres
    static const uint8_t font8x8[][8] = {
        // '0'
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},
        // '1' 
        {0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E},
        // ... outros caracteres seriam adicionados aqui
    };
    
    if (c >= '0' && c <= '1') { // Suporte limitado apenas 0-1 neste exemplo
        const uint8_t *glyph = font8x8[c - '0'];
        
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (glyph[row] & (1 << (7 - col))) {
                    graphics_draw_pixel(x + col, y + row, color);
                }
            }
        }
    }
}

// Desenhar string simples
void graphics_draw_string(int x, int y, const char *str, uint16_t color) {
    int offset = 0;
    while (*str) {
        graphics_draw_char(x + offset * 8, y, *str, color);
        str++;
        offset++;
    }
}