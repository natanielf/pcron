CC = gcc
CFLAGS = -Wall -Wextra
RM = rm -f

TARGETS = pcron

all: $(TARGETS)
.PHONY : all

pcron : pcron.c
	$(CC) $(CFLAGS) pcron.c -o pcron

clean :
	$(RM) $(TARGETS)
.PHONY : clean
