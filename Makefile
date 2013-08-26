CFLAGS=-Werror -g
JAVAC=javac

all: locktest LockTest

locktest: locktest.o
	$(CC) $< $(LDFLAGS) -o $@ 

locktest.o: locktest.c
	$(CC) $(CFLAGS) -c $< 

LockTest: LockTest.class

LockTest.class: LockTest.java
	$(JAVAC) LockTest.java

clean:
	rm -f locktest locktest.o LockTest*.class
