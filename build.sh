#!/bin/bash

# Script de build para Snake Game + USPI
# Atualizado com todas as correções

set -e

echo "[INFO] Iniciando build do Snake Game + USPI..."

# Verificar toolchain
echo "[INFO] Verificando toolchain ARM..."
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo "[ERRO] Toolchain ARM não encontrada!"
    echo "Instale: sudo apt-get install gcc-arm-none-eabi"
    exit 1
fi

TOOLCHAIN_VERSION=$(arm-none-eabi-gcc --version | head -n1)
echo "[INFO] Toolchain encontrada: $TOOLCHAIN_VERSION"

# Criar estrutura de diretórios
echo "[INFO] Criando estrutura de diretórios..."
mkdir -p src
mkdir -p include
mkdir -p build

# Verificar se os arquivos necessários existem
if [ ! -f "src/main.c" ]; then
    echo "[ERRO] src/main.c não encontrado!"
    echo "Certifique-se de que o arquivo main.c corrigido está em src/"
    exit 1
fi

if [ ! -f "src/startup.s" ]; then
    echo "[ERRO] src/startup.s não encontrado!"
    echo "Certifique-se de que o arquivo startup.s está em src/"
    exit 1
fi

# Verificar se existe um linker script
if [ ! -f "kernel.ld" ] && [ ! -f "src/kernel.ld" ] && [ ! -f "link.ld" ]; then
    echo "[AVISO] Nenhum linker script encontrado (kernel.ld, src/kernel.ld, ou link.ld)"
    echo "Você pode precisar criar um ou ajustar o Makefile"
fi

if [ ! -f "include/config.h" ]; then
    echo "[ERRO] include/config.h não encontrado!"
    echo "Certifique-se de que o arquivo config.h está em include/"
    exit 1
fi

if [ ! -f "include/graphics.h" ]; then
    echo "[ERRO] include/graphics.h não encontrado!"
    echo "Certifique-se de que o arquivo graphics.h está em include/"
    exit 1
fi

if [ ! -f "src/graphics.c" ]; then
    echo "[ERRO] src/graphics.c não encontrado!"
    echo "Certifique-se de que o arquivo graphics.c está em src/"
    exit 1
fi

# Construir biblioteca USPI
if [ ! -d "uspi" ]; then
    echo "[ERRO] Diretório USPI não encontrado!"
    echo "Clone o repositório USPI ou verifique o caminho"
    exit 1
fi

echo "[INFO] Construindo biblioteca USPI..."
make -C uspi/lib
if [ $? -eq 0 ]; then
    echo "[INFO] USPI construída com sucesso!"
else
    echo "[ERRO] Falha na construção da USPI!"
    exit 1
fi

# Construir projeto principal
echo "[INFO] Construindo projeto principal..."

# Limpar build anterior
make clean

# Compilar
make all

if [ $? -eq 0 ]; then
    echo "[SUCESSO] Build concluído com sucesso!"
    echo ""
    echo "Arquivos gerados:"
    echo "  - kernel.elf (executável)"
    echo "  - kernel.img (imagem para Raspberry Pi)"
    echo ""
    echo "Para testar no Raspberry Pi 3:"
    echo "1. Copie kernel.img para o cartão SD"
    echo "2. Certifique-se de ter config.txt adequado"
    echo "3. Conecte um teclado USB"
    echo "4. Inicialize o Pi"
    ls -la kernel.*
else
    echo "[ERRO] Falha na construção do projeto!"
    exit 1
fi