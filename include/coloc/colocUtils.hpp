#pragma once

#pragma once

#include "openMVG/features/akaze/image_describer_akaze.hpp"
#include "openMVG/features/image_describer_akaze_io.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/features/descriptor.hpp"
#include "openMVG/features/feature.hpp"
#include "openMVG/matching/indMatch.hpp"
#include "openMVG/matching/indMatch_utils.hpp"
#include "openMVG/matching_image_collection/Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/Cascade_Hashing_Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/GeometricFilter.hpp"
#include "openMVG/sfm/pipelines/sfm_robust_model_estimation.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider_cache.hpp"
#include "openMVG/matching_image_collection/F_ACRobust.hpp"
#include "openMVG/matching_image_collection/E_ACRobust.hpp"
#include "openMVG/matching_image_collection/H_ACRobust.hpp"
#include "openMVG/matching_image_collection/Pair_Builder.hpp"
#include "openMVG/matching/pairwiseAdjacencyDisplay.hpp"
#include "openMVG/matching/regions_matcher.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/stl/stl.hpp"
#include "openMVG/system/timer.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/sfm/pipelines/sequential/sequential_SfM.hpp"
#include "openMVG/geometry/pose3.hpp"
#include "openMVG/multiview/triangulation.hpp"
#include "openMVG/numeric/eigen_alias_definition.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/pipelines/localization/SfM_Localizer.hpp"
#include "openMVG/sfm/pipelines/sfm_robust_model_estimation.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_BA.hpp"
#include "openMVG/sfm/sfm_data_BA_ceres.hpp"
#include "openMVG/sfm/sfm_data_filters.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/stl/stl.hpp"
#include "openMVG/multiview/triangulation.hpp"
#include "openMVG/matching/svg_matches.hpp"

#include "third_party/cmdLine/cmdLine.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include "colocParams.hpp"
#include "colocData.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <dlib/optimization.h>

using namespace openMVG;
using namespace openMVG::features;
using namespace openMVG::matching;
using namespace openMVG::sfm;

using namespace dlib;

namespace coloc
{
	class Utils
	{
	public:
		double computeScaleDifference(colocParams& params, colocData &data1, colocData &data2, std::vector<IndMatch> commonFeatures);
		bool matchSceneWithMap(Scene& scene);
		bool rescaleMap(Scene& scene, double scale);
		int drawFeaturePoints(std::string& imageName, features::PointFeatures points);
		bool drawMatches(std::string& outputFilename, std::string& image1, std::string& image2, Regions& regions1, Regions& regions2, std::vector <IndMatch>& matches);

		void loadPoseCovariance(Cov6& cov, matrix<double, 3, 3>& covNew)
		{
			covNew = cov.at(0)[21], cov.at(0)[22], cov.at(0)[23],
				cov.at(0)[27], cov.at(0)[28], cov.at(0)[29],
				cov.at(0)[33], cov.at(0)[34], cov.at(0)[35];
		}

		void readPoseCovariance(matrix<double, 3, 3>& covNew, Cov6& cov)
		{
			cov.at(0)[21] = covNew(0, 0);
			cov.at(0)[22] = covNew(0, 1);
			cov.at(0)[23] = covNew(0, 2);
			cov.at(0)[27] = covNew(1, 0);
			cov.at(0)[28] = covNew(1, 1);
			cov.at(0)[29] = covNew(1, 2);
			cov.at(0)[33] = covNew(2, 0);
			cov.at(0)[34] = covNew(2, 1);
			cov.at(0)[35] = covNew(2, 2);
		}

		static Pair_Set handlePairs(const uint8_t numImages)
		{
			return exhaustivePairs(numImages);
		}

		static cv::Mat rot2euler(const cv::Mat & rotationMatrix)
		{
			cv::Mat euler(3, 1, CV_64F);

			double m00 = rotationMatrix.at<double>(0, 0);
			double m02 = rotationMatrix.at<double>(0, 2);
			double m10 = rotationMatrix.at<double>(1, 0);
			double m11 = rotationMatrix.at<double>(1, 1);
			double m12 = rotationMatrix.at<double>(1, 2);
			double m20 = rotationMatrix.at<double>(2, 0);
			double m22 = rotationMatrix.at<double>(2, 2);

			double bank, attitude, heading;

			// Assuming the angles are in radians.
			if (m10 > 0.998) { // singularity at north pole
				bank = 0;
				attitude = CV_PI / 2;
				heading = atan2(m02, m22);
			}
			else if (m10 < -0.998) { // singularity at south pole
				bank = 0;
				attitude = -CV_PI / 2;
				heading = atan2(m02, m22);
			}
			else
			{
				bank = atan2(-m12, m11);
				attitude = asin(m10);
				heading = atan2(-m20, m00);
			}

			euler.at<double>(0) = bank;
			euler.at<double>(1) = attitude;
			euler.at<double>(2) = heading;

			return euler;
		}

		static cv::Mat euler2rot(const cv::Mat & euler)
		{
			cv::Mat rotationMatrix(3, 3, CV_64F);

			double bank = euler.at<double>(0);
			double attitude = euler.at<double>(1);
			double heading = euler.at<double>(2);

			// Assuming the angles are in radians.
			double ch = cos(heading);
			double sh = sin(heading);
			double ca = cos(attitude);
			double sa = sin(attitude);
			double cb = cos(bank);
			double sb = sin(bank);

			double m00, m01, m02, m10, m11, m12, m20, m21, m22;

			m00 = ch * ca;
			m01 = sh * sb - ch * sa*cb;
			m02 = ch * sa*sb + sh * cb;
			m10 = sa;
			m11 = ca * cb;
			m12 = -ca * sb;
			m20 = -sh * ca;
			m21 = sh * sa*cb + ch * sb;
			m22 = -sh * sa*sb + ch * cb;

			rotationMatrix.at<double>(0, 0) = m00;
			rotationMatrix.at<double>(0, 1) = m01;
			rotationMatrix.at<double>(0, 2) = m02;
			rotationMatrix.at<double>(1, 0) = m10;
			rotationMatrix.at<double>(1, 1) = m11;
			rotationMatrix.at<double>(1, 2) = m12;
			rotationMatrix.at<double>(2, 0) = m20;
			rotationMatrix.at<double>(2, 1) = m21;
			rotationMatrix.at<double>(2, 2) = m22;

			return rotationMatrix;
		}


	private:

		std::shared_ptr<Regions_Provider> mapFeatures;
		EMatcherType matchingType;
	};

	bool Utils::drawMatches(std::string& outputFilename, std::string& image1, std::string& image2, Regions& regions1, Regions& regions2, std::vector <IndMatch>& matches)
	{
		Matches2SVG(
			image1, { 1280, 720 }, regions1.GetRegionsPositions(),
			image2, { 1280, 720 }, regions2.GetRegionsPositions(),
			matches, outputFilename, true);
		return EXIT_SUCCESS;
	}

	double Utils::computeScaleDifference(colocParams& params, colocData &data1, colocData &data2, std::vector<IndMatch> commonFeatures)
	{
		if (commonFeatures.empty()) {
			std::cout << "No common features between the maps." << std::endl;
			return 1.0;
		}

		std::cout << "Number of common features between the maps: " << commonFeatures.size() << std::endl;

		double scale = 0.0, scaleDiff = 1.0;
		/*
		Mat pt3D_1, pt3D_2;

		pt3D_1.resize(3, commonFeatures.size());
		pt3D_2.resize(3, commonFeatures.size());

		for (size_t i = 0; i < commonFeatures.size(); ++i) {
		pt3D_1.col(i) = data1.scene.GetLandmarks().at(data1.mapRegionIdx[commonFeatures[i].i_]).X;
		pt3D_2.col(i) = data2.scene.GetLandmarks().at(data2.mapRegionIdx[commonFeatures[i].j_]).X;
		}
		*/
		for (size_t i = 0; i < commonFeatures.size() - 1; ++i) {
			Vec3 X11 = data1.scene.GetLandmarks().at(data1.mapRegionIdx[commonFeatures[i].i_]).X;
			Vec3 X12 = data1.scene.GetLandmarks().at(data1.mapRegionIdx[commonFeatures[i + 1].i_]).X;

			Vec3 X21 = data2.scene.GetLandmarks().at(data2.mapRegionIdx[commonFeatures[i].j_]).X;
			Vec3 X22 = data2.scene.GetLandmarks().at(data2.mapRegionIdx[commonFeatures[i + 1].j_]).X;

			float dist1 = (X12 - X11).norm();
			float dist2 = (X22 - X21).norm();

			scale += dist1 / dist2;

			std::cout << dist1 << std::endl;
			std::cout << dist2 << std::endl;
		}

		scaleDiff = scale / (commonFeatures.size() - 1);
		return scaleDiff;
	}

	bool Utils::rescaleMap(Scene& scene, double scale)
	{
		for (const auto & landmark : scene.GetLandmarks()) {
			scene.structure.at(landmark.first).X = scene.structure.at(landmark.first).X * scale;
		}

		for (unsigned int i = 0; i < scene.poses.size(); ++i) {
			scene.poses[i] = Pose3(scene.poses.at(i).rotation(), scene.poses.at(i).center() * scale);
		}
		return EXIT_SUCCESS;
	}

}
