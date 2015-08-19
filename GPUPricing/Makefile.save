COMPILER = g++

JHOME = ~/projects/GPUPricing

# paths for all the source files
COMMON_src = $(JHOME)/common
PRICING_src = $(JHOME)
VPATH = $(PRICING_src) $(COMMON_src) 

# paths for all the header files
INCL_DIR  = -I /usr/include -I/usr/local/include
INCL_DIR += -I $(COMMON_src)

LIB_COMMON = libcommon
LIB_VR = libvr

# all the object files for PRICING

PRICING_OBJS = AnnualLoss.o Simulation.o virtualYear.o pricing.o

ALL_OBJS = $(COMMON_OBJS) $(PRICING_OBJS)

# compiler flags
CFLAGS = -fPIC -fexceptions -DNDEBUG -W -std=c++0x
CFLAGS += ${INCL_DIR} -O3 -s
#CFLAGS += -Wall -fopenmp -g

# link options and libraries to be linked
LIBRARY = -L /usr/local/lib -lgomp -lpthread

# targets to be compiled
all: pricing

clean:
	echo $(VPATH)
	rm -f *.o core 

pricing: $(ALL_OBJS)
	$(COMPILER) -o $@ $(PRICING_OBJS) $(LIBRARY) -static -lm

# how the object files are generated
%.o:%.cpp
	$(COMPILER) -o $@ $(CFLAGS) -c $<

%.o:%.cc
	$(COMPILER) -o $@ $(CFLAGS) -c $<


