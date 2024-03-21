COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare 
LINKLIBS = -lpthread
.PHONY: all clean


all: dvr lsr

dvr: obj/distancevector.o obj/router.o obj/minHeap.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

lsr: obj/linkstate.o obj/router.o obj/minHeap.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

router: obj/router.o obj/minHeap.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

minHeap: obj/minHeap.obj.o 
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

clean :
	$(RM) obj/*.o distancevector linkstate router minHeap


obj/%.o: src/%.c
	$(CC) $(COMPILERFLAGS) -c -o $@ $<
obj:
	mkdir -p obj