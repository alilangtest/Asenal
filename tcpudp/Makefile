.PHONY: all clean

OBJECT= srv cli

all: $(OBJECT)

srv: srv.cc csapp.cc
	g++ srv.cc csapp.cc -lpthread -o $@

cli: cli.cc csapp.cc
	g++ cli.cc csapp.cc -lpthread -o $@

clean:
	rm -rf $(OBJECT)
