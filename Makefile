# Makefile riscv

# Compilador
CC = gcc

# Compiler flags
# -Wall: ativa os alertas comuns
# -Wextra: ativa os alertas extras
# -std=c99: compila usando o padrao c99
# -o: especifica o nome do arquivo
CFLAGS = -Wall -Wextra -std=c99

# nome do executavel
TARGET = assembler

# fonte
SRCS = main.c

# Phony targets: .PHONY specifies targets that are not actual files
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f *.o