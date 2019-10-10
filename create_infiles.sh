#!/bin/bash

if [ "$#" -ne 4 ]; then
    echo "Wrong arguments"
    exit 1
fi

if [ "$2" -lt 0 ]; then
	echo "Invalid number of files"
	exit 1 
fi

if [ "$3" -lt 0 ]; then
	echo "Invalid number of dirs"
	exit 1
fi

if [ "$4" -lt 0 ]; then
	echo "Invalid number of levels"	
	exit 1
fi

if [ -r $1 ]; then
	echo "directory exists"
	exit 1
else
	echo "i will create dir"
	mkdir ./$1
fi 

#width=$((($RANDOM%8)+1))
#name $(head /dev/urandom | tr -dc A-Za-z0-9 | head -c $width )

files=0
dirs=0
path="./$1/"

while [ "$dirs" -lt "$3" ]
do
		levels=0
		path="./$1/"
		#touch "$path/$files.txt"
		for((levels=0; levels< $4; levels++))
		do
			if [ "$dirs" -ge "$3" ]; then
			 	break;
			fi
			width=$((($RANDOM%8)+1))
			name=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c $width )
			path+="$name/"			

			mkdir -p $path
			dirs=`expr $dirs + 1`
			
		done
		
	
done

while [ "$files" -lt "$2" ]
do
	dirs=0
	i=1
	
	while [ "$dirs" -lt "$3" ]
	do  
		if [ "$files" -ge "$2" ]; then
			 	break;
		fi
	
		#touch "./$1/$files.txt"
		size=$((($RANDOM%128)+1))
		width=$((($RANDOM%8)+1))
		name=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c $width )
		dd bs=1024 count=$size if=/dev/urandom | tr -dc A-Za-z0-9 > ./$1/$name.txt  
	 
		files=`expr $files + 1`

		if [ "$files" -ge "$2" ]; then
			 	break;
		fi
		echo "i" $i
		last= ls -lc ./$1 | grep ^d | awk '{print $NF}' |sed -n "$i"p

		
				
		path="./$1/$(ls -lc ./$1 | grep ^d | awk '{print $NF}' |sed -n "$i"p)"
		
		if [ ! -d "$path" ]; then
				echo "NO path"
				break;
		fi
		
		#touch "$path/$files.txt"
		size=$((($RANDOM%128)+1))
		width=$((($RANDOM%8)+1))
		name=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c $width )
		dd bs=1024 count=$size if=/dev/urandom  | tr -dc A-Za-z0-9 > $path/$name.txt

		dirs=`expr $dirs + 1`
		files=`expr $files + 1`
		
		for((levels=1; levels< $4; levels++))
		do
			if [ "$dirs" -ge "$3" ]; then
			 	break;
			fi
			if [ "$files" -ge "$2" ];then
				break;
			fi

			echo "lev" $levels
			path+="/$(ls -lc $path | grep ^d | awk '{print $NF}' | sed -n 1p)"
			if [ ! -d "$path" ]; then
				echo "NO path"
				break;
			fi
			
			
			#touch "$path/$files.txt"
			size=$((($RANDOM%128)+1))
			width=$((($RANDOM%8)+1))
			name=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c $width )
			dd bs=1024 count=$size if=/dev/urandom | tr -dc A-Za-z0-9 > $path/$name.txt
			dirs=`expr $dirs + 1`
			files=`expr $files + 1`

		done
		i=`expr $i + 1`
		

	done
done
