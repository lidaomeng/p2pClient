.SUFFIXES:
.SUFFIXES: .o .cpp

CC = g++
FLAGS = -g -Wall -fPIC
CFLAGS = -g -Wall -DBOOST_ASIO_SEPARATE_COMPILATION -Wvla -Wsizeof-pointer-memaccess
AR = ar
LIB = -ltorrent-rasterbar -lboost_system-mt -lpthread
LIBPATH = -L/usr/local/libtorrent/lib/

SOURCES = main.cpp config.cpp Torrent.cpp  soapC.cpp soapClient.cpp soapServer.cpp stdsoap2.cpp ftpdownload.cpp LogManage.cpp

OBJS = $(SOURCES:.cpp=.o)

p2pclient:$(OBJS)
	$(CC) -o $@ $^ -g  $(LIBPATH) $(LIB)
	#cp $@ ./../build/
.cpp.o:
	$(CC) $(CFLAGS) -c  $< -o $@ -I/usr/local/libtorrent/include/

clean:
	-rm p2pclient *.o -rf
.PHONY: clean p2pclient
