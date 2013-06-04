CFLAGS +=
LDFLAGS += -lrt 

TARGET = granularity


%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(TARGET:=.o)
	$(CC) $(LDFLAGS) -o $@ $<


clean:
	rm $(TARGET) $(TARGET:=.o)
