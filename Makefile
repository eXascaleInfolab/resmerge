#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -Iautogen -Iinclude
CFLAGS = -Wnon-virtual-dtor -Winit-self -Wcast-align -Wundef -Wfloat-equal -Wunreachable-code -Wmissing-include-dirs -Weffc++ -Wzero-as-null-pointer-constant -std=c++14 -fexceptions
RESINC = 
LIBDIR = 
LIB = -lstdc++fs
LDFLAGS = 

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -Wredundant-decls -Winline -Wswitch-default -Wmain -Wall -g -DDEBUG -DHEAVY_VALIDATION=2 -DTRACE=2
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/resmerge

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -march=core2 -fomit-frame-pointer -O3 -DHEAVY_VALIDATION=1 -DTRACE=1
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/resmerge

OBJ_DEBUG = $(OBJDIR_DEBUG)/autogen/cmdline.o $(OBJDIR_DEBUG)/src/fileio.o $(OBJDIR_DEBUG)/src/main.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/autogen/cmdline.o $(OBJDIR_RELEASE)/src/fileio.o $(OBJDIR_RELEASE)/src/main.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG)/autogen || mkdir -p $(OBJDIR_DEBUG)/autogen
	test -d $(OBJDIR_DEBUG)/src || mkdir -p $(OBJDIR_DEBUG)/src

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/autogen/cmdline.o: autogen/cmdline.c
	$(CC) $(CFLAGS_DEBUG) $(INC_DEBUG) -c autogen/cmdline.c -o $(OBJDIR_DEBUG)/autogen/cmdline.o

$(OBJDIR_DEBUG)/src/fileio.o: src/fileio.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/fileio.cpp -o $(OBJDIR_DEBUG)/src/fileio.o

$(OBJDIR_DEBUG)/src/main.o: src/main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c src/main.cpp -o $(OBJDIR_DEBUG)/src/main.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)/autogen
	rm -rf $(OBJDIR_DEBUG)/src

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/autogen || mkdir -p $(OBJDIR_RELEASE)/autogen
	test -d $(OBJDIR_RELEASE)/src || mkdir -p $(OBJDIR_RELEASE)/src

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/autogen/cmdline.o: autogen/cmdline.c
	$(CC) $(CFLAGS_RELEASE) $(INC_RELEASE) -c autogen/cmdline.c -o $(OBJDIR_RELEASE)/autogen/cmdline.o

$(OBJDIR_RELEASE)/src/fileio.o: src/fileio.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/fileio.cpp -o $(OBJDIR_RELEASE)/src/fileio.o

$(OBJDIR_RELEASE)/src/main.o: src/main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c src/main.cpp -o $(OBJDIR_RELEASE)/src/main.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/autogen
	rm -rf $(OBJDIR_RELEASE)/src

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

