CC := sudo gcc
CFLAGS := -Wall -Wextra -g
LIBS := 'pkg-config --cflags --libs gstreamer-1.0'

# Define source files and target executables
SENDER_SRC := sender.c
RECEIVER_SRC := receiver.c
SENDER := sender
RECEIVER := receiver

all: $(SENDER) $(RECEIVER)

$(SENDER): $(SENDER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(RECEIVER): $(RECEIVER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(SENDER) $(RECEIVER)
 
 