#!/bin/bash
# 
# File:   runscript.sh
# Author: tbrown
#
# Created on Jan 19, 2018, 6:57:08 PM
#

 #cols="%10s %8s %8s %12s\n" ; printf "${cols}" alg threads trial throughput ; for threads in 1 2 3 4 ; do for alg in single multi ; do for ((i=0;i<3;++i)) ; do throughput=`./counter_$alg -t 1000 -n $threads -param 1 -bind 0-47 | grep "throughput" | cut -d"=" -f2 | cut -d"." -f1` ; printf "${cols}" $alg $threads $i $throughput ; done ; done ; done

t=1000
param=1
#algs="single_casonce single_casloop single multi multi_numa"
algs="multi_numa"
pinning="-bind 0-11,24-35,12-23,36-47"
trials=3
outfile="data.csv"

thread_counts="1"
for ((thr=4;thr<=48;thr+=4)); do
    thread_counts="$thread_counts $thr"
done

cols_pretty="%16s %8s %8s %12s %s\n"
cols_csv="%s,%s,%s,%s,%s\n"
cols_names="alg threads trial throughput cmd"

printf "${cols_pretty}" ${cols_names}
printf "${cols_csv}" ${cols_names} > $outfile
for alg in ${algs} ; do
    for n in ${thread_counts} ; do
        for ((trial=0;trial<trials;++trial)) ; do
            cmd="./counter_$alg -t $t -n $n -param $param $pinning"
            throughput=`$cmd | grep "throughput" | cut -d"=" -f2`
            data="$alg $n $trial $throughput"
            printf "${cols_pretty}" $data "$cmd"
            printf "${cols_csv}" $data "$cmd" >> $outfile
        done
    done
done
