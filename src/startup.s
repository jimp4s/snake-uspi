/*
 * startup.s - Boot code for Raspberry Pi
 * Configuração inicial do sistema antes de chamar main()
 */

.section .text.boot

.global _start

_start:
    /* Desabilitar IRQ e FIQ */
    cpsid if
    
    /* Verificar se somos o core 0 */
    mrc p15, 0, r0, c0, c0, 5
    and r0, r0, #3
    cmp r0, #0
    bne halt
    
    /* Configurar stack pointer */
    ldr r0, =_start
    mov sp, r0
    
    /* Limpar BSS section */
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
    
clear_bss:
    cmp r0, r1
    bge bss_cleared
    str r2, [r0], #4
    b clear_bss
    
bss_cleared:
    /* Habilitar cache L1 */
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #(1 << 2)  /* Data cache */
    orr r0, r0, #(1 << 12) /* Instruction cache */
    mcr p15, 0, r0, c1, c0, 0
    
    /* Configurar vector table */
    ldr r0, =vector_table
    mcr p15, 0, r0, c12, c0, 0
    
    /* Chamar main() */
    bl main
    
halt:
    wfi
    b halt

/* Vector table básica */
.align 5
vector_table:
    ldr pc, reset_handler_addr
    ldr pc, undefined_handler_addr
    ldr pc, swi_handler_addr
    ldr pc, prefetch_handler_addr
    ldr pc, data_handler_addr
    ldr pc, unused_handler_addr
    ldr pc, irq_handler_addr
    ldr pc, fiq_handler_addr

reset_handler_addr:     .word _start
undefined_handler_addr: .word undefined_handler
swi_handler_addr:       .word swi_handler
prefetch_handler_addr:  .word prefetch_handler
data_handler_addr:      .word data_handler
unused_handler_addr:    .word unused_handler
irq_handler_addr:       .word irq_handler
fiq_handler_addr:       .word fiq_handler

/* Exception handlers básicos */
undefined_handler:
    b undefined_handler

swi_handler:
    b swi_handler

prefetch_handler:
    b prefetch_handler

data_handler:
    b data_handler

unused_handler:
    b unused_handler

irq_handler:
    /* Salvar contexto */
    push {r0-r12, lr}
    
    /* Chamar handler C se necessário */
    bl timer_handler
    
    /* Restaurar contexto */
    pop {r0-r12, lr}
    subs pc, lr, #4

fiq_handler:
    b fiq_handler