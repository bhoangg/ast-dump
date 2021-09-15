.PHONY: all, print, clean

TARGET=dump

INC+= ast-dump.h
CSRCS+= ast-dump.cpp main.cpp

OBJSDIR=build
OBJS+= $(patsubst %.cpp, $(OBJSDIR)/%.o, $(CSRCS))

CFLAGS += -I./include -DDEBUG -Wall -g
LDFLAGS += -L./lib -lm -lclang

Cxx:= clang++

all: mkdir ${TARGET}
	@echo Build complete!

${TARGET} : $(OBJS)
	@echo " [LINK] $@"
	$(Cxx) $(OBJS) -o $@ $(LDFLAGS)

$(OBJSDIR)/%.o: %.cpp $(INC)
	@echo " [CC]  $@"
	$(Cxx) -c $< -o $@ $(CFLAGS)

mkdir:
	@mkdir -p $(OBJSDIR)

print:
	@echo $(OBJS)

clean:
	rm -rf ${OBJSDIR}
	rm -rf ${TARGET}

