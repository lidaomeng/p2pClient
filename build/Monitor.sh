#!/bin/bash

PROC=p2pclient
P_LOG=P2PERROR.log

#if ps -ef | grep "$PROC" | grep -v "grep"
PROCESS_NUM=`ps -ef | grep "$PROC" | grep -v "grep" |wc -l`

if [ $PROCESS_NUM = 1 ]; then
 echo -e "[`date "+%Y-%m-%d %H:%M:%S"`] : ERROR $PROC no exit, Restart..." >> /var/log/$P_LOG
 ./p2pclient &
 echo -e "[`date "+%Y-%m-%d %H:%M:%S"`] : `ps -ef |grep -v 'grep'|grep $PROC`" >> /var/log/$P_LOG
fi

