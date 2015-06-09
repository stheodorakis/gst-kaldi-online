#! /bin/bash

BASE_URL=http://kaldi-asr.org/downloads/build/2/sandbox/online/egs/fisher_english/s5

MODEL=exp/tri5a
MODEL_Nnet2=exp/nnet2_online/nnet_a_gpu_online
GRAPH=exp/tri5a

wget -N $BASE_URL/$MODEL/final.mdl || exit 1
wget -N $BASE_URL/$GRAPH/graph/HCLG.fst || exit 1
wget -N $BASE_URL/$GRAPH/graph/words.txt || exit 1
(mkdir -p conf; cd conf; wget -N $BASE_URL/$MODEL_Nnet2/conf/{mfcc.conf,online_cmvn.conf,splice.conf}) || exit 1
