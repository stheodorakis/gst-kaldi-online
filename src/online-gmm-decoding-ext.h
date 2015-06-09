// online2/online-gmm-decoding.h

// Copyright 2014  Johns Hopkins University (author: Daniel Povey)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.


#ifndef KALDI_ONLINE2_ONLINE_GMM_DECODING_EXT_H_
#define KALDI_ONLINE2_ONLINE_GMM_DECODING_EXT_H_

#include <string>
#include <vector>
#include <deque>

#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"
#include "transform/basis-fmllr-diag-gmm.h"
#include "transform/fmllr-diag-gmm.h"
#include "online2/online-feature-pipeline.h"
#include "online2/online-gmm-decodable.h"
#include "online2/online-endpoint.h"
#include "online2/online-gmm-decoding.h"
#include "decoder/lattice-faster-online-decoder.h"
#include "hmm/transition-model.h"
#include "gmm/am-diag-gmm.h"
#include "hmm/posterior.h"
#include "lat/lattice-functions.h"
#include "lat/determinize-lattice-pruned.h"


namespace kaldi {

/**
   You will instantiate this class when you want to decode a single
   utterance using the online-decoding setup.  This is an alternative
   to manually putting things together yourself.
*/
class SingleUtteranceGmmDecoderExt {
 public:
  SingleUtteranceGmmDecoderExt(const OnlineGmmDecodingConfig &config,
                            const OnlineGmmDecodingModels &models,
                            const OnlineFeaturePipeline &feature_prototype,
                            const fst::Fst<fst::StdArc> &fst,
                            const OnlineGmmAdaptationState &adaptation_state);
  
  OnlineFeaturePipeline &FeaturePipeline() { return *feature_pipeline_; }

  /// advance the decoding as far as we can.  May also estimate fMLLR after
  /// advancing the decoding, depending on the configuration values in
  /// config_.adaptation_policy_opts.  [Note: we expect the user will also call
  /// EstimateFmllr() at utterance end, which should generally improve the
  /// quality of the estimated transforms, although we don't rely on this].
  void AdvanceDecoding();

  /// Finalize the decoding. Cleanups and prunes remaining tokens, so the final result
  /// is faster to obtain.
  void FinalizeDecoding();

  /// Returns true if we already have an fMLLR transform.  The user will
  /// already know this; the call is for convenience.  
  bool HaveTransform() const;
  
  /// Estimate the [basis-]fMLLR transform and apply it to the features.
  /// This will get used if you call RescoreLattice() or if you just
  /// continue decoding; however to get it applied retroactively
  /// you'd have to call RescoreLattice().
  /// "end_of_utterance" just affects how we interpret the final-probs in the
  /// lattice.  This should generally be true if you think you've reached
  /// the end of the grammar, and false otherwise.
  void EstimateFmllr(bool end_of_utterance);
  
  void GetAdaptationState(OnlineGmmAdaptationState *adaptation_state) const;

  /// Gets the lattice.  If rescore_if_needed is true, and if there is any point
  /// in rescoring the state-level lattice (see RescoringIsNeeded()), it will
  /// rescore the lattice.  The output lattice has any acoustic scaling in it
  /// (which will typically be desirable in an online-decoding context); if you
  /// want an un-scaled lattice, scale it using ScaleLattice() with the inverse
  /// of the acoustic weight.  "end_of_utterance" will be true if you want the
  /// final-probs to be included.
  void GetLattice(bool rescore_if_needed,
                  bool end_of_utterance,
                  CompactLattice *clat) const;

  /// Outputs an FST corresponding to the single best path through the current
  /// lattice. If "use_final_probs" is true AND we reached the final-state of
  /// the graph then it will include those as final-probs, else it will treat
  /// all final-probs as one.
  void GetBestPath(bool end_of_utterance,
                   Lattice *best_path) const;

  /// This function outputs to "final_relative_cost", if non-NULL, a number >= 0
  /// that will be close to zero if the final-probs were close to the best probs
  /// active on the final frame.  (the output to final_relative_cost is based on
  /// the first-pass decoding).  If it's close to zero (e.g. < 5, as a guess),
  /// it means you reached the end of the grammar with good probability, which
  /// can be taken as a good sign that the input was OK.
  BaseFloat FinalRelativeCost() { return decoder_.FinalRelativeCost(); }


  /// This function calls EndpointDetected from online-endpoint.h,
  /// with the required arguments.
  bool EndpointDetected(const OnlineEndpointConfig &config);

  ~SingleUtteranceGmmDecoderExt();
 private:
  bool GetGaussianPosteriors(bool end_of_utterance, GaussPost *gpost);

  /// Returns true if doing a lattice rescoring pass would have any point, i.e.
  /// if we have estimated fMLLR during this utterance, or if we have a
  /// discriminative model that differs from the fMLLR model *and* we currently
  /// have fMLLR features.
  bool RescoringIsNeeded() const;

  OnlineGmmDecodingConfig config_;
  std::vector<int32> silence_phones_; // sorted, unique list of silence phones,
                                      // derived from config_
  const OnlineGmmDecodingModels &models_;
  OnlineFeaturePipeline *feature_pipeline_;  // owned here.
  const OnlineGmmAdaptationState &orig_adaptation_state_;
  // adaptation_state_ generally reflects the "current" state of the
  // adaptation. Note: adaptation_state_.cmvn_state is just copied from
  // orig_adaptation_state, the function GetAdaptationState() gets the CMVN
  // state.
  OnlineGmmAdaptationState adaptation_state_;
  LatticeFasterOnlineDecoder decoder_;
};

}
#endif  // KALDI_ONLINE2_ONLINE_GMM_DECODING_EXT_H_
