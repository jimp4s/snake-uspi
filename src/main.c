#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "graphics.h"
#include <uspi.h>

// Variáveis globais
Game game;
uint32_t tick_count = 0;
uint32_t last_input_time = 0;
extern void init_system(void);
extern void ProcessKernelTimers(void);

// Declarações de funções
void handle_input(unsigned char key);
void spawn_food(void);
bool check_collision(Position pos);
void init_game(void);
void update_game(void);
void draw_game(void);
void delay_ms(unsigned int ms);
void debug_print_game_state(void);
void init_random(void);
void init_graphics_system(void);
uint32_t get_ticks(void);
void timer_handler(void);
void keyboard_handler(unsigned char ucModifiers, const unsigned char *pKeys);

// Função para obter timestamp (implementação básica)
uint32_t get_ticks(void) {
    return tick_count++;
}

// Timer callback para incrementar ticks
void timer_handler(void) {
    tick_count++;
}

// Handler corrigido para teclado (assinatura correta para USPI)
void keyboard_handler(unsigned char ucModifiers, const unsigned char *pKeys) {
    uint32_t current_time = get_ticks();
    
    // Debounce de entrada
    if ((current_time - last_input_time) > INPUT_DEBOUNCE_MS) {
        // Verifica se alguma tecla foi pressionada
        for (int i = 0; i < 6 && pKeys[i] != 0; i++) {
            unsigned char key = pKeys[i];
            
            if (key) {
                handle_input(key);
                last_input_time = current_time;
                break; // Processa apenas a primeira tecla
            }
        }
    }
}

// Inicialização gráfica
void init_graphics_system(void) {
    init_graphics();
}

// Verificar colisão
bool check_collision(Position pos) {
    // Verificar colisão com bordas
    if (pos.x < 0 || pos.x >= GAME_WIDTH || pos.y < 0 || pos.y >= GAME_HEIGHT) {
        return true;
    }
    
    // Verificar colisão com o corpo da cobra
    for (int i = 1; i < game.snake.length; i++) {
        if (game.snake.body[i].x == pos.x && game.snake.body[i].y == pos.y) {
            return true;
        }
    }
    
    return false;
}

// Inicializar jogo
void init_game(void) {
    game.snake.length = INITIAL_SNAKE_LENGTH;
    game.snake.direction = DIR_RIGHT;
    game.snake.next_direction = DIR_RIGHT;
    game.score = 0;
    game.state = GAME_RUNNING;
    game.last_update = 0;
    
    // Posicionar cobra no centro
    int start_x = GAME_WIDTH / 2;
    int start_y = GAME_HEIGHT / 2;
    
    for (int i = 0; i < game.snake.length; i++) {
        game.snake.body[i].x = start_x - i;
        game.snake.body[i].y = start_y;
    }
    
    spawn_food();
}

// Gerar nova comida
void spawn_food(void) {
    int attempts = 0;
    do {
        game.food.x = rand() % GAME_WIDTH;
        game.food.y = rand() % GAME_HEIGHT;
        attempts++;
    } while (check_collision(game.food) && attempts < FOOD_SPAWN_RETRIES);
}

// Tratamento de entrada
void handle_input(unsigned char key) {
    if (game.state == GAME_OVER) {
        switch (key) {
            case KEY_RESTART_1:
            case KEY_RESTART_2:
                init_game();
                return;
            case KEY_QUIT_1:
            case KEY_QUIT_2:
                game.state = GAME_OVER; // Manter no game over ou implementar saída
                return;
        }
        return;
    }
    
    switch (key) {
        case KEY_UP_1:
        case KEY_UP_2:
            if (game.snake.direction != DIR_DOWN) {
                game.snake.next_direction = DIR_UP;
            }
            break;
        case KEY_DOWN_1:
        case KEY_DOWN_2:
            if (game.snake.direction != DIR_UP) {
                game.snake.next_direction = DIR_DOWN;
            }
            break;
        case KEY_LEFT_1:
        case KEY_LEFT_2:
            if (game.snake.direction != DIR_RIGHT) {
                game.snake.next_direction = DIR_LEFT;
            }
            break;
        case KEY_RIGHT_1:
        case KEY_RIGHT_2:
            if (game.snake.direction != DIR_LEFT) {
                game.snake.next_direction = DIR_RIGHT;
            }
            break;
        case KEY_RESTART_1:
        case KEY_RESTART_2:
            init_game();
            break;
        case KEY_QUIT_1:
        case KEY_QUIT_2:
            game.state = GAME_OVER;
            break;
        case KEY_PAUSE_1:
        case KEY_PAUSE_2:
            if (game.state == GAME_RUNNING) {
                game.state = GAME_PAUSED;
            } else if (game.state == GAME_PAUSED) {
                game.state = GAME_RUNNING;
            }
            break;
    }
}

// Atualizar jogo
void update_game(void) {
    if (game.state != GAME_RUNNING) {
        return;
    }
    
    uint32_t current_time = get_ticks();
    if (current_time - game.last_update < GAME_SPEED_MS) {
        return;
    }
    
    game.last_update = current_time;
    
    // Atualizar direção
    game.snake.direction = game.snake.next_direction;
    
    // Calcular nova posição da cabeça
    Position new_head = game.snake.body[0];
    
    switch (game.snake.direction) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
    }
    
    // Verificar colisão
    if (check_collision(new_head)) {
        game.state = GAME_OVER;
        return;
    }
    
    // Mover corpo da cobra
    for (int i = game.snake.length - 1; i > 0; i--) {
        game.snake.body[i] = game.snake.body[i - 1];
    }
    game.snake.body[0] = new_head;
    
    // Verificar se comeu comida
    if (new_head.x == game.food.x && new_head.y == game.food.y) {
        if (game.snake.length < MAX_SNAKE_LENGTH) {
            game.snake.length++;
        }
        game.score += POINTS_PER_FOOD;
        spawn_food();
    }
}

// Desenhar jogo
void draw_game(void) {
    graphics_clear_screen(BACKGROUND_COLOR);
    
    // Desenhar cobra
    for (int i = 0; i < game.snake.length; i++) {
        uint16_t color = (i == 0) ? SNAKE_HEAD_COLOR : SNAKE_BODY_COLOR;
        graphics_draw_game_cell(game.snake.body[i].x, game.snake.body[i].y, color);
    }
    
    // Desenhar comida
    graphics_draw_game_cell_bordered(game.food.x, game.food.y, FOOD_COLOR, COLOR_WHITE);
    
    // Desenhar pontuação
    char score_text[32];
    sprintf(score_text, "Score: %d", game.score);
    graphics_draw_string(10, 10, score_text, TEXT_COLOR);
    
    // Desenhar mensagens de estado
    if (game.state == GAME_PAUSED) {
        graphics_draw_rect(SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2 - 20,
                          100, 40, PAUSE_BG_COLOR);
        graphics_draw_string(SCREEN_WIDTH/2 - 32, SCREEN_HEIGHT/2 - 8,
                           "PAUSED", TEXT_COLOR);
    } else if (game.state == GAME_OVER) {
        graphics_draw_rect(SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 30,
                          120, 60, PAUSE_BG_COLOR);
        graphics_draw_string(SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT/2 - 16,
                           "GAME OVER", TEXT_COLOR);
        graphics_draw_string(SCREEN_WIDTH/2 - 48, SCREEN_HEIGHT/2,
                           "Press R to restart", TEXT_COLOR);
    }
    
    graphics_swap_buffers();
}

// Função de delay - implementação robusta
void delay_ms(unsigned int ms) {
    // Implementação de delay básico para bare metal
    // Ajuste o multiplicador conforme a frequência do seu sistema
    for (unsigned int i = 0; i < ms; i++) {
        for (volatile unsigned int j = 0; j < 1000; j++) {
            __asm__ __volatile__("nop");
        }
    }
}

// Função auxiliar para debug (opcional)
void debug_print_game_state(void) {
    printf("Snake pos: (%d,%d), Length: %d, Score: %d, State: %d\n",
           game.snake.body[0].x, game.snake.body[0].y,
           game.snake.length, game.score, game.state);
}

// Função para inicializar sistema de random
void init_random(void) {
    // Usar tick count como seed básico
    srand(get_ticks());
}

int main(void) {
    init_system(); 
    // Inicializar USPI
    printf("Inicializando USPI...\n");
    if (!USPiInitialize()) {
        printf("ERRO: Falha ao inicializar USPI!\n");
        return 1;
    }
    printf("USPI inicializado com sucesso!\n");
    
    // Inicializar sistema gráfico
    printf("Inicializando sistema gráfico...\n");
    init_graphics_system();
    printf("Sistema gráfico inicializado!\n");
    
    // Inicializar sistema de números aleatórios
    init_random();
    
    // Inicializar jogo
    printf("Inicializando jogo...\n");
    init_game();
    printf("Jogo inicializado!\n");
    
    // Aguardar dispositivos USB
    printf("Aguardando dispositivos USB...\n");
    bool keyboard_found = false;
    for (int i = 0; i < 50; i++) { // Aguardar até 5 segundos
        delay_ms(100);
        if (USPiKeyboardAvailable()) {
            keyboard_found = true;
            break;
        }
        if (i % 10 == 0) {
            printf("Aguardando teclado... (%d/5s)\n", i/10 + 1);
        }
    }
    
    if (!keyboard_found) {
        printf("AVISO: Teclado USB não encontrado!\n");
        printf("O jogo continuará, mas você precisará conectar um teclado.\n");
    } else {
        printf("Teclado USB detectado!\n");
    }
    
    // Registrar handler de teclado com assinatura correta
    if (keyboard_found) {
        USPiKeyboardRegisterKeyStatusHandlerRaw(keyboard_handler);
        printf("Handler de teclado registrado!\n");
    }
    
    printf("\n=== SNAKE GAME ===\n");
    printf("Controles:\n");
    printf("  Movimento: WASD ou Setas\n");
    printf("  Pausar: SPACE ou P\n");
    printf("  Reiniciar: R ou F\n");
    printf("  Sair: ESC ou Q\n");
    printf("================\n\n");
    
    // Desenhar tela inicial
    draw_game();
    
    uint32_t frame_count = 0;
    uint32_t last_debug_print = 0;
    
    // Loop principal do jogo
    while (true) {
        uint32_t current_time = get_ticks();
        
        // Processar entrada USB
        if (USPiKeyboardAvailable()) {
            // Não precisamos chamar USPiKeyboardUpdate() pois o handler raw é chamado automaticamente
            // Apenas manter o sistema USB ativo
        }
        
        // Atualizar lógica do jogo
        update_game();
        
        // Renderizar
        draw_game();
        
        // Debug info a cada 5 segundos (opcional)
        if (current_time - last_debug_print > 5000) {
            debug_print_game_state();
            last_debug_print = current_time;
        }
        
        // Controle de FPS (~60 FPS)
        delay_ms(16);
        frame_count++;
        
        // Verificar se deve sair (implementar lógica de saída se necessário)
        // Por enquanto, loop infinito - o jogo roda continuamente
        ProcessKernelTimers();
    }
    
    // Cleanup (nunca alcançado neste exemplo)
    printf("Encerrando Snake Game...\n");
    return 0;
}