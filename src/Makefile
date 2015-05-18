CC=g++
CPFLAGS=-g -Wall -std=c++11
LDFLAGS= -lssl -lcrypto -lpthread
INCFLAGS = -I .

SRC= bt_client.cpp bt_lib.cpp bt_setup.cpp parser.cpp sha1check.cpp Peer.cpp PeerManager.cpp Message.cpp FileManager.cpp 
OBJ=$(SRC:.cpp=.o)
BIN=bt_client

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)

%.o:%.cpp
	$(CC) -c $(CPFLAGS) $(INCFLAGS) -o $@ $<

$(SRC):

clean:
	rm -rf $(OBJ) $(BIN)
