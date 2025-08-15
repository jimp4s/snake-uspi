#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "config.h"

// Variável global do framebuffer
extern uint16_t *framebuffer;

// Inicialização gráfica
void init_graphics(void);

// Funções básicas de desenho
void draw_pixel(int x, int y, uint16_t color);
void graphics_clear_screen(uint16_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint16_t color);
void graphics_draw_rect_outline(int x, int y, int width, int height, uint16_t color);

// Funções de texto
void graphics_draw_char(int x, int y, char c, uint16_t color);
void graphics_draw_string(int x, int y, const char *str, uint16_t color);

// Funções específicas do jogo
void graphics_draw_game_cell(int grid_x, int grid_y, uint16_t color);
void graphics_draw_game_cell_bordered(int grid_x, int grid_y, uint16_t fill_color, uint16_t border_color);

// Buffer management (se necessário para double buffering)
void graphics_swap_buffers(void);

#endif // GRAPHICS_H