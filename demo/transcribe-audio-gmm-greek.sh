#!/bin/bash

#ModelPath=../../../../models/english/fisher_english/s5/exp/tri5a_0.1/
ModelPath=../../../../models/greek/logotypografia/acoustic/tri2b_lda_mllt
LangPath=../../../../models/greek/logotypografia/language/digits

if [ $# != 1 ]; then
    echo "Usage: transcribe-audio.sh <audio>"
    echo "e.g.: transcribe-audio.sh dr_strangelove.mp3" 
    exit 1;
fi

#! GST_PLUGIN_PATH=../src gst-inspect-1.0 kaldigmm2onlinedecoder > /dev/null 2>&1 && echo "Compile the plugin in ../src first" && exit 1;

if [ ! -f ${LangPath}/HCLG.fst ]; then
    echo "Run ./prepare-models.sh first to download models in path ${ModelPath}"
    exit 1;
fi

audio=$1

GST_PLUGIN_PATH=../src gst-launch-1.0 --gst-debug="" -q filesrc location=$audio ! decodebin ! audioconvert ! audioresample ! \
kaldigmm2onlinedecoder \
  model=${ModelPath}/kaldi.mdl \
  fst=${LangPath}/HCLG.fst \
  word-syms=${LangPath}/words.txt \
  global-cmvn-stats=${ModelPath}/global_cmvn.mat \
  feature-type=mfcc \
  mfcc-config=${ModelPath}/conf/mfcc.conf \
  lda-matrix=${ModelPath}/final.mat \
  splice-feats=true \
  splice-config=${ModelPath}/conf/splice.conf \
  max-active=7000 \
  beam=13.0 \
  lattice-beam=6.0 \
  acoustic-scale=0.0833333 \
  do-endpointing=true \
  endpoint-rule5-min-trailing-silence=1 \
  endpoint-rule5-min-utterance-length=20 \
  endpoint-silence-phones="1:2:3" \
  chunk-length-in-secs=0.2 \
! filesink location=/dev/stdout buffer-mode=2
