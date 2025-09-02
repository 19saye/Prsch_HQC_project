CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra -O2
LDFLAGS := -lm

SRC     := src/main_rt.c src/packet.c src/channel.c src/crypto.c
BIN     := HQC_rt_secure

.PHONY: all clean run

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(BIN)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

