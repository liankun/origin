#! /bin/bash
for i in `ls -d 45*`;
  do
  echo $i
  Num_gif=`ls $i/*gif | wc -l`
  echo "Number of Histogram: $Num_gif"
  if test $Num_gif -eq 64;
    then 
    echo "make webpage for this run"
    mkdir -p /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i
    cp $i/*gif /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_adc.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_adc.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_hl.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_hl.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_sensor_adc.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_sensor_adc.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_layer_mean_adc.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_layer_mean_adc.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_layer_adc_combine.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_layer_adc_combine.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_exogram.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_exogram.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpc_tower.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpc_tower.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_vs_bbc.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_vs_bbc.html
    sed 's/454782/'${i}'/g' /direct/phenix+WWW/p/draft/gukuoo/run16/QA/test_mpcex_mpc.html > /direct/phenix+WWW/p/draft/gukuoo/run16/QA/$i/mpcex_mpc.html
    echo $i >> /direct/phenix+WWW/p/draft/gukuoo/run16/QA/run_Number.txt
  fi
done

