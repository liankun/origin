#!/bin/bash
comment=$1
old_time=`stat -c %Y update_bench`
echo "old time `stat -c %y update_bench` "
for i in `ls`
  do 
    if [[ $i == *.[hC] ]] || [[ $i == *.am ]]; then
      if grep -q $i CVS/Entries ; then
         echo "$i is already in CVS"
	 file_time=`stat -c %Y $i`
         echo "$i last modified time `stat -c %y $i`"
         if [ $file_time -gt $old_time ] ; then
           echo "$i is modified and will upload to CVS "
	   cvs update $i
	   cvs commit -m "$comment" $i
         fi
       else
         echo "$i is not in CVS and will add to CVS"
	   cvs add $i
	   cvs commit -m "$comment" $i
       fi
    fi
done

rm update_bench
current_time=`date +%Y%m%d%H%M`
touch -t ${current_time} update_bench
