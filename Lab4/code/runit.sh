#!/bin/bash


# Author: Hubertus Franke  (frankeh@nyu.edu)
OUTDIR=${1:-.}
shift
SCHED=${*:-./iosched}

echo "sched=<$SCHED> outdir=<$OUTDIR>"

INS="`seq 0 6`"
INPRE="input"
OUTPRE="out"

SCHEDS="f s S C F"


############################################################################
#  NO TRACING 
############################################################################

# run with RFILE1 
#ulimit -v 300000   # just limit the processes 

for f in ${INS}; do
	for s in ${SCHEDS}; do 
		echo "${SCHED} -s${s} ${INPRE}${f}"
		${SCHED} -s${s} ${INPRE}${f} > ${OUTDIR}/${OUTPRE}${f}_${s} 
	done
done

#for f in ${INS}; do
#	for s in ${SCHEDS}; do 
#		echo "${SCHED} -s${s} ${INPRE}${f}"
#		${SCHED} -v -d -s${s} ${INPRE}${f} > ${OUTDIR}/${OUTPRE}${f}_${s}_long
#	done
#done

