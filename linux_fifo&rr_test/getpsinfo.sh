#!/bin/sh

 for((i = 0; i < 40; i++))
 do
     ps -C test -o pid,pri,cmd,time,psr >>psinfo.log 2>&1
     sleep 2;
 done