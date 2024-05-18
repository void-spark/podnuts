#!/bin/bash
#--------------------------------------------------------------
# boot sequence for the talker combining 'up' and old 'go'
# with some error checking to not copy files that are not there
# 
# written by: fck
#--------------------------------------------------------------

#--------------------------------------------------------------
# declare the PNUTS directory, and save users `pwd`
#--------------------------------------------------------------
PNUTS=$HOME/podnuts
PEXE=$HOME/podnuts/bin/pod.ex
OPWD=$PWD


#--------------------------------------------------------------
# move syslog to syslog.bak if the syslog exists
#--------------------------------------------------------------
#if [ -f $PNUTS/logfiles/syslog ];then
#	echo -n "Moving syslog to 'syslog.bak'..."
#	mv $PNUTS/logfiles/syslog $PNUTS/logfiles/syslog.bak
#	echo ..done
#fi


#--------------------------------------------------------------
# boot the talker after changing to the directory that the 
# pod.ex file is in, unless already in the directory
#--------------------------------------------------------------
if [ $OPWD != $PNUTS ];then
	echo -n "Changing to '$PNUTS'..."
	cd $PNUTS 
	echo ..done
fi

	# Booting the talker now
	command $PEXE

if [ $OPWD != $PNUTS ];then
	echo -n "Changing back to '$OPWD'..."
	cd $OPWD
	echo ..done
fi

#--------------------------------------------------------------
# end.
#--------------------------------------------------------------

