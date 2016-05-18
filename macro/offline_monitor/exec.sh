#!/bin/csh

#####################################
#                                   #
# Inputs are: ${1} Run Number       #
#             ${2} Segment Number   #
#                                   #
#####################################

#make a directory to work in:
mkdir run_mpcex_production_${1}_${2}
cd    run_mpcex_production_${1}_${2}
set mydir = /gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/production/offline/analysis/mpcexcode/OwnProduction/
set outdir = /gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/OwnProduction/run_0000448000_0000449000/
set data_dir = /gpfs/mnt/gpfs02/phenix/mpcex/liankun/copy_from_dcache/run16/run_0000448000_0000449000/
#set temperate prdf store position if the disk space on condor is not large enough

source $mydir/setup.csh

# print out the env and hostname
#env
#hostname



###########################################################################
#copy the fun4all and output manager here
cp $mydir/Fun4AllMPCEX.C .
cp $mydir/OutputManager.C .
###########################################################################



#copy the PRDF files to the .
#echo "copy the prdf files"
#perl copy_prdf.pl EVENTDATA_P00-0000${1}-${2}.PRDFF .


#run events
env ODBCINI=/opt/phenix/etc/odbc.ini.mirror root.exe -l -b -q Fun4AllMPCEX.C'(0,"'${data_dir}'EVENTDATA_P00-0000'${1}'-00'`printf %02d ${2}`'.PRDFF")'

mv DST_MPCEX_MB/DST_MPCEX_MB*.root $outdir/

cd ../
rm -rf run_mpcex_production_${1}_${2}
