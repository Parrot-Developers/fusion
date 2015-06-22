# fusion

## Quick overview

This project gathers different utility libraries and executables used in some
projects at Parrot.
The main design goals, shared by all these projects are:

 * robustness, by the mean of a large test code coverage
 * DRY, by providing easily reusable pieces of code
 * ***linux/glibc/gcc compatibility only***, which allows to use bleeding-edge
 features while keeping implementations free of compatibility code
 * readability and conciseness

## Special warning

The project is **not** considered stable at the moment, especially, the functions
being marked as deprecated **will** be removed in a near future.  
The same goes for the **io\_src\_msg\_uad.h** and **io\_src\_msg.h**, which will be
deprecated as soon as I'll have a generic socket support which I am satisfied
of.
On the contrary, most of the other pieces of code have been extensively tested
and used in many place, so they are not likely to change a lot in the near
future.

## Sub-projects

1. fautes  
It is composed of an executable, **fautes** and a library, **libfautes**, used to
ease the process of embedding an running automatic tests in a library or a
binary.
It is built around CUnit.
**libfautes** provides the definitions and functions necessary to support **fautes**
(see **fautes.h**), plus some functions / macros which comes in handy in the
process of writing tests (see **fautes\_utils.h**).  
**note:** shared objects running unit tests can be ran directly if crafted
specially, for exemple, please see the **CMakeLists.txt** for **libutils** and the
**ut\_fautes.c** file.
1. libioutils  
This is the main component of **fusion**.
It aims to provide two things.
First, a nice frame to perform asynchronous I/O around file descriptor, in a
similar way as libuv et al., but **recursively**.
Second, it aims to provide file descriptor based facilities to monitor I/O
events, for the widest kind of event types possible, e.g. timers, signals,
processes...
It is still incomplete (mainly lacking support for sockets), but is still usable
(and used...).
1. librs  
This library aims to gather robust implementations for sets.
It provides doubly-linked nodes, for higher level sets implementations (see
**rs\_node.h**), doubly-linked lists implementation (based on rs\_node.h, see
**rs\_dll.h**), "magical" ring buffers (wrapping will never be an issue again,
see **rs\_rb.h**), hash maps (see **rs\_hmap.h**).
1. libutils  
Could have been named libstuff, libmisc...
Gathers what didn't fit in standalone libraries.
Provides utilities for bits, files, kernel modules, processes and strings
manipulation.
1. libpidwatch  
Small library, which allows to monitor the death of a process with a file
descriptor.
Sadly, because it is based on the proc\_connector linux facility, it
requires **CAP\_NET\_ADMIN**.
This restriction contaminates the io process and src\_pid submodules, which rely
on this library.
For now it has not been a big deal because we use it only in root processes,
but ideas for a non-priviledged implementation (**NOT** based on SIGCHLD) will
be warmly welcome.

## License

The fusion project is provided under a **3-clauses BSD license**.
Please refer to the **LICENSE** file for more information.

## Contributions

Until either a CLA or a CAA has been decided and published by Parrot, no
contribution under any form, will be integrated.
It includes (but not limited to) pull requests and emailed patches.
I hope we will able to lift this limitation soon.
