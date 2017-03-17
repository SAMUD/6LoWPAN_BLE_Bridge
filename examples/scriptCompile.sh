#!/bin/bash      

echo $1
echo $2
cd $1

echo -------------
echo $HEAD
make TARGET=z1 $2




