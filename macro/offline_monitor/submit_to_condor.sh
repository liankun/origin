#!/bin/sh
#####################################
#                                   #
# Inputs are: ${1} Run Number       #
#             ${2} Segment Number   #
#                                   #
#####################################

if [ $# -lt 2 ]; then
    echo " scripts needs input variables: Run and Segment Number "
    exit
fi

echo " submitting: run " ${1} "(" ${2} ") into the condor queue executing:" submit_to_condor.csh
/bin/rm -f  job_condor_${1}_${2}

dir="/gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Liankun/macro/offline_monitor"
ldir=$dir"/logs/"

cat >  job_condor_${1}_${2} << +EOF
    
universe     = vanilla
Executable   = run_one_job.sh
Arguments    = $1 $2
#Requirements = (ARCH=="INTEL" || ARCH=="X86_64")
GetEnv       = True
InitialDir   = $dir
Input        = /dev/null
Output       = $ldir/mpcex_logs_${1}_${2}.stdout
Error        = $ldir/mpcex_logs_${1}_${2}.stderr
Log          = $ldir/mpcex_logs_${1}_${2}.log
should_transfer_files = YES
when_to_transfer_output = ON_EXIT_OR_EVICT
+Experiment     = "phenix"

Queue
+EOF

/usr/bin/condor_submit  job_condor_${1}_${2}
