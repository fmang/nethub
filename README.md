nethub
======

`nethub` opens a Unix-domain or TCP server and listens for incoming
connections.  When some data is received from one of the clients, it is
forwarded to all the other clients; pretty much like a multi-user chat.

Used in conjuction with a software like `socat`, it allows connection
sharing (e.g. run several IRC bots on the same connection). Add OpenBSD's
`nc` and you'll be able to inject data into the shared connection. Note that
in that case both side will catch it.

Using `socat`'s unidirectional mode (`-u`), you could also broadcast data
to multiple clients (e.g. audio streams, or notifications).

Requirements
------------

Not much. All it should need is a POSIX compliant system.

Installing
----------

Just use make:

    make
    make DESTDIR=/usr/local install

Documentation
-------------

    Usage: nethub --help
           nethub [-v] -u SOCKET
           nethub [-v] [-46] [-l ADDRESS] -p PORT

    Options:
      -h, --help           print this help
      -v, --verbose        no need to explain
      -n, --slots NUM      specify the maximum number of clients
      -u, --socket PATH    create a Unix domain socket
      -p, --port PORT      open a TCP server
      -4, --ipv4           force IPv4
      -6, --ipv6           force IPv6
      -l, --bind ADDRESS   specify the address to bind (TCP)

See the man page, `nethub.1`, for extensive documentation.
