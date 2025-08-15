//
// syscalls.c - Implementações de funções de sistema necessárias para USPI
// Adicione este arquivo ao seu projeto e compile junto com os outros arquivos
//

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// ================================
// MEMORY MANAGEMENT
// ================================

// Heap simples para malloc/free
#define HEAP_SIZE (1024 * 1024)  // 1MB de heap
static uint8_t heap[HEAP_SIZE];
static uint32_t heap_ptr = 0;

void* malloc(size_t size) {
    // Alinha o tamanho para 8 bytes
    size = (size + 7) & ~7;
    
    if (heap_ptr + size > HEAP_SIZE) {
        return NULL;  // Sem memória suficiente
    }
    
    void* ptr = &heap[heap_ptr];
    heap_ptr += size;
    return ptr;
}

void free(void* ptr) {
    // Implementação simples - não libera realmente a memória
    // Para um sistema mais complexo, você implementaria um gerenciador real
    (void)ptr;  // Evita warning de parâmetro não usado
}

void* calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void* ptr = malloc(total_size);
    if (ptr) {
        // Zera a memória alocada
        uint8_t* byte_ptr = (uint8_t*)ptr;
        for (size_t i = 0; i < total_size; i++) {
            byte_ptr[i] = 0;
        }
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    // Implementação simples - aloca novo bloco e copia
    if (!ptr) {
        return malloc(size);
    }
    
    void* new_ptr = malloc(size);
    if (new_ptr && ptr) {
        // Copia os dados (assumindo que o tamanho antigo era menor)
        uint8_t* src = (uint8_t*)ptr;
        uint8_t* dst = (uint8_t*)new_ptr;
        for (size_t i = 0; i < size; i++) {
            dst[i] = src[i];
        }
    }
    return new_ptr;
}

// ================================
// TIMER FUNCTIONS
// ================================

// Função para acessar o timer do sistema
static uint64_t get_system_timer(void) {
    volatile uint32_t* timer_clo = (uint32_t*)0x3F003004;
    volatile uint32_t* timer_chi = (uint32_t*)0x3F003008;
    
    uint32_t hi1 = *timer_chi;
    uint32_t lo = *timer_clo;
    uint32_t hi2 = *timer_chi;
    
    if (hi1 != hi2) {
        lo = *timer_clo;
    }
    
    return ((uint64_t)hi2 << 32) | lo;
}

void MsDelay(unsigned nMilliSeconds) {
    uint64_t start = get_system_timer();
    uint64_t delay_us = (uint64_t)nMilliSeconds * 1000;
    
    while ((get_system_timer() - start) < delay_us) {
        // Busy wait
        __asm__ volatile("nop");
    }
}

void usDelay(unsigned nMicroSeconds) {
    uint64_t start = get_system_timer();
    
    while ((get_system_timer() - start) < nMicroSeconds) {
        __asm__ volatile("nop");
    }
}

// ================================
// RANDOM NUMBER GENERATION
// ================================

static unsigned int seed = 1;

void srand(unsigned int s) {
    seed = s;
}

int rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// ================================
// STRING AND I/O FUNCTIONS
// ================================

// Função básica para escrever caractere (você pode implementar via UART/GPU)
static void putchar_basic(char c) {
    // Para Raspberry Pi 3, UART está em 0x3F201000
    volatile uint32_t* uart_dr = (uint32_t*)0x3F201000;
    volatile uint32_t* uart_fr = (uint32_t*)0x3F201018;
    
    // Espera até que o transmissor esteja livre
    while (*uart_fr & (1 << 5)) {
        __asm__ volatile("nop");
    }
    
    *uart_dr = c;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    const char* p = format;
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        putchar_basic('-');
                        val = -val;
                        count++;
                    }
                    
                    char buffer[12];
                    int i = 0;
                    do {
                        buffer[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val);
                    
                    for (int j = i - 1; j >= 0; j--) {
                        putchar_basic(buffer[j]);
                        count++;
                    }
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char hex[] = "0123456789abcdef";
                    char buffer[9];
                    int i = 0;
                    
                    do {
                        buffer[i++] = hex[val % 16];
                        val /= 16;
                    } while (val);
                    
                    for (int j = i - 1; j >= 0; j--) {
                        putchar_basic(buffer[j]);
                        count++;
                    }
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    while (*str) {
                        putchar_basic(*str++);
                        count++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar_basic(c);
                    count++;
                    break;
                }
                case '%':
                    putchar_basic('%');
                    count++;
                    break;
                default:
                    putchar_basic('%');
                    putchar_basic(*p);
                    count += 2;
                    break;
            }
        } else {
            putchar_basic(*p);
            count++;
        }
        p++;
    }
    
    va_end(args);
    return count;
}

int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    const char* p = format;
    char* out = str;
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        *out++ = '-';
                        val = -val;
                        count++;
                    }
                    
                    char buffer[12];
                    int i = 0;
                    do {
                        buffer[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val);
                    
                    for (int j = i - 1; j >= 0; j--) {
                        *out++ = buffer[j];
                        count++;
                    }
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    while (*s) {
                        *out++ = *s++;
                        count++;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *out++ = c;
                    count++;
                    break;
                }
                default:
                    *out++ = '%';
                    *out++ = *p;
                    count += 2;
                    break;
            }
        } else {
            *out++ = *p;
            count++;
        }
        p++;
    }
    
    *out = '\0';
    va_end(args);
    return count;
}

// ================================
// USPI SPECIFIC FUNCTIONS
// ================================

void LogWrite(const char* pSource, unsigned Severity, const char* pMessage, ...) {
    // Implementação básica de logging
    printf("[%s] ", pSource);
    
    va_list args;
    va_start(args, pMessage);
    
    const char* p = pMessage;
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 'd': {
                    int val = va_arg(args, int);
                    printf("%d", val);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    printf("%s", str);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    printf("%x", val);
                    break;
                }
                default:
                    putchar_basic('%');
                    putchar_basic(*p);
                    break;
            }
        } else {
            putchar_basic(*p);
        }
        p++;
    }
    
    va_end(args);
    putchar_basic('\n');
}

void uspi_assertion_failed(const char* pExpr, const char* pFile, unsigned nLine) {
    printf("ASSERTION FAILED: %s at %s:%d\n", pExpr, pFile, nLine);
    
    // Para em caso de assertion failure
    while (1) {
        __asm__ volatile("wfi");  // Wait for interrupt
    }
}

// ================================
// PROPERTY TAG INTERFACE (Raspberry Pi specific)
// ================================

int GetMACAddress(unsigned char Buffer[6]) {
    // Implementação stub - você pode implementar via mailbox se precisar
    // Por enquanto, retorna um MAC address fictício
    Buffer[0] = 0xB8;
    Buffer[1] = 0x27;
    Buffer[2] = 0xEB;
    Buffer[3] = 0x12;
    Buffer[4] = 0x34;
    Buffer[5] = 0x56;
    return 1;  // Sucesso
}

// ================================
// INTERRUPT HANDLING
// ================================

void ConnectInterrupt(unsigned nIRQ, void (*pHandler)(void)) {
    // Implementação stub - você precisa implementar se usar interrupções
    (void)nIRQ;
    (void)pHandler;
}

void DisconnectInterrupt(unsigned nIRQ) {
    (void)nIRQ;
}

// ================================
// ARM AEABI FUNCTIONS (required for division)
// ================================

unsigned int __aeabi_uidiv(unsigned int numerator, unsigned int denominator) {
    if (denominator == 0) return 0;
    
    unsigned int quotient = 0;
    unsigned int remainder = numerator;
    
    while (remainder >= denominator) {
        unsigned int temp = denominator;
        unsigned int multiple = 1;
        
        while ((temp << 1) <= remainder) {
            temp <<= 1;
            multiple <<= 1;
        }
        
        remainder -= temp;
        quotient += multiple;
    }
    
    return quotient;
}

unsigned int __aeabi_uidivmod(unsigned int numerator, unsigned int denominator) {
    if (denominator == 0) return 0;
    
    unsigned int quotient = __aeabi_uidiv(numerator, denominator);
    return numerator - (quotient * denominator);  // Return remainder
}

int __aeabi_idiv(int numerator, int denominator) {
    if (denominator == 0) return 0;
    
    int sign = 1;
    if (numerator < 0) {
        numerator = -numerator;
        sign = -sign;
    }
    if (denominator < 0) {
        denominator = -denominator;
        sign = -sign;
    }
    
    return sign * (int)__aeabi_uidiv((unsigned int)numerator, (unsigned int)denominator);
}

// ================================
// POWER MANAGEMENT
// ================================

int SetPowerStateOn(unsigned nDeviceId) {
    // Implementação básica para power management
    // Para um sistema real, você usaria o mailbox interface
    (void)nDeviceId;
    return 1;  // Assume sucesso
}

// ================================
// KERNEL TIMER FUNCTIONS
// ================================

typedef struct {
    unsigned int active;
    unsigned int timeout;
    void (*handler)(unsigned int, void*);
    void* param;
    unsigned int hTimer;
} KernelTimer;

#define MAX_TIMERS 16
static KernelTimer timers[MAX_TIMERS];
static unsigned int next_timer_id = 1;

unsigned int StartKernelTimer(unsigned nHundredthsOfSecond, 
                              void (*pHandler)(unsigned int hTimer, void *pParam),
                              void *pParam, void *pContext) {
    (void)pContext;
    
    // Encontra um slot livre
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timers[i].active) {
            timers[i].active = 1;
            timers[i].timeout = get_system_timer() + (nHundredthsOfSecond * 10000); // Convert to microseconds
            timers[i].handler = pHandler;
            timers[i].param = pParam;
            timers[i].hTimer = next_timer_id++;
            return timers[i].hTimer;
        }
    }
    
    return 0;  // No free timers
}

void CancelKernelTimer(unsigned int hTimer) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].active && timers[i].hTimer == hTimer) {
            timers[i].active = 0;
            break;
        }
    }
}

// Função para processar timers (deve ser chamada periodicamente)
void ProcessKernelTimers(void) {
    uint64_t current_time = get_system_timer();
    
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].active && current_time >= timers[i].timeout) {
            timers[i].active = 0;  // Timer fires only once
            if (timers[i].handler) {
                timers[i].handler(timers[i].hTimer, timers[i].param);
            }
        }
    }
}

// ================================
// DEBUG FUNCTIONS
// ================================

void DebugHexdump(const void *pBuffer, unsigned nBufLen, const char *pSource) {
    const unsigned char *buf = (const unsigned char *)pBuffer;
    
    printf("[%s] Hexdump (%u bytes):\n", pSource ? pSource : "DEBUG", nBufLen);
    
    for (unsigned i = 0; i < nBufLen; i += 16) {
        printf("%04x: ", i);
        
        // Print hex values
        for (unsigned j = 0; j < 16 && (i + j) < nBufLen; j++) {
            printf("%02x ", buf[i + j]);
        }
        
        // Padding for short lines
        for (unsigned j = nBufLen - i; j < 16; j++) {
            printf("   ");
        }
        
        printf(" |");
        
        // Print ASCII values
        for (unsigned j = 0; j < 16 && (i + j) < nBufLen; j++) {
            unsigned char c = buf[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
}

// ================================
// ADDITIONAL HELPER FUNCTIONS
// ================================

void* memset(void* s, int c, size_t n) {
    unsigned char* ptr = (unsigned char*)s;
    while (n--) {
        *ptr++ = (unsigned char)c;
    }
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

// ================================
// SYSTEM INITIALIZATION
// ================================

void init_system(void) {
    // Initialize timer array
    for (int i = 0; i < MAX_TIMERS; i++) {
        timers[i].active = 0;
    }
    
    // Initialize UART for debug output
    volatile uint32_t* uart_cr = (uint32_t*)0x3F201030;
    volatile uint32_t* uart_ibrd = (uint32_t*)0x3F201024;
    volatile uint32_t* uart_fbrd = (uint32_t*)0x3F201028;
    volatile uint32_t* uart_lcrh = (uint32_t*)0x3F20102C;
    
    *uart_cr = 0;      // Disable UART
    *uart_ibrd = 26;   // 115200 baud
    *uart_fbrd = 3;
    *uart_lcrh = 0x70; // 8 bits, FIFO enable
    *uart_cr = 0x301;  // Enable UART, TX, RX
}