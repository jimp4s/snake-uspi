# Configurações do compilador
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy

# Flags de compilação
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding
CFLAGS += -I./uspi/include -Iinclude
CFLAGS += -mcpu=cortex-a53 -DRASPPI=3

# Flags de linking
LDFLAGS = -L./uspi/lib -luspi

# Diretórios
SRCDIR = src
BUILDDIR = build
USPIDIR = uspi
INCLUDEDIR = include

# Arquivos fonte
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/graphics.c $(SRCDIR)/syscalls.c
ASM_SOURCES = $(SRCDIR)/startup.s
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o) $(ASM_SOURCES:$(SRCDIR)/%.s=$(BUILDDIR)/%.o)

# Biblioteca USPI
USPI_LIB = $(USPIDIR)/lib/libuspi.a

# Targets
TARGET = kernel.elf
IMAGE = kernel.img

.PHONY: all clean uspi

all: $(IMAGE)

# Construir a imagem final
$(IMAGE): $(TARGET)
	$(OBJCOPY) $(TARGET) -O binary $(IMAGE)

# Linkar o executável
$(TARGET): $(OBJECTS) $(USPI_LIB)
	$(CC) $(CFLAGS) -T kernel.ld -o $@ $(OBJECTS) $(LDFLAGS)

# Compilar objetos C
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar objetos Assembly
$(BUILDDIR)/%.o: $(SRCDIR)/%.s | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Criar diretório de build
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Construir biblioteca USPI
$(USPI_LIB): uspi

uspi:
	$(MAKE) -C $(USPIDIR)/lib

# Limpeza
clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET) $(IMAGE)
	$(MAKE) -C $(USPIDIR)/lib clean

# Dependências
$(BUILDDIR)/main.o: $(SRCDIR)/main.c $(INCLUDEDIR)/config.h $(INCLUDEDIR)/graphics.h
$(BUILDDIR)/graphics.o: $(SRCDIR)/graphics.c $(INCLUDEDIR)/config.h $(INCLUDEDIR)/graphics.h
$(BUILDDIR)/startup.o: $(SRCDIR)/startup.s