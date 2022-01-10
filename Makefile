ifeq (${shell uname},Darwin)
    CC=g++ -g -Wall -std=c++17 -D_XOPEN_SOURCE
    LIBCPU=libcpu_macos.o
else
    CC=g++ -g -Wall -std=c++17
    LIBCPU=libcpu.o
endif

# List of source files for your thread library
THREAD_SOURCES=cpu.cpp thread.cpp mutex.cpp cv.cpp

# Generate the names of the thread library's object files
THREAD_OBJS=${THREAD_SOURCES:.cpp=.o}

all: libthread.o app8

# Compile the thread library and tag this compilation
libthread.o: ${THREAD_OBJS}
	./autotag.sh
	ld -r -o $@ ${THREAD_OBJS}

# Compile an application program
app12: test12.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app2: test2.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app3: test3.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app4: test4.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app5: test5.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app6: test6.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app7: test7.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Compile an application program
app8: test8.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

apps: test${n}.cpp libthread.o ${LIBCPU}
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${THREAD_OBJS} libthread.o app*

commit:
	git add .
	git commit -m $(m)
	git push

xuxf:
	git config user.name "Xiaofeng Xu"
	git config user.email "xuxf@umich.edu"
	git add .

xiyu:
	git config user.name "Xiyu Tian"
	git config user.email "t1062464285@gmail.com"
	git add .
	git commit -m $(m)
	git push

zhengji:
	git config user.name "Jiayun Zheng"
	git config user.email "zhengji@umich.edu"
	git add .
	git commit -m $(m)
	git push