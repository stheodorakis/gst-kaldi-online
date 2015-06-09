#!/bin/bash

#ModelPath=../../../../models/english/fisher_english/s5/exp/tri5a_0.1/
ModelPath=../../../../models/italian/GestureChallenge

if [ $# != 1 ]; then
    echo "Usage: transcribe-audio.sh <audio>"
    echo "e.g.: transcribe-audio.sh dr_strangelove.mp3" 
    exit 1;
fi

#! GST_PLUGIN_PATH=../src gst-inspect-1.0 kaldigmm2onlinedecoder > /dev/null 2>&1 && echo "Compile the plugin in ../src first" && exit 1;

if [ ! -f ${ModelPath}/HCLG.fst ]; then
    echo "Run ./prepare-models.sh first to download models in path ${ModelPath}"
    exit 1;
fi

audio=$1

GST_PLUGIN_PATH=../src gst-launch-1.0 --gst-debug="" -q filesrc location=$audio ! decodebin ! audioconvert ! audioresample ! \
kaldigmm2onlinedecoder \
  model=${ModelPath}/kaldi.mdl \
  fst=${ModelPath}/HCLG.fst \
  word-syms=${ModelPath}/words_w.txt \
  global-cmvn-stats=${ModelPath}/cmvn_train_matsum.ark \
  feature-type=mfcc \
  mfcc-config=${ModelPath}/conf/mfcc.conf \
  add-deltas=true \
  max-active=7000 \
  beam=11.0 \
  lattice-beam=5.0 \
  acoustic-scale=0.1 \
  do-endpointing=true \
  endpoint-rule5-min-trailing-silence=1 \
  endpoint-rule5-min-utterance-length=20 \
  endpoint-silence-phones="1:22" \
  chunk-length-in-secs=0.2 \
! filesink location=/dev/stdout buffer-mode=2
