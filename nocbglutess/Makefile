CXXFLAGS+=-fPIC

html/index.html: doxyfile test.out nocbglutess.h main.c
	doxygen $<
	see $@

test.out: nocbglutess
	type valgrind 2>/dev/null && { LD_LIBRARY_PATH=. valgrind --leak-check=full ./$< >$@.tmp; } || { LD_LIBRARY_PATH=. ./$< >$@.tmp;}
	mv test.out.tmp $@

nocbglutess: main.o libnocbglutess.so
	$(CC) -o $@ $< -L. -lnocbglutess $(LDFLAGS)

libnocbglutess.so: nocbglutess.o
	$(CXX) -o $@ -shared $< -lGLU $(LDFLAGS)

nocbglutess.o: nocbglutess.h nocbglutess.cpp

main.o: nocbglutess.h main.c

