AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(includedir) -I$(OFFLINE_MAIN)/include -I$(ROOTSYS)/include

SUBDIRS = 

lib_LTLIBRARIES = \
  libMyMpcEx.la 

noinst_HEADERS = \
  MyMpcExLinkDef.h 

include_HEADERS = \
  mMpcExDataAna.h \
  MpcExEventQuality.h \
  MpcExHitMap.h \
  mMpcExMakeEventQuality.h \
  TMpcExLShower.h \
  TMpcExLShowerContainer.h \
  mMpcExLShower.h \
  mMpcExMyShower.h \
  mMpcExCreateTree.h \
  mMpcExAuAuAna.h \
  mMpcExLoadMyCalibrations.h
 

libMyMpcEx_la_SOURCES = \
  TMpcExLShower.C \
  TMpcExLShowerContainer.C \
  mMpcExDataAna.C \
  MpcExEventQuality.C \
  MpcExHitMap.C \
  mMpcExMakeEventQuality.C \
  mMpcExMyShower.C \
  mMpcExLShower.C \
  mMpcExCreateTree.C \
  mMpcExLoadMyCalibrations.C \
  mMpcExAuAuAna.C \
  MyMpcEx_Dict.C

libMyMpcEx_la_LIBADD = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -luspin \
  -lmpcex_base \
  -lmpcex_interface \
  -lmpcex_modules \
  -lmpcex_utils \
  -lMpcExReco

# Rule for generating CINT dictionaries from class headers.
MyMpcEx_Dict.C: \
  TMpcExLShower.h \
  TMpcExLShowerContainer.h \
  mMpcExDataAna.h \
  mMpcExMakeEventQuality.h \
  mMpcExMyShower.h \
  mMpcExLShower.h \
  mMpcExCreateTree.h \
  mMpcExLoadMyCalibrations.h \
  mMpcExAuAuAna.h \
  MyMpcExLinkDef.h 
	rootcint -f $@ -c $(DEFAULT_INCLUDES) $(INCLUDES) $^

clean-local:
	rm -f *Dict*

