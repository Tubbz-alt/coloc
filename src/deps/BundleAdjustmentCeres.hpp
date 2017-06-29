// Copyright (c) 2015 Pierre Moulon.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "openMVG/numeric/numeric.h"
#include "BundleAdjustment.hpp"

using namespace openMVG;

namespace ceres {
class CostFunction;
}

namespace openMVG {

	namespace cameras {
		struct IntrinsicBase;
	}
}

/// Create the appropriate cost functor according the provided input camera intrinsic model
/// Can be residual cost functor can be weighetd if desired (default 0.0 means no weight).
ceres::CostFunction * IntrinsicsToCostFunction
(
  cameras::IntrinsicBase * intrinsic,
  const Vec2 & observation,
  const double weight = 0.0
);

class Bundle_Adjustment_Ceres : public BA
{
  public:
  struct BA_Ceres_options
  {
    bool bVerbose_;
    unsigned int nb_threads_;
    bool bCeres_summary_;
    int linear_solver_type_;
    int preconditioner_type_;
    int sparse_linear_algebra_library_type_;
    double parameter_tolerance_;
    bool bUse_loss_function_;

    BA_Ceres_options(const bool bVerbose = true, bool bmultithreaded = true);
  };
  private:
    BA_Ceres_options ceres_options_;

  public:
  Bundle_Adjustment_Ceres(Bundle_Adjustment_Ceres::BA_Ceres_options options = BA_Ceres_options());

  BA_Ceres_options & ceres_options();

  bool Adjust
  (
    // the SfM scene to refine
	  sfm::SfM_Data & sfm_data,
    // tell which parameter needs to be adjusted
    const Optimize_Options options
  ) override;
};