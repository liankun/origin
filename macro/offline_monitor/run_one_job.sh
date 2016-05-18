#!/bin/csh

#####################################
#                                   #
# Inputs are: ${1} Run Number       #
#             ${2} index:           #
#                  0,first segment  #
#                  1,middle segment #
#                  2,last segment   #
#                                   #
#####################################

#make a directory to work in:
mkdir run_mpcex_${1}_${2}
cd    run_mpcex_${1}_${2}
set mydir=/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Liankun/macro/offline_monitor/
set outdir=/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Liankun/macro/offline_monitor/${1}/
set datadir=/gpfs/mnt/gpfs02/phenix/mpcex/online_production/run16_mpcex_mpcex/

set DST_EVE=`find ${datadir} -name "DST_EVE_*${1}-*${2}.root"`
set DST_MPC=`find ${datadir} -name "DST_MPC_*${1}-*${2}.root"`
set DST_MPCEX=`find ${datadir} -name "DST_MPCEX_*${1}-*${2}.root"`

source $mydir/setup.csh 

cp $mydir/Fun4All_MpcExDataAna.C .

root -l -b -q Fun4All_MpcExDataAna.C'("'${DST_MPCEX}'","'${DST_MPC}'","'${DST_EVE}'")'

mv *.root $outdir/
cd ../
rm -rf run_mpcex_${1}_${2}
