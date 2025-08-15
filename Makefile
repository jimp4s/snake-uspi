#
# Makefile para Snake Game com USPI
# Raspberry Pi bare metal
#

# Configurações
RASPPI = 3
PREFIX = arm-none-eabi-

# Diretórios
USPI_DIR = ./uspi
SRCDIR = src
BUILDDIR = build

# Compilador e flags
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

# Flags do compilador
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding
CFLAGS += -I$(USPI_DIR)/include
CFLAGS += -I$(SRCDIR)

# Flags específicas para Raspberry Pi
ifeq ($(RASPPI), 1)
	CFLAGS += -mcpu=arm1176jzf-s -DRASPPI=1
	LDFLAGS = -T rpi1.ld
else ifeq ($(RASPPI), 2)
	CFLAGS += -mcpu=cortex-a7 -DRASPPI=2
	LDFLAGS = -T rpi2.ld
else ifeq ($(RASPPI), 3)
	CFLAGS += -mcpu=cortex-a53 -DRASPPI=3
	LDFLAGS = -T rpi3.ld
endif

# Arquivos
SOURCES = main.c graphics.c startup.s
OBJECTS = $(SOURCES:%.c=$(BUILDDIR)/%.o)
OBJECTS := $(OBJECTS:%.s=$(BUILDDIR)/%.o)

# Bibliotecas USPI
USPI_LIB = $(USPI_DIR)/lib/libuspi.a

# Targets
all: kernel.img

kernel.img: kernel.elf
	$(OBJCOPY) $< -O binary $@

kernel.elf: $(OBJECTS) $(USPI_LIB)
	$(LD) $(LDFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.s | $(BUILDDIR)
	$(AS) $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(USPI_LIB):
	$(MAKE) -C $(USPI_DIR)/lib

clean:
	rm -rf $(BUILDDIR)
	rm -f kernel.elf kernel.img
	$(MAKE) -C $(USPI_DIR)/lib clean

# Target para executar no QEMU (para testes)
qemu: kernel.img
	qemu-system-arm -M raspi3 -kernel kernel.img -serial stdio -display none

# Target para instalar no cartão SD
install: kernel.img
	@echo "Copy kernel.img to your SD card along with:"
	@echo "- bootcode.bin"
	@echo "- fixup.dat (or fixup_cd.dat for Pi 4)"
	@echo "- start.elf (or start_cd.elf for Pi 4)"
	@echo "These files can be found at:"
	@echo "https://github.com/raspberrypi/firmware/tree/master/boot"

.PHONY: all clean qemu install