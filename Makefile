CC = gcc
CFLAGS = -Wall -O2
FINAL = perceptron

$(FINAL): $(FINAL).o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(FINAL) *.o