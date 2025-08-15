//
// Snake Game para Raspberry Pi usando USPI
// Integração do Polisnake com USPI para controle por teclado USB
//

#include "uspi.h"
#include "uspios.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Definições do jogo
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define GAME_WIDTH      40
#define GAME_HEIGHT     30
#define CELL_SIZE       (SCREEN_WIDTH / GAME_WIDTH)
#define MAX_SNAKE_LENGTH (GAME_WIDTH * GAME_HEIGHT)

// Cores (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800
#define COLOR_WHITE     0xFFFF
#define COLOR_BLUE      0x001F

// Direções
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// Estrutura do jogo
typedef struct {
    int x, y;
} Point;

typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    Direction direction;
    Direction next_direction;
} Snake;

typedef struct {
    Point food;
    Snake snake;
    int score;
    bool game_over;
    bool game_running;
} GameState;

// Variáveis globais
static GameState game;
static volatile uint32_t tick_count = 0;
static volatile char last_key = 0;
static bool game_paused = false;
static uint32_t last_input_time = 0;

// Forward declarations
void init_game(void);
void update_game(void);
void draw_game(void);
void handle_input(void);
void spawn_food(void);
bool check_collision(Point p);
void draw_score(void);
void draw_game_over_screen(void);

// Timer callback para USPi
void timer_handler(void) {
    tick_count++;
}

// Keyboard callback para USPi
void keyboard_handler(const char *key) {
    if (key && *key && (tick_count - last_input_time) > INPUT_DEBOUNCE_MS) {
        last_key = *key;
        last_input_time = tick_count;
    }
}

// Inicialização dos gráficos (framebuffer)
void init_graphics(void) {
    // Configurar framebuffer - isso seria específico para cada implementação
    // Aqui assumimos que temos acesso direto à memória de vídeo
    framebuffer = (uint16_t*)0x3C000000; // Endereço típico do framebuffer no RPi
    
    // Limpar tela
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = COLOR_BLACK;
    }
}

// Desenhar pixel
void draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

// Desenhar retângulo
void draw_rect(int x, int y, int w, int h, uint16_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            draw_pixel(x + dx, y + dy, color);
        }
    }
}

// Inicializar estado do jogo
void init_game(void) {
    // Inicializar cobra
    game.snake.length = INITIAL_SNAKE_LENGTH;
    game.snake.direction = DIR_RIGHT;
    game.snake.next_direction = DIR_RIGHT;
    
    game.snake.body[0].x = GAME_WIDTH / 2;
    game.snake.body[0].y = GAME_HEIGHT / 2;
    
    for (int i = 1; i < game.snake.length; i++) {
        game.snake.body[i].x = game.snake.body[0].x - i;
        game.snake.body[i].y = game.snake.body[0].y;
    }
    
    // Inicializar estado do jogo
    game.score = 0;
    game.game_over = false;
    game.game_running = true;
    game_paused = false;
    
    // Spawnar primeira comida
    spawn_food();
}

// Spawnar comida em posição aleatória
void spawn_food(void) {
    int attempts = 0;
    do {
        game.food.x = (tick_count * 7) % GAME_WIDTH;
        game.food.y = (tick_count * 13) % GAME_HEIGHT;
        attempts++;
    } while (check_collision(game.food) && attempts < FOOD_SPAWN_RETRIES);
}

// Verificar colisão
bool check_collision(Point p) {
    // Verificar colisão com paredes
    if (p.x < 0 || p.x >= GAME_WIDTH || p.y < 0 || p.y >= GAME_HEIGHT) {
        return true;
    }
    
    // Verificar colisão com corpo da cobra
    for (int i = 0; i < game.snake.length; i++) {
        if (game.snake.body[i].x == p.x && game.snake.body[i].y == p.y) {
            return true;
        }
    }
    
    return false;
}

// Processar entrada do teclado
void handle_input(void) {
    if (!last_key) return;
    
    Direction new_dir = game.snake.direction;
    
    switch (last_key) {
        case KEY_UP_1:
        case KEY_UP_2:
            if (game.snake.direction != DIR_DOWN) new_dir = DIR_UP;
            break;
        case KEY_DOWN_1:
        case KEY_DOWN_2:
            if (game.snake.direction != DIR_UP) new_dir = DIR_DOWN;
            break;
        case KEY_LEFT_1:
        case KEY_LEFT_2:
            if (game.snake.direction != DIR_RIGHT) new_dir = DIR_LEFT;
            break;
        case KEY_RIGHT_1:
        case KEY_RIGHT_2:
            if (game.snake.direction != DIR_LEFT) new_dir = DIR_RIGHT;
            break;
        case KEY_RESTART_1:
        case KEY_RESTART_2:
            if (game.game_over) {
                init_game();
                last_key = 0;
                return;
            }
            break;
        case KEY_QUIT_1:
        case KEY_QUIT_2:
            game.game_running = false;
            last_key = 0;
            return;
        case KEY_PAUSE_1:
        case KEY_PAUSE_2:
            if (!game.game_over) {
                game_paused = !game_paused;
            }
            break;
    }
    
    if (!game.game_over && !game_paused) {
        game.snake.next_direction = new_dir;
    }
    
    last_key = 0; // Limpar tecla processada
}

// Atualizar lógica do jogo
void update_game(void) {
    if (game.game_over || game_paused) {
        return;
    }
    
    // Atualizar direção
    game.snake.direction = game.snake.next_direction;
    
    // Calcular nova posição da cabeça
    Point new_head = game.snake.body[0];
    
    switch (game.snake.direction) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
    }
    
    // Verificar colisão
    if (check_collision(new_head)) {
        game.game_over = true;
        return;
    }
    
    // Verificar se comeu a comida
    bool ate_food = (new_head.x == game.food.x && new_head.y == game.food.y);
    
    if (ate_food) {
        game.score += POINTS_PER_FOOD;
        spawn_food();
        
        // Aumentar tamanho da cobra
        if (game.snake.length < MAX_SNAKE_LENGTH) {
            game.snake.length++;
        }
    }
    
    // Mover corpo da cobra
    for (int i = game.snake.length - 1; i > 0; i--) {
        game.snake.body[i] = game.snake.body[i - 1];
    }
    
    // Mover cabeça
    game.snake.body[0] = new_head;
}

// Desenhar o jogo
void draw_game(void) {
    // Limpar tela
    graphics_clear_screen(BACKGROUND_COLOR);
    
    // Desenhar grade se habilitada
    #if ENABLE_GRID
    graphics_draw_grid(GRID_COLOR);
    #endif
    
    // Desenhar cobra
    for (int i = 0; i < game.snake.length; i++) {
        uint16_t color = (i == 0) ? SNAKE_HEAD_COLOR : SNAKE_BODY_COLOR;
        graphics_draw_game_cell(game.snake.body[i].x, game.snake.body[i].y, color);
    }
    
    // Desenhar comida
    graphics_draw_game_cell_bordered(game.food.x, game.food.y, FOOD_COLOR, COLOR_WHITE);
    
    // Desenhar score
    draw_score();
    
    // Desenhar tela de game over se necessário
    if (game.game_over) {
        draw_game_over_screen();
    }
    
    // Desenhar indicador de pause
    if (game_paused && !game.game_over) {
        graphics_draw_rect(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 20, 
                          100, 40, COLOR_WHITE);
        graphics_draw_rect(SCREEN_WIDTH/2 - 48, SCREEN_HEIGHT/2 - 18, 
                          96, 36, COLOR_BLACK);
        graphics_draw_string(SCREEN_WIDTH/2 - 32, SCREEN_HEIGHT/2 - 8, 
                           "PAUSED", COLOR_WHITE);
    }
    
    // Apresentar buffer se double buffering estiver habilitado
    graphics_swap_buffers();
}

// Desenhar score
void draw_score(void) {
    // Área do score no topo da tela
    graphics_draw_rect(0, 0, SCREEN_WIDTH, 30, COLOR_BLACK);
    graphics_draw_string(10, 10, "Score: ", COLOR_WHITE);
    
    // Converter score para string (implementação simples)
    char score_str[16];
    int score = game.score;
    int pos = 0;
    
    if (score == 0) {
        score_str[pos++] = '0';
    } else {
        // Converter número para string (ordem reversa)
        char temp[16];
        int temp_pos = 0;
        while (score > 0) {
            temp[temp_pos++] = '0' + (score % 10);
            score /= 10;
        }
        // Reverter ordem
        for (int i = temp_pos - 1; i >= 0; i--) {
            score_str[pos++] = temp[i];
        }
    }
    score_str[pos] = '\0';
    
    graphics_draw_string(70, 10, score_str, COLOR_WHITE);
}

// Desenhar tela de game over
void draw_game_over_screen(void) {
    // Fundo semi-transparente (simulado)
    graphics_draw_rect(SCREEN_WIDTH/4, SCREEN_HEIGHT/2 - 80, 
                      SCREEN_WIDTH/2, 160, COLOR_WHITE);
    graphics_draw_rect(SCREEN_WIDTH/4 + 2, SCREEN_HEIGHT/2 - 78, 
                      SCREEN_WIDTH/2 - 4, 156, COLOR_BLACK);
    
    // Texto "GAME OVER"
    graphics_draw_string(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 - 60, 
                        "GAME OVER", COLOR_RED);
    
    // Score final
    graphics_draw_string(SCREEN_WIDTH/2 - 35, SCREEN_HEIGHT/2 - 30, 
                        "Final Score:", COLOR_WHITE);
    
    // Instruções
    graphics_draw_string(SCREEN_WIDTH/2 - 45, SCREEN_HEIGHT/2 + 20, 
                        "Press R to restart", COLOR_WHITE);
    graphics_draw_string(SCREEN_WIDTH/2 - 35, SCREEN_HEIGHT/2 + 40, 
                        "Press Q to quit", COLOR_WHITE);
}

// Função principal
int main(void) {
    // Inicializar USPI
    if (!USPiInitialize()) {
        return 1; // Falha na inicialização
    }
    
    // Aguardar inicialização do teclado
    while (!USPiKeyboardAvailable()) {
        USPiMsDelay(100);
    }
    
    // Configurar callback do teclado
    USPiKeyboardRegisterKeyStatusHandlerRaw(keyboard_handler);
    
    // Inicializar gráficos e jogo
    init_graphics();
    init_game();
    
    uint32_t last_update = 0;
    const uint32_t update_interval = 20; // ~50 FPS
    
    // Loop principal do jogo
    while (game.game_running) {
        // Atualizar USPi
        boolean keyboard_updated = USPiKeyboardUpdate();
        
        // Processar entrada
        if (keyboard_updated || last_key != 0) {
            handle_input();
        }
        
        // Atualizar jogo em intervalos regulares
        if (tick_count - last_update >= update_interval) {
            update_game();
            draw_game();
            last_update = tick_count;
        }
        
        // Pequeno delay para não sobrecarregar CPU
        USPiMsDelay(10);
    }
    
    return 0;
}

// Implementações necessárias para USPi (uspios.h)

void *malloc(size_t size) {
    // Implementação simples de malloc
    static char heap[0x100000]; // 1MB heap
    static size_t heap_pos = 0;
    
    if (heap_pos + size > sizeof(heap)) {
        return 0;
    }
    
    void *ptr = &heap[heap_pos];
    heap_pos += (size + 7) & ~7; // Align to 8 bytes
    return ptr;
}

void free(void *ptr) {
    // Implementação simples - não faz nada
    (void)ptr;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    // Implementação simples
    if (!ptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return 0;
    }
    
    // Para simplicidade, sempre aloca novo bloco
    void *new_ptr = malloc(size);
    if (new_ptr && ptr) {
        // Copiar dados existentes (assumindo tamanho menor)
        memcpy(new_ptr, ptr, size);
    }
    return new_ptr;
}

// Timer functions para USPi
void uspi_assertion_failed(const char *expr, const char *file, unsigned line) {
    // Handle assertion failure
    while (1) {} // Halt
}

void uspi_log_write(const char *source, unsigned severity, const char *message, ...) {
    // Log function - can be empty for now
}

unsigned uspi_get_ticks(void) {
    return tick_count;
}

void uspi_ms_delay(unsigned ms) {
    unsigned start = tick_count;
    while (tick_count - start < ms) {
        // Wait
    }
}