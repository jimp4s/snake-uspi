#!/bin/bash

#
# Script de build automatizado para Snake Game + USPI
# 

set -e

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Função para imprimir mensagens coloridas
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Verificar se estamos no diretório correto
if [ ! -f "Makefile" ]; then
    print_error "Makefile não encontrado. Execute este script no diretório raiz do projeto."
    exit 1
fi

print_status "Iniciando build do Snake Game + USPI..."

# Verificar toolchain
print_status "Verificando toolchain ARM..."
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    print_error "arm-none-eabi-gcc não encontrado!"
    print_error "Instale com: sudo apt-get install gcc-arm-none-eabi"
    exit 1
fi

print_status "Toolchain encontrada: $(arm-none-eabi-gcc --version | head -n1)"

# Verificar se USPI está presente
if [ ! -d "uspi" ]; then
    print_warning "Diretório USPI não encontrado."
    print_status "Clonando USPI..."
    git clone https://github.com/rsta2/uspi.git
fi

# Criar estrutura de diretórios se necessário
print_status "Criando estrutura de diretórios..."
mkdir -p src build

# Verificar se arquivos fonte existem
if [ ! -f "src/main.c" ]; then
    print_error "src/main.c não encontrado!"
    exit 1
fi

if [ ! -f "src/startup.s" ]; then
    print_error "src/startup.s não encontrado!"
    exit 1
fi

# Build USPI library
print_status "Construindo biblioteca USPI..."
if [ -f "uspi/lib/libuspi.a" ]; then
    print_warning "libuspi.a já existe, reconstruindo..."
    make -C uspi/lib clean
fi

make -C uspi/lib

if [ ! -f "uspi/lib/libuspi.a" ]; then
    print_error "Falha ao construir libuspi.a"
    exit 1
fi

print_status "USPI construída com sucesso!"

# Build projeto principal
print_status "Construindo projeto principal..."
make clean
make

if [ ! -f "kernel.img" ]; then
    print_error "Falha ao gerar kernel.img"
    exit 1
fi

print_status "Build concluída com sucesso!"
print_status "Arquivo kernel.img gerado."

# Verificar tamanho do kernel
KERNEL_SIZE=$(stat -c%s "kernel.img")
print_status "Tamanho do kernel: $KERNEL_SIZE bytes"

if [ $KERNEL_SIZE -gt 8388608 ]; then  # 8MB
    print_warning "Kernel muito grande! Considere otimizar o código."
fi

# Criar diretório de deploy se não existir
mkdir -p deploy

# Copiar arquivos necessários
print_status "Preparando arquivos para deploy..."
cp kernel.img deploy/
cp config.txt deploy/

print_status "Arquivos copiados para diretório deploy/:"
ls -la deploy/

print_status "===========================================" 
print_status "BUILD CONCLUÍDA COM SUCESSO!"
print_status "==========================================="
print_status ""
print_status "Próximos passos:"
print_status "1. Baixar arquivos de firmware do Pi:"
print_status "   - bootcode.bin"
print_status "   - fixup.dat"  
print_status "   - start.elf"
print_status ""
print_status "2. Copiar todos os arquivos para cartão SD:"
print_status "   - kernel.img (gerado)"
print_status "   - config.txt (gerado)"
print_status "   - Arquivos de firmware (baixados)"
print_status ""
print_status "3. Inserir SD no Raspberry Pi e ligar"
print_status ""
print_status "Comando para baixar firmware:"
echo "wget https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin -O deploy/bootcode.bin"
echo "wget https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat -O deploy/fixup.dat"  
echo "wget https://github.com/raspberrypi/firmware/raw/master/boot/start.elf -O deploy/start.elf"