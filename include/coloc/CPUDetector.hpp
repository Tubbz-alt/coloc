//
// Created by Sai Vemprala on 7/8/18.
//

#pragma once

#include "openMVG/features/akaze/image_describer_akaze.hpp"
#include "openMVG/features/image_describer_akaze_io.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/features/descriptor.hpp"
#include "openMVG/features/feature.hpp"
#include "nonFree/sift/SIFT_describer_io.hpp"

#include "coloc/colocParams.hpp"
#include "coloc/colocData.hpp"
#include "coloc/AKAZE.hpp"

using namespace openMVG;

namespace coloc
{
	template <typename T>
	class CPUDetector {

	private:
		std::unique_ptr<features::Image_describer> image_describer;

	public:
		CPUDetector(DetectorOptions opts)
		{
			image_describer = features::AKAZE_Image_describer::create(features::AKAZE_Image_describer::Params(features::AKAZE::Params(), features::AKAZE_MLDB), true);
			image_describer->Set_configuration_preset(features::NORMAL_PRESET);
		}

		T detectFeaturesFile(unsigned int idx, FeatureMap &regions, std::string &imageName)
		{
			image::Image<unsigned char> imageGray;
			std::cout << imageName << std::endl;

			if (!ReadImage(imageName.c_str(), &imageGray)) {
				std::cout << "Unable to read image from the given path." << std::endl;
			}

			regions[idx] = describe_AKAZE(imageGray);
			return EXIT_SUCCESS;
		}

#ifdef USE_STREAM
		bool detectFeaturesTopic(unsigned int idx, FeatureMap &regions, cv_bridge::CvImagePtr imagePtr)
		{

		}
#endif
/*
		bool saveFeatureData(uint16_t id, FeatureMap &regions, std::string &name)
		{
			const std::string
				sFeat = stlplus::create_filespec(stlplus::folder_part(name), stlplus::basename_part(name), "feat"),
				sDesc = stlplus::create_filespec(stlplus::folder_part(name), stlplus::basename_part(name), "desc");

			if (!image_describer->Save(regions.at(id).get(), sFeat, sDesc))
				return EXIT_FAILURE;
			else
				return EXIT_SUCCESS;
		}
		*/
	};
}