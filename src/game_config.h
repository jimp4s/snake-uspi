//
// game_config.h - Configurações do jogo Snake
// Permite ajustar facilmente parâmetros do jogo
//

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// ========================
// CONFIGURAÇÕES DE TELA
// ========================
#define SCREEN_WIDTH        800
#define SCREEN_HEIGHT       600
#define BITS_PER_PIXEL      16      // RGB565

// ========================  
// CONFIGURAÇÕES DO JOGO
// ========================
#define GAME_WIDTH          40      // Células horizontais
#define GAME_HEIGHT         30      // Células verticais
#define CELL_SIZE           (SCREEN_WIDTH / GAME_WIDTH)
#define MAX_SNAKE_LENGTH    (GAME_WIDTH * GAME_HEIGHT)

// Velocidade do jogo (ms entre updates)
#define GAME_SPEED_SLOW     300
#define GAME_SPEED_NORMAL   200  
#define GAME_SPEED_FAST     100
#define DEFAULT_GAME_SPEED  GAME_SPEED_NORMAL

// ========================
// CORES (RGB565)
// ========================
#define COLOR_BLACK         0x0000  // 000000
#define COLOR_WHITE         0xFFFF  // FFFFFF
#define COLOR_RED           0xF800  // FF0000
#define COLOR_GREEN         0x07E0  // 00FF00  
#define COLOR_BLUE          0x001F  // 0000FF
#define COLOR_YELLOW        0xFFE0  // FFFF00
#define COLOR_CYAN          0x07FF  // 00FFFF
#define COLOR_MAGENTA       0xF81F  // FF00FF
#define COLOR_GRAY          0x7BEF  // 777777
#define COLOR_DARK_GREEN    0x03E0  // 007700
#define COLOR_DARK_RED      0x7800  // 770000

// Cores específicas do jogo
#define SNAKE_HEAD_COLOR    COLOR_GREEN
#define SNAKE_BODY_COLOR    COLOR_DARK_GREEN
#define FOOD_COLOR          COLOR_RED
#define BACKGROUND_COLOR    COLOR_BLACK
#define GRID_COLOR          COLOR_GRAY
#define TEXT_COLOR          COLOR_WHITE

// ========================
// CONFIGURAÇÕES DE INPUT
// ========================
#define KEY_UP_1            'w'
#define KEY_UP_2            'W'
#define KEY_DOWN_1          's'
#define KEY_DOWN_2          'S'  
#define KEY_LEFT_1          'a'
#define KEY_LEFT_2          'A'
#define KEY_RIGHT_1         'd'
#define KEY_RIGHT_2         'D'
#define KEY_RESTART_1       'r'
#define KEY_RESTART_2       'R'
#define KEY_QUIT_1          'q'
#define KEY_QUIT_2          'Q'
#define KEY_PAUSE_1         'p'
#define KEY_PAUSE_2         'P'

// ========================
// CONFIGURAÇÕES DE SISTEMA
// ========================
#define HEAP_SIZE           0x100000    // 1MB heap
#define STACK_SIZE          0x10000     // 64KB stack
#define FRAMEBUFFER_ADDR    0x3C000000  // Endereço típico do framebuffer

// Timer e delays
#define TIMER_FREQUENCY     1000        // 1000 Hz = 1ms resolution
#define INPUT_DEBOUNCE_MS   50          // Debounce de teclas em ms
#define GAME_UPDATE_MS      DEFAULT_GAME_SPEED

// ========================
// CONFIGURAÇÕES DE DEBUG
// ========================
#define DEBUG_ENABLED       1           // 0 = desabilitado, 1 = habilitado
#define DEBUG_SHOW_FPS      1           // Mostrar FPS (requer implementação)
#define DEBUG_SHOW_COORDS   0           // Mostrar coordenadas da cobra
#define DEBUG_UART_OUTPUT   1           // Debug via UART

// ========================
// CONFIGURAÇÕES AVANÇADAS
// ========================
#define ENABLE_SOUND        0           // Audio não implementado ainda
#define ENABLE_HIGHSCORE    0           // High scores não implementado ainda
#define ENABLE_PAUSE        1           // Funcionalidade de pause
#define ENABLE_GRID         1           // Mostrar grid de fundo

// Configurações de gameplay
#define FOOD_SPAWN_RETRIES  10          // Tentativas para spawnar comida
#define INITIAL_SNAKE_LENGTH 3          // Tamanho inicial da cobra
#define POINTS_PER_FOOD     10          // Pontos por comida
#define SPEED_INCREASE_SCORE 100        // Score para aumentar velocidade

// ========================
// VALIDAÇÕES
// ========================
#if GAME_WIDTH * CELL_SIZE != SCREEN_WIDTH
#warning "GAME_WIDTH * CELL_SIZE não é igual a SCREEN_WIDTH"
#endif

#if GAME_HEIGHT * CELL_SIZE != SCREEN_HEIGHT  
#warning "GAME_HEIGHT * CELL_SIZE não é igual a SCREEN_HEIGHT"
#endif

#if INITIAL_SNAKE_LENGTH > GAME_WIDTH
#error "INITIAL_SNAKE_LENGTH muito grande para GAME_WIDTH"
#endif

#if MAX_SNAKE_LENGTH < 10
#error "MAX_SNAKE_LENGTH muito pequeno"
#endif

#endif // GAME_CONFIG_H