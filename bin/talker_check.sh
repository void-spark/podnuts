#!/bin/bash
PROC="NULL"
TALK=99
PATH=/bin:/usr/bin:~/podnuts/bin
PROC=`ps -x`
TALK=`echo $PROC | grep -c 'pod.ex'`
if [ ${TALK} -lt 1 ]
then
   go.sh
fi
