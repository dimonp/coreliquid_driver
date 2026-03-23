#!/bin/bash

case $1/$2 in
    pre/*)
        killall -SIGTSTP my_msi_coreliquid_driver
        ;;
    post/*)
        killall -SIGCONT my_msi_coreliquid_driver
        ;;
esac