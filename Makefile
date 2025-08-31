CC = gcc
CFLAGS = -Wall -Wextra
RM = rm -f

pcron : pcron.c
	$(CC) $(CFLAGS) pcron.c -o pcron

clean :
	$(RM) pcron
.PHONY : clean
