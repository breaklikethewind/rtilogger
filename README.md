rtilogger monitoring software written by Eric Nelson

This program uses a synology NAS (or any Linux box) to log debug messages. 
That data is sent to an RTI XP processor for display to the user.

The files are:

# rtilogger.c
This is the entry point. This file manages capture & sending of app
level messages. This app opens a text file to log all captured data.
Futuer versions will add the ability to push the data to a mySQL database.

# transport.c
This controls the communication to the RTI processor. The communciation
uses the RTI driver "two way strings".


Each driver, and transport.c are designed to be self contained re-usable
modules for other programs. 

The transport.c module contains the mechanism to manage the communication
to the RTI processor. Programs can re-use the transport module by defining a
command table (**commandlist_t**), and a push table (**pushlist_t**). The command
table defines what variables, or functions, are called when an XP processor request
arrives. The push table defines what periodic data is sent to the processor.

On the RTI processor two way strings driver, you must define the tag strings from
the push list, and the command strings from the tags in the command list. The 
full command list consists of tags defined in your application (rtilogger.c in this case),
and tags defined in transport.c.

*Note: The RTI two way strings driver referenced above, is a modified version of the original RTI two way strings driver. This new two way strings driver includes the ability to transmit strings containing SYSVAR values.*

The RTI GIT repository for RTI Logger is:
git@gitlab.rticorp.local:ericn/RTILogger.git

The RTI GIT repository for the modified two way strings driver is:
git@gitlab.rticorp.local:ericn/TwoWayV2.git



