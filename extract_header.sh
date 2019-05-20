#!/bin/bash
# extract from c/c++ source files
# $1 c/c++ source files dir
# $2 extract header dir
ALL_OVER_PATH=$1
EXTRACT_PATH=$2;
echo "ALL_OVER_PATH:"$ALL_OVER_PATH"\n"
echo "EXTRACT_PATH:"$EXTRACT_PATH"\n"
function getdir(){
    for element in `ls $1`
    do
        dir_or_file=$1"/"$element
        if [ -d $dir_or_file ]
        then
            getdir $dir_or_file
        else
	    if [[ $dir_or_file =~ .*\.h$ ]]
	    then
		SPLIT_PATH=${dir_or_file/$ALL_OVER_PATH/}
    		OUTPUT_PATH=$EXTRACT_PATH$SPLIT_PATH
		DIRS=${OUTPUT_PATH%/*}
		mkdir -p $DIRS
                cp -f -v $dir_or_file $EXTRACT_PATH$SPLIT_PATH
	    fi
        fi
    done
}
getdir $1 
