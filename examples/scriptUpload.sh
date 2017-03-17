#!/bin/bash       

if pgrep make; then
  echo "Close serial port before upload !"
else
  echo $1
  echo $2
  cd $1
  make TARGET=z1 $2.upload
  make TARGET=z1 login
fi

