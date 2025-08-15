//
// graphics.h - Header do módulo de gráficos
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

// Inicialização
bool graphics_init(void);

// Controle de buffer duplo
void graphics_set_double_buffer(bool enabled);
uint16_t* graphics_get_current_buffer(void);
void graphics_swap_buffers(void);

// Operações básicas de desenho
void graphics_draw_pixel(int x, int y, uint16_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint16_t color);
void graphics_draw_rect_outline(int x, int y, int width, int height, uint16_t color);
void graphics_clear_screen(uint16_t color);

// Linhas
void graphics_draw_hline(int x, int y, int width, uint16_t color);
void graphics_draw_vline(int x, int y, int height, uint16_t color);

// Círculos
void graphics_draw_circle(int center_x, int center_y, int radius, uint16_t color);
void graphics_draw_filled_circle(int center_x, int center_y, int radius, uint16_t color);

// Conversões de cor
uint16_t graphics_rgb_to_565(uint8_t r, uint8_t g, uint8_t b);
void graphics_565_to_rgb(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b);

// Funções específicas do jogo
void graphics_draw_grid(uint16_t color);
void graphics_draw_game_cell(int game_x, int game_y, uint16_t color);
void graphics_draw_game_cell_bordered(int game_x, int game_y, uint16_t fill_color, uint16_t border_color);

// Texto simples
void graphics_draw_char(int x, int y, char c, uint16_t color);
void graphics_draw_string(int x, int y, const char *str, uint16_t color);

#endif // GRAPHICS_H