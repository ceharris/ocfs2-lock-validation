ocfs2-lock-validation
=====================

This small project provides a couple of useful utilities for 
validating lock support on an OCFS2 filesystem.  Historically, OCFS2
supported only advisory locks using the ```flock```(2) system call as
documented in [1] section III, item 10, page 12.

OCFS2 supports POSIX style locks (see ```lockf```(3) and ```fcntl```(2))
but only when using a **userspace cluster stack**.  This is documented 
in [2] in the section entitled _Cluster aware POSIX file locks_.  The 
feature is supported in Linux Kernel 2.6.28 and higher.  Apparently, the 
default O2CB cluster stack is a kernel-based cluster stack not a 
userspace cluster stack.

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

References
----------

1. http://oss.oracle.com/projects/ocfs2/dist/documentation/v1.4/ocfs2-1_4-usersguide.pdf
2. https://oss.oracle.com/osswiki/OCFS2/NewFeaturesList.html

