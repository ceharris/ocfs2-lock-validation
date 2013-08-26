ocfs2-lock-validation
=====================

This small project provides a couple of useful utilities for 
validating lock support on an OCFS2 filesystem.  Historically, OCFS2
supported only advisory locks using the ```flock```(2) system call.

OCFS2 supports POSIX style locks (see ```lockf```(3) and ```fcntl```(2))
but only when using an userspace cluster stack_.  Apparently, the 
default O2CB cluster stack is a kernel-based cluster stack.

Building
--------
This project includes a simple utility written in C as well as a 
simple Java program (for those of you needing to test locks acquired 
using Java's ```FileChannel``` class).

The project's Makefile will build both utilities by default by simply
running
```
make
```
If you don't have Java or simply don't want to build the Java program,
you can build just the C-based utility using

```
make locktest
```

locktest
--------

The ```locktest``` utility provided in this project is a C program that
can acquire a lock using either ```flock```, ```lockf```, or ```fcntl```.
Using this utility on two different cluster nodes, you can attempt to 
obtain a lock on a given file using any of these mechanisms, and see for
yourself whether what should be an exclusive lock is actually an
exclusive lock.

Run ```locktest``` with no arguments to see the command line syntax.

Example -- acquire a lock on a file named LOCK using ```flock```:
```
./locktest flock LOCK
```

This example uses the default ten second delay after acquiring the lock
before releasing it.


java LockTest
-------------

The ```LockTest.class``` is a Java program that acquires a lock using
the ```FileChannel.lock``` method. 

Run ```java LockTest``` to see the command line syntax.

Example --- acquire a lock on a file named LOCK
```
java LockTest LOCK
```

