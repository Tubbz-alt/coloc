#pragma once

#include "openMVG/features/feature.hpp"
#include "openMVG/sfm/pipelines/sfm_robust_model_estimation.hpp"
#include "openMVG/sfm/sfm_data.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#define Success 1
#define Failure 0

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::matching;
using namespace openMVG::sfm;


namespace coloc
{
	typedef std::vector<std::array<double, 6 * 6>> Cov6;
	typedef std::map<IndexT, std::unique_ptr<features::Regions> > FeatureMap;
	typedef std::map<Pair, RelativePose_Info> InterPoseMap;
	typedef SfM_Data Scene;

	class LocalizationParams {
	public:
		std::string imageFolder;
		std::string featureDetectorType;
		std::pair <size_t, size_t> imageSize;
		Mat3 K;
	};
}