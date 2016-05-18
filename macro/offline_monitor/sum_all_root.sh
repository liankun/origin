#! /bin/bash
for i in `ls -d 45*`;
  do
  echo $i
  Num=`ls $i/Run16*.root |wc -l`
  Num_gif=`ls $i/*.gif |wc -l`
  echo $Num
  echo $Num_gif
  if [ $Num -gt 0 ] && [ $Num_gif -eq 0 ];
    then
    echo "run merge for this directory"
    ./sum_root.sh ${i}
  fi
done
