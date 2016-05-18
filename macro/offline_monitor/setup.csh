source /opt/phenix/bin/phenix_setup.csh -n new

mkdir -p install
setenv MYINSTALL /gpfs/mnt/gpfs02/phenix/mpcex/liankun/Run16/Ana/offline/analysis/mpcexcode/Liankun/install
setenv LD_LIBRARY_PATH $MYINSTALL/lib:${LD_LIBRARY_PATH}
setenv PATH $MYINSTALL/bin:${PATH}
