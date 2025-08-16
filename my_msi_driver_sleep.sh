#!/bin/bash

case $1/$2 in
    pre/*)
        killall -SIGTSTP my_msi_driver
        ;;
    post/*)
        killall -SIGCONT my_msi_driver
        ;;
esac