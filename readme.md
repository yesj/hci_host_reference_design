# Introduction
The hci_host_reference_design is a simple CLI application interfacing to GEM series of modules from North Pole Engineering. The purpose of this application is to demonstrate the capabiliities of the GEM and, more importantly, to provide a cross platform reference design for interfacing to GEM from a host microprocessor. The code is written in ANSI 'C' standard using POSIX threads. It compiles on windows (msys2), linux, macos, FreeBSD and others. 

This application requires the libserialport library. This is a free, cross platform 'C' based serial port library available from sigrok.org.

# Architecture
![stack](./img/hci_reference_design.png)