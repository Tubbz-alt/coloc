#pragma once

#include <string>
#include <fstream>
#include "localizationData.hpp"
#include "localizationParams.hpp"

namespace coloc
{
	class Logger
	{
	public:
		bool logMaptoPLY(Scene& scene, std::string& filename);
		bool logPosetoPLY(Pose3& pose, std::string& filename);
		bool logPoseCovtoFile(int idx, int source, int dest, Pose3& pose, Cov6& cov, std::string& filename);

	private:
		void convertAnglesForLogging(Vec3& angles);
	};

	void Logger::convertAnglesForLogging(Vec3& angles)
	{
		float a1, a2, a3;

		a1 = angles[0] * 180 / M_PI;
		a2 = angles[2] * 180 / M_PI;
		a3 = angles[1] * 180 / M_PI;
		if (abs(a2) > 120) {
			if (a2 < 0)
				a2 = (-1 * a2 - 180);
			else
				a2 = 180 - a2;
		}
		
		if (abs(a3) > 120) {
			if (a3 < 0)
				a3 = 180 + a3;
			else
				a3 = a3 - 180;
		}
		else
			a3 = -1 * a3;

		angles[0] = a1 * M_PI / 180;
		angles[1] = a2 * M_PI / 180;
		angles[2] = a3 * M_PI / 180;
	}

	bool Logger::logPoseCovtoFile(int idx, int source, int dest, Pose3& pose, Cov6& cov, std::string& filename)
	{
		std::ofstream file;

		Vec3 position = pose.center();
		file.open(filename, std::ios::out | std::ios::app);
		if (file.fail())
			throw std::ios_base::failure(std::strerror(errno));

		//make sure write fails with exception if something is wrong
		file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

		Vec3 eulerAngles = pose.rotation().eulerAngles(2, 1, 0);

		Vec3 eulerAngles_old = eulerAngles;

		convertAnglesForLogging(eulerAngles);
		float roll = eulerAngles[0] * 180 / M_PI;
		float pitch = eulerAngles[1] * 180 / M_PI;
		float yaw = eulerAngles[2] * 180 / M_PI;

		// float xPos = position[0] - 0.35 * yaw;
		// float yPos = position[1] + 0.35 * pitch;
		// float zPos = position[2];

		file << idx << "," << dest << "," << source << "," 
			 << position[0] << "," << position[1] << "," << position[2] << "," 
		//	 << xPos << "," << yPos << "," << zPos << ","
			 << cov.at(0)[21] << "," << cov.at(0)[22] << "," << cov.at(0)[23] << ","  
			 << cov.at(0)[27] << "," << cov.at(0)[28] << "," << cov.at(0)[29] << ","  
		     << cov.at(0)[33] << "," << cov.at(0)[34] << "," << cov.at(0)[35] << "," 
			 <<	roll << "," << pitch << "," << yaw << std::endl;

		bool logStatus = file.good();
		return logStatus;
	}

	bool Logger::logPosetoPLY(Pose3& pose, std::string& filename)
	{
		std::ofstream stream(filename.c_str(), std::ios::out | std::ios::app);
		if (!stream.is_open())
			return false;

		stream << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 + 1);

		using Vec3uc = Eigen::Matrix<unsigned char, 3, 1>;
		stream
			<< pose.center()(0) << ' '
			<< pose.center()(1) << ' '
			<< pose.center()(2) << ' '
			<< "0 255 0\n";

		bool logStatus = stream.good();
		return logStatus;
	}

	bool Logger::logMaptoPLY(Scene& scene, std::string& filename)
	{
		std::ofstream stream(filename.c_str(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return false;

		stream << std::fixed << std::setprecision(std::numeric_limits<double>::digits10 + 1);

		using Vec3uc = Eigen::Matrix<unsigned char, 3, 1>;

		stream << "ply"	<< '\n' << "format " << "ascii 1.0"
			<< '\n' << "comment generated by coloc"
			<< '\n' << "element vertex "
			<< scene.GetLandmarks().size()
				+ scene.GetPoses().size()
			<< '\n' << "property double x"
			<< '\n' << "property double y"
			<< '\n' << "property double z"
			<< '\n' << "property uchar red"
			<< '\n' << "property uchar green"
			<< '\n' << "property uchar blue"
			<< '\n' << "end_header" << std::endl;

		for (const auto & view : scene.GetViews()) {
			if (scene.IsPoseAndIntrinsicDefined(view.second.get())) {
				const geometry::Pose3 pose = scene.GetPoseOrDie(view.second.get());
				stream
					<< pose.center()(0) << ' '
					<< pose.center()(1) << ' '
					<< pose.center()(2) << ' '
					<< "0 255 0\n";
			}
		}

		const Landmarks & landmarks = scene.GetLandmarks();
		for (const auto & iterLandmarks : landmarks) {
			stream	<< iterLandmarks.second.X(0) << ' '
					<< iterLandmarks.second.X(1) << ' '
					<< iterLandmarks.second.X(2) << ' '
					<< "255 255 255\n";
		}

		stream.flush();
		bool logStatus = stream.good();
		stream.close();

		return logStatus;
	}
}