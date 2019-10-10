#!/bin/bash

if [ -p /dev/stdin ]; then
        echo "Data was piped to this script!"
       
        STDIN=$(cat)
       
        echo "Number of clients: "
        echo "$STDIN" |grep -ci '!id'
       
        ids=$(echo "$STDIN" |grep -w '!id'  | awk '{print $2}')
        echo "IDs:" "$ids"
        echo "min"
        echo "$ids" | sort -n | head -1
        echo "max"
        echo "$ids" | sort -n | tail -1

        
        echo "Number of files read: "
        echo "$STDIN" |grep -ci '!r'
       
     	
     	SUM_R=0
		echo "Total number of bytes read: " 
		echo "$STDIN" |grep -w '!r'  | awk '{ SUM_R += $2} END { print SUM_R }' #vriskei oles tis grammes pou exoun !r kai prosthetei to deutero orisma ths grammhs


		echo "Number of files written: "
        echo "$STDIN" |grep -ci '!w' #metraei oles tis grammes me !w
		    
       	SUM_W=0
		echo "Total number of bytes written:" 
		echo "$STDIN" |grep -w '!w'  | awk '{ SUM_W += $2} END { print SUM_W }'

		echo "Number of clients exited: "
        echo "$STDIN" |grep -ci '!e'

       
else
        echo "No logfiles were found on stdin!"
        
fi
