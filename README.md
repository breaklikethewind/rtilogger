
rtilogger monitoring software written by Eric Nelson

This program uses a synology NAS to log debug messages. That data is sent to 
an RTI XP processor for display to the user.

The files are:

rtilogger.c
This is the entry point. This file manages capture & sending of app
level messages. This app opens a text file to log all captured data.
Futuer versions will add the ability to push the data to a mySQL database.

transport.c
This controls the communication to the RTI processor. The communciation
uses the RTI driver "two way strings".


Each driver, and transport.c are designed to be self contained re-usable
modules for other programs. 

The transport.c module contains the mechanism to manage the communication
to the RTI processor. Programs can re-use the transport module by defining a
command table (commandlist_t), and a push table (pushlist_t). The command
table defines what variables, or functions, are called when an XP processor request
arrives. The push table defines what periodic data is sent to the processor.

On the RTI processor two way strings driver, you must define the tag strings from
the push list, and the command strings from the tags in the command list. The 
full command list consists of tags defined in your application (sump.c in this case),
and tags defined in transport.c.


