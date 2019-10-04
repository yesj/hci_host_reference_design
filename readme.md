# Introduction
The hci_host_reference_design is a simple CLI application interfacing to GEM series of modules from North Pole Engineering. The purpose of this application is to demonstrate the capabiliities of the GEM and, more importantly, to provide a cross platform reference design for interfacing to GEM from a host microprocessor. The code is written in ANSI 'C' standard using POSIX threads. It compiles on Windows (Msys2), Linux, MacOS, Android, FreeBSD and others. 

This application requires the libserialport library. This is a free, cross platform 'C' based serial port library available from sigrok.org.

# Architecture
![stack](./img/hci_reference_design.png)

## main
The 'main' module implements the UI as a simple command line interface and also generates simulated data which is sent to the GEM at 1Hz. 

The CLI has the following options:

'h' - help              <= Prints the Help menu
'a' - start advertising <= Starts Advertising on the GEM
's' - stop advertising  <= Stops Advertising on the GEM
'i' - goto IDLE         <= Puts the GEM into the IDLE stte
'u' - goto IN-USE       <= Puts the GEM into the IN-USE state
'p' - goto PAUSED       <= Puts the GEM into the PAUSED state
'f' - goto FINISHED     <= Puts the GEM in tot he FINISHED state
'+' - increase grade    <= Increases the grade sent to the GEM by 5%
'-' - decrease grade    <= Decreases the grade sent to the GEM by 5%
'q' - quit              <= Quits the application

# Installation