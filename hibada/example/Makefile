CXX = g++
CXXFLAGS = -Wall -W -DDEBUG -g -O0 -D__XDEBUG__
OBJECT = bada
SRC_DIR = ./

LIB_PATH = -L../ \
		   -L../output/bin/

LIBS = -lbada \
	   -lpthread

INCLUDE_PATH = -I../output/include/

LIBRARY = libbada.a
.PHONY: all clean


BASE_BOJS := $(wildcard $(SRC_DIR)/*.cc)
BASE_BOJS += $(wildcard $(SRC_DIR)/*.c)
BASE_BOJS += $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst %.cc,%.o,$(BASE_BOJS))

all: $(OBJECT)
	@echo "Success, go, go, go..."

$(OBJECT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)

$(OBJS): %.o : %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE_PATH) 

$(LIBRARY): $(OBJS)
	rm -rf $@
	ar -rcs $@ $(OBJS)


clean: 
	rm -f $(SRC_DIR)/*.o
	rm -rf $(OBJECT)
