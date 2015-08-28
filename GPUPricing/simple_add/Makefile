COMPILER = g++

JHOME = ~/projects/simple_add

# paths for all the header files
INCL_DIR  = -I /usr/include -I/usr/local/include

# all the object files for PRICING

PRICING_OBJS = add.o

ALL_OBJS = $(PRICING_OBJS)

# compiler flags
CFLAGS = -fPIC -fexceptions -DNDEBUG -W -std=c++0x
CFLAGS += ${INCL_DIR} -O3 -s
#CFLAGS += -Wall -fopenmp -g

# link options and libraries to be linked
#LIBRARY = -L /usr/local/lib -lgomp -lpthread -lOpenCL
LIBRARY = -L /usr/local/lib -lgomp -lpthread -L/usr/lib/x86_64-linux-gnu/libOpenCL.so

# targets to be compiled
all: add

clean:
	echo $(VPATH)
	rm -f *.o core 

pricing: $(ALL_OBJS)
	$(COMPILER) -o $@ $(PRICING_OBJS) $(LIBRARY) -lm

# how the object files are generated
%.o:%.cpp
	$(COMPILER) -o $@ $(CFLAGS) -c $<

%.o:%.cc
	$(COMPILER) -o $@ $(CFLAGS) -c $<


