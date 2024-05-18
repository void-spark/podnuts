CC           = g++
OBJS         = boarddeamon.o
TINYXML_OBJS = ../tinyxml/tinyxml.o ../tinyxml/tinyxmlerror.o ../tinyxml/tinyxmlparser.o
HEADERS      = ../tinyxml/tinyxml.h
LINK_OPTS    = -lcrypt
GEN_OPTS     = -DTIXML_USE_STL -g -pipe

all: boardDeamon.ex

boardDeamon.ex: $(TINYXML_OBJS) $(OBJS) $(HEADERS)
	$(CC) $(TINYXML_OBJS) $(OBJS) -o boardDeamon.ex $(LINK_OPTS)

%.o: %.cpp
	$(CC) $(GEN_OPTS) -c $*.cpp -o $*.o

clean:
	rm -f boardDeamon.ex $(TINYXML_OBJS) $(OBJS)

