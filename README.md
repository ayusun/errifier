errifier
========

An error Notifying application that makes error discovery faster

Why errifier
============

errifier was made with the idea to make the error discovery faster. That means, to get notified about the error as soon as it occurs. Without actually constantly looking at the logs, waiting for the error to occur, or to wait for someone else to tell you that "this particular thing isn't working", in which case again you will look at the logs to know what actually happened.

errifier makes your job easy, by looking for errors and then by notifying you about it, when it occurs; and in the mean time you can actually work on what ever useful thing you were working on.


Let the error occur when it has to occur
---------------------------------------

How To Use It?
==============

Following a scenario that i believe, where it can be useful, however you are free to think of any other scenario that you think of

So, Suppose you are a server guy, who has to manage the server. That means you know, the importance that error log plays in your work right? So traditionally how you were managing logs: You open log and see whats the timestamp of last error, Or you would open your mail for logs that contain error and what not. So, did you see, how much time is wasted on opening those logs and then locating errors.

Now errifier does this job for you, the simple, fast, easy to use and lightweight application will notify you about the error as soon as it occurs. You even get to choose when to monitor that particular log and when not, all it requires is a click. You can even manage logs of remote servers.

Supported Platforms
===================

Only Linux(Tested on Ubuntu, should work on others too) as of now


Dependency
==========

Server
------
* Thrift
* Boost
* inotify

Client
------
* QT
* libnotify
* Thrift
* Boost

Installation
============

Server
------

* Install all the dependency
* Go to the Server directory
* Use make server
* A Server Executable will be generated

Client
------

* First do remember to install all the dependency
* Go to the Client Directory
* qmake -o "Makefile" errifier.pro
* make
* errifier executable will be generated

Double click errifier(application generated to run the appliation)

**[OPTIONAL = To Use SSL in ERRIFIER]**

The first step is to generate all the keys

Generate Server Private Key by 
```
openssl genrsa -out server-key.pem 2048
```
Generate Server Public key and then self sign it by 
```
openssl req -new -x509 -key server-key.pem -out server-cert.pem -days 10000
```

Generate Client Private key by 
```
openssl genrsa -out client-key.pem 2048
```
Generate Client Public Key and self sign it by 
```
openssl req -new -x509 -key client-key.pem -out client-cert.pem -days 10000
```
> For the client put client-key.pem, client-cert.pem and server-cert.pem in the folder where executable is present
> For the server put server-key.pem, server-cert.pem and client-cert.pem in the folder where server executable is located

Thats how you add certificates for one server and one client

>To allow the client to connect to multiple server. Concatinate the server-cert.pem of those servers using cat command and name it as server-cert.pem.


Contact Me
==========

E-Mail: ayush.choubey@gmail.com

