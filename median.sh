#!/bin/sh

coordinates_file=$1
output_file=$2
schedules_file=$3
d=$4
hStart=$5
mStart=$6
hEnd=$7
mEnd=$8

h=$hStart
m=$mStart
deltaM=30
baseUrl="https://www.google.it/maps/dir/"

> $output_file

while [ $((h*100+m)) -lt $((hEnd*100+mEnd)) ]
do
	./median $coordinates_file $output_file $schedules_file $d $((h*100+m))
	m=$((m+deltaM))
	if [ $m -ge 60 ]
	then
		m=$((m-60))
		h=$((h+1))
	fi
done

firefox $baseUrl$(cat $output_file)
