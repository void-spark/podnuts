CC           = g++
OBJS         = dnsdeamon.o
TINYXML_OBJS = ../tinyxml/tinyxml.o ../tinyxml/tinyxmlerror.o ../tinyxml/tinyxmlparser.o
HEADERS      = ../tinyxml/tinyxml.h dnsdeamon.h errorClasses.h
LINK_OPTS    =
GEN_OPTS     = -DTIXML_USE_STL -g -pipe

all: dnsDeamon.ex

dnsDeamon.ex: $(TINYXML_OBJS) $(OBJS) $(HEADERS)
	$(CC) $(LINK_OPTS) $(TINYXML_OBJS) $(OBJS) -o dnsDeamon.ex

dnsDeamonTest.ex: $(TINYXML_OBJS) dnsDeamonTest.o $(HEADERS)
	$(CC) $(LINK_OPTS) $(TINYXML_OBJS) dnsDeamonTest.o -o dnsDeamonTest.ex

%.o: %.cpp
	$(CC) $(GEN_OPTS) -c $*.cpp -o $*.o

clean:
	rm -f dnsDeamon.ex $(TINYXML_OBJS) $(OBJS)

