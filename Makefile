CC=g++
CCFLAGS=-Wall
#CCFLAGS+=-g

.PHONY:all sync clean

all:testskiplist Ayatomic testlist testhashtable testarray testsynclist testos
	
testos:./bin/os_test

./bin/os_test:./test/os_test.cc
	${CC} ${CCFLAGS} ./test/os_test.cc -o ./bin/os_test

testskiplist:./bin/skiplist_test
	
./bin/skiplist_test:./test/skiplist_test.cc ./open/Aytable.h ./open/Aytype.h Aytable Ayskiplist
	${CC} ${CCFLAGS} ./test/skiplist_test.cc -I./open ./bin/Ayskiplist.o ./bin/Aytable.o -o ./bin/skiplist_test

Aytable:./bin/Aytable.o
	
./bin/Aytable.o:./src/Aytable.cc ./include/Aytable.h
	${CC} ${CCFLAGS} -c ./src/Aytable.cc -I./include -o ./bin/Aytable.o

Ayskiplist:./bin/Ayskiplist.o
	
./bin/Ayskiplist.o:./src/Ayskiplist.cc ./include/Aytable.h ./include/os.h
	${CC} ${CCFLAGS} -c ./src/Ayskiplist.cc -I./include -o ./bin/Ayskiplist.o

Ayatomic:./bin/Ayatomic.o

./bin/Ayatomic.o:./src/Ayatomic.cc ./open/Ayatomic.h
	${CC} ${CCFLAGS} -c ./src/Ayatomic.cc -I./open -o ./bin/Ayatomic.o

testlist:./bin/list_test

./bin/list_test:./test/list_test.cc ./open/Aylist.h ./open/Aytype.h ./include/os.h
	${CC} ${CCFLAGS} ./test/list_test.cc -I./open -I./include -o ./bin/list_test

testhashtable:./bin/hashtable_test
	
./bin/hashtable_test:./test/hashtable_test.cc ./open/Aytable.h ./open/Aytype.h Aytable Ayhashtable
	${CC} ${CCFLAGS} ./test/hashtable_test.cc -I./open ./bin/Aytable.o ./bin/Ayhashtable.o -o ./bin/hashtable_test

Ayhashtable:./bin/Ayhashtable.o

./bin/Ayhashtable.o:./src/Ayhashtable.cc ./include/Aytable.h ./include/os.h
	${CC} ${CCFLAGS} -c ./src/Ayhashtable.cc -I./include -o ./bin/Ayhashtable.o

testarray:./bin/array_test
	
./bin/array_test:./test/array_test.cc ./open/Aytable.h ./open/Aytype.h Aytable Ayarray
	${CC} ${CCFLAGS} ./test/array_test.cc -I./open ./bin/Aytable.o ./bin/Ayarray.o -o ./bin/array_test

Ayarray:./bin/Ayarray.o

./bin/Ayarray.o:./src/Ayarray.cc ./include/Aytable.h ./include/os.h
	${CC} ${CCFLAGS} -c ./src/Ayarray.cc -I./include -o ./bin/Ayarray.o

testsynclist:./bin/synclist_test

./bin/synclist_test:./test/synclist_test.cc ./open/Aysynclist.h Aysynclist
	${CC} ${CCFLAGS} ./test/synclist_test.cc -I./open ./bin/Aysynclist.o ./bin/Ayatomic.o -o ./bin/synclist_test

Aysynclist:./bin/Aysynclist.o

./bin/Aysynclist.o:./src/Aysynclist.cc ./open/Aysynclist.h ./include/os.h ./open/Aytype.h ./open/Ayatomic.h Ayatomic
	${CC} ${CCFLAGS} -c ./src/Aysynclist.cc -I./open -I./include -o ./bin/Aysynclist.o

sync:clean
	rm -rf /mnt/windows/libay
	cp -r ../libay /mnt/windows

clean:
	rm -f ./bin/*
	find . -type f | sed -n '/~/p' | xargs -i rm -f {}
