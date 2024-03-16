COMPILERFLAGS = -g -Wall -Wextra -Wno-sign-compare 
LINKLIBS = -lpthread
.PHONY: all clean


all: distancevector linkstate

distancevector: obj/distancevector.o obj/router.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

linkstate: obj/linkstate.o obj/router.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

router: obj/router.o
	$(CC) $(COMPILERFLAGS) $^ -o $@ $(LINKLIBS)

clean :
	$(RM) obj/*.o distancevector linkstate router


obj/%.o: src/%.c
	$(CC) $(COMPILERFLAGS) -c -o $@ $<
obj:
	mkdir -p obj