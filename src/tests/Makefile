
PNAME = tests.out

CC = g++-11

CFLAGS = -std=c++2b -Wall -O3

PATHINC = -I./../../../aftermath/src -I./../../../../include

PATHLIB =

LDFLAGS = 

.VPATH: .

$(PNAME): $(patsubst %.cpp, %.o, $(wildcard *.cpp))
	$(CC) $^ -o $@ $(PATHLIB) $(LDFLAGS) $(LDLIBS)
	rm -rf *.o *.d

%.o: %.cpp
	$(CC) $< $(CFLAGS) -c -MD $(PATHINC)

include $(wildcard *.d)

.PHONY: clean

clean:
	rm -rf *.o *.d

