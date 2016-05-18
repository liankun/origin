#!/bin/bash
PD_Dir=/gpfs/mnt/gpfs02/phenix/mpcex/online_production/run16_mpcex_mpcex/
for i in `ls $PD_Dir`;
  do
    if [[ $i == "run"* ]]
      then
      echo "directory "$i
      MPCEX_Dir=${PD_Dir}/$i/DST_MPCEX
      Run_Number_List=`ls ${MPCEX_Dir} | cut -d "-" -f2 | sed 's/^0*//' | sort -u`
      echo "Run Number list: " ${Run_Number_List}
      for j in `echo ${Run_Number_List}`;
        do
	Run_Number=${j}
        Seg_Number=`ls ${MPCEX_Dir}/*${Run_Number}* | wc -l`
	Dst_num=`ls ${Run_Number}/Run16Ana*.root | wc -l`
        echo "Run Number: ${Run_Number} Number of segments: ${Seg_Number}"
	echo "Number of DSTs: ${Dst_num}"
        if [ ! -d "${Run_Number}" ] || [ ${Dst_num} -lt 2 ]
          then
	  echo "file ${Run_Number} doesn't exists or DSTs Number is small and will run Fun4All !"
	  mkdir -p ${Run_Number}
	  #1 stands for 0,run first segment
	  #run all segments
          for k in `ls ${MPCEX_Dir}/*${Run_Number}*`;
	    do
           true_seg=`echo ${k} | cut -d "-" -f3 | cut -d "." -f1`
           echo "${Run_Number} ${true_seg}"
          ./submit_to_condor.sh ${Run_Number} ${true_seg}
          done
	fi
      done
    fi
done
