SCANNER = scanner
CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -g3 -fsanitize=address -fno-omit-frame-pointer
LEX     = flex
ifeq ($(shell uname),Darwin)
LIBS    = -ll
else
LIBS    = -lfl
endif

all: $(SCANNER)

$(SCANNER): $(SCANNER).c
	$(CC) $(CFLAGS) $< -o $(SCANNER) $(LIBS)

$(SCANNER).c: %.c: %.l
	$(LEX) -o $@ $<

.PHONY: clean

clean:
	$(RM) $(SCANNER) $(SCANNER:=.c)
