CFLAGS +=
LDFLAGS += -lm -lrt 

TARGET = drift


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(TARGET:=.o)
	$(CC) $(LDFLAGS) -o $@ $<


clean:
	rm $(TARGET) $(TARGET:=.o)