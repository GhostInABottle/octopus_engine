BIN = octopus_engine
INCOCB = ../include/
INCXD = ../vendor/xd/include/
DEFINES = -D LUABIND_DYNAMIC_LINK
INCDIRS = -I$(INCOCB) -I$(INCXD) -I/usr/include/freetype2
LDFLAGS = -lfreetype -llua5.2 -lboost_unit_test_framework -lboost_regex -lluabind -lfmod -lIL -lGL -lGLEW -lglfw3 -lpthread -lX11 -lXxf86vm -lXi -lXrandr -lz -lm -ldl
CXXFLAGS = -std=c++11 -O2 $(DEFINES) $(INCDIRS)
SOURCES = $(shell find ../src/ -name *.cpp -not -path "../src/tests/*") $(shell find ../vendor/xd/src -name *.cpp -not -path "../vendor/xd/src/test/*")
OBJS = 	$(SOURCES:%.cpp=%.o)
default: all

%.o: %.c
	g++ -c -o $@ $? ${CCFLAGS}

all: ${OBJS}
	g++ -o ${BIN} $+ ${LDFLAGS}

clean:	
	rm ${OBJS}

