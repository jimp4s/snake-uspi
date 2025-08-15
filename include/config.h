#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// Tipos básicos para compatibilidade com USPI (DEVE vir ANTES de qualquer include USPI)
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef bool boolean;

// Configurações de tela
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CELL_SIZE 16
#define GAME_WIDTH (SCREEN_WIDTH / CELL_SIZE)
#define GAME_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)

// Configurações do jogo
#define MAX_SNAKE_LENGTH 100
#define INITIAL_SNAKE_LENGTH 3
#define FOOD_SPAWN_RETRIES 10
#define POINTS_PER_FOOD 10
#define INPUT_DEBOUNCE_MS 150
#define GAME_SPEED_MS 200

// Cores (RGB565 format)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_GRAY      0x7BEF
#define COLOR_DARKGRAY  0x39E7

// Cores do jogo
#define BACKGROUND_COLOR    COLOR_BLACK
#define SNAKE_HEAD_COLOR    COLOR_GREEN
#define SNAKE_BODY_COLOR    COLOR_DARKGRAY
#define FOOD_COLOR          COLOR_RED
#define BORDER_COLOR        COLOR_WHITE
#define TEXT_COLOR          COLOR_WHITE
#define PAUSE_BG_COLOR      COLOR_GRAY

// Definições de teclas (USB HID keycodes)
#define KEY_UP_1        0x52  // Arrow Up
#define KEY_UP_2        0x1A  // W
#define KEY_DOWN_1      0x51  // Arrow Down
#define KEY_DOWN_2      0x16  // S
#define KEY_LEFT_1      0x50  // Arrow Left
#define KEY_LEFT_2      0x04  // A
#define KEY_RIGHT_1     0x4F  // Arrow Right
#define KEY_RIGHT_2     0x07  // D
#define KEY_RESTART_1   0x15  // R
#define KEY_RESTART_2   0x09  // F
#define KEY_QUIT_1      0x29  // ESC
#define KEY_QUIT_2      0x14  // Q
#define KEY_PAUSE_1     0x2C  // SPACE
#define KEY_PAUSE_2     0x13  // P

// Tipos básicos para compatibilidade com USPI (movido para o topo)
// typedef uint8_t u8;
// typedef uint16_t u16;
// typedef uint32_t u32;
// typedef bool boolean;

// Estruturas do jogo
typedef struct {
    int x, y;
} Position;

typedef enum {
    DIR_UP = 0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef enum {
    GAME_RUNNING = 0,
    GAME_PAUSED,
    GAME_OVER
} GameState;

typedef struct {
    Position body[MAX_SNAKE_LENGTH];
    int length;
    Direction direction;
    Direction next_direction;
} Snake;

typedef struct {
    Snake snake;
    Position food;
    int score;
    GameState state;
    uint32_t last_update;
} Game;

#endif // CONFIG_H