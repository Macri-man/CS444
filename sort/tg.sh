#!/bin/bash

for i in `seq 1 $2`; 
do
  ./tg $3 > ./thread/$1$i'.txt'  
done
