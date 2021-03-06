#pragma once

#pragma once

#include "openMVG/matching/svg_matches.hpp"

#include "colocParams.hpp"
#include "colocData.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <dlib/optimization.h>
#include <algorithm>
#include <functional>

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
		double computeScaleDifference(Scene &scene1, std::vector <IndexT> &idx1, Scene &scene2, std::vector <IndexT> &idx2, std::vector<IndMatch> commonFeatures);
		bool matchSceneWithMap(Scene& scene);
		bool rescaleMap(Scene& scene, double scale);
		int drawFeaturePoints(std::string& imageName, features::PointFeatures points);
		bool drawFeatures(std::string& image, std::pair <int, int>& imageSize, features::SIOPointFeatures& features, std::string& outputFilename, const double feature_circle_radius,
			const double stroke_size);
		bool drawMatches(std::pair <int, int>& imageSize, std::string& outputFilename, std::string& image1, std::string& image2, Regions& regions1, Regions& regions2, std::vector <IndMatch>& matches);

		void loadPoseCovariance(Cov6& cov, matrix<double, 3, 3>& covNew)
		{
			covNew = cov[21], cov[22], cov[23],
				cov[27], cov[28], cov[29],
				cov[33], cov[34], cov[35];
		}

		void readPoseCovariance(matrix<double, 3, 3>& covNew, Cov6& cov)
		{
			cov[21] = covNew(0, 0);
			cov[22] = covNew(0, 1);
			cov[23] = covNew(0, 2);
			cov[27] = covNew(1, 0);
			cov[28] = covNew(1, 1);
			cov[29] = covNew(1, 2);
			cov[33] = covNew(2, 0);
			cov[34] = covNew(2, 1);
			cov[35] = covNew(2, 2);
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

	bool Utils::drawMatches(std::pair <int, int>& imageSize, std::string& outputFilename, std::string& image1, std::string& image2, Regions& regions1, Regions& regions2, std::vector <IndMatch>& matches)
	{
		Matches2SVG(
			image1, { imageSize.first, imageSize.second }, regions1.GetRegionsPositions(),
			image2, { imageSize.first, imageSize.second }, regions2.GetRegionsPositions(),
			matches, outputFilename, true);
		return EXIT_SUCCESS;
	}

	bool Utils::drawFeatures(std::string& image, std::pair <int, int>& imageSize, features::SIOPointFeatures& features, std::string& outputFilename, const double feature_circle_radius = 3.0,
		const double stroke_size = 3.0)
	{
		svg::svgDrawer svgStream(imageSize.first, imageSize.second);

		// Draw image
		svgStream.drawImage(image, imageSize.first, imageSize.second);

		// Draw features
		for (const features::PointFeature & feat_it : features) {
			// Draw the feature (circle)
			svgStream.drawCircle(
				feat_it.x(), feat_it.y(), feature_circle_radius,
				svg::svgStyle().stroke("green", stroke_size));
		}

		// Save the SVG file
		std::ofstream svgFile(outputFilename.c_str());
		if (svgFile.is_open())
		{
			svgFile << svgStream.closeSvgFile().str();
			svgFile.close();
			return true;
		}
		return false;
	}

	double Utils::computeScaleDifference(Scene &scene1, std::vector <IndexT> &idx1, Scene &scene2, std::vector <IndexT> &idx2, std::vector<IndMatch> commonFeatures)
	{
		if (commonFeatures.empty()) {
			std::cout << "No common features between the maps." << std::endl;
			return 1.0;
		}

		std::cout << "Number of common features between the maps: " << commonFeatures.size() << std::endl;

		double scale = 0.0, scaleDiff = 1.0;
		for (size_t i = 0; i < commonFeatures.size() - 1; ++i) {
			Vec3 X11 = scene1.GetLandmarks().at(idx1[commonFeatures[i].i_]).X;
			Vec3 X12 = scene1.GetLandmarks().at(idx1[commonFeatures[i + 1].i_]).X;

			Vec3 X21 = scene2.GetLandmarks().at(idx2[commonFeatures[i].j_]).X;
			Vec3 X22 = scene2.GetLandmarks().at(idx2[commonFeatures[i + 1].j_]).X;

			float dist1 = (X12 - X11).norm();
			float dist2 = (X22 - X21).norm();

			scale += dist1 / dist2;

			//std::cout << "Distance ratio: " << dist1/dist2 << std::endl;
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

