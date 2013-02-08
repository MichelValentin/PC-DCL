NAME = dcl
TARGET=$(OUTDIR)/$(NAME).exe
UNIX_TARGET=$(OUTDIR)/$(NAME)

##USE_READLINE = 1
USE_CURSES = 1
CC=gcc
CXX=g++
LD=gcc
RC=windres -O COFF

#CFLAGS=-O0 -g3 -Wall -fmessage-length=0 -DUSE_CURSES=1 -D_XOPEN_SOURCE_EXTENDED
CFLAGS=-O0 -g3 -Wall -DUSE_CURSES=1 -D_XOPEN_SOURCE_EXTENDED -DSTDC_HEADERS
#CFLAGS=-O0 -g3 -Wall -DUSE_CURSES=1  -D__EXTENSIONS__
CXXFLAGS=$(CFLAGS) 
LDFLAGS=-g -lm
OUTDIR=Debug

ifeq "$(NDEBUG)" "1"
CFLAGS=-Wall -fmessage-length=0 -DUSE_CURSES=1 -D_XOPEN_SOURCE_EXTENDED -DSTDC_HEADERS
LDFLAGS=-lm 
OUTDIR=Release
endif

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS)  -o $@ -c $<

$(OUTDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS)  -o $@ -c $<

$(OUTDIR)/%.res: %.rc
	$(RC) -o $@ -i $<

#SRCS=$(wildcard *.c *.cpp *.rc) dcl.ico
SRCS=$(wildcard *.c *.cpp) 

OBJS=$(addprefix $(OUTDIR)/,$(patsubst %.rc,%.res,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(filter %.c %.cpp %.rc,$(SRCS))))))

LIBS=

UNIXLIBS=

ifeq "$(USE_CURSES)" "1"
UNIXLIBS= -lcurses
endif

ifeq "$(USE_READLINE)" "1"
UNIXLIBS= -l readline -l termcap
endif

$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

$(UNIX_TARGET): $(OBJS)
	@echo 'Building target: $@'
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(UNIXLIBS)
	@echo 'Finished building target: $@'
	cp $(OUTDIR)/dcl ../
	@echo ' '

.PHONY: all prepare clean depends rebuild

all: prepare $(TARGET)

unix: prepare $(UNIX_TARGET)

rebuild: clean depends all

prepare: 
	@mkdir -p $(OUTDIR)
	@echo ' '
	
clean:
	@echo 'Deleting output files.'
	-rm -f $(OBJS) $(TARGET) $(UNIX_TARGET)
	@echo ' '

depends:
	@echo 'Building dependencies.'
	-$(CC) $(CFLAGS) -MM $(filter %.c,$(SRCS)) > $(NAME).tmp
	-sed -e's|\(.*\.o:\)|$(OUTDIR)\/\1|g' $(NAME).tmp > $(NAME).dep
	-rm $(NAME).tmp

-include $(NAME).dep
