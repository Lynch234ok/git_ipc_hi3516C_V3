#!/bin/sh

kill -9 `ps | grep startup.sh | head -n 1 | awk '{print $1}'`
kill -9 `pidof app`
rmmod watchdog

