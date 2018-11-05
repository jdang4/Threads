LIB=-lpthread
CCPP=g++

all: addem life

addem: addem.cpp Mailbox.cpp
	$(CCPP) addem.cpp Mailbox.cpp -o addem $(LIB)

life: life.cpp Mailbox.cpp
	$(CCPP) life.cpp Mailbox.cpp -o life $(LIB)

clean:
	rm -f addem.o Mailbox.o life.o addem life