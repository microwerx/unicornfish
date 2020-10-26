#ifndef FLUXIONS_CORONA_SCENE_FILE_HPP
#define FLUXIONS_CORONA_SCENE_FILE_HPP

#include <unicornfish_base.hpp>
#include <fluxions_ssg_writer.hpp>

namespace Uf {
	struct CoronaConfig {
		int maxRayDepth{ 5 };
		int passLimit{ 1 };
		bool clearCache{ false };
		bool enableHQ{ false };
		bool enableHDR{ false };
		bool enableSpecular{ false };
		bool usePreviousRun{ false };
		float simpleExposure{ 0.0f };
		Fluxions::Vector2i resolution{ 0, 0 };
		std::string confname;

		void write(const std::string& path, const std::string& cameraType);
	};

	static constexpr int DefaultHighQualityMaxRayDepth = 25;
	static constexpr int DefaultMedQualityMaxRayDepth = 5;
	static constexpr int DefaultLowQualityMaxRayDepth = 2;

	static constexpr int DefaultHighQualityPassLimit = 25;
	static constexpr int DefaultMedQualityPassLimit = 5;
	static constexpr int DefaultLowQualityPassLimit = 1;

	class CoronaSceneFile {
	public:
		CoronaSceneFile();
		~CoronaSceneFile();

		void stashConfig() { stash_ = currentConfig; }
		void restoreConfig() { currentConfig = stash_; }

		void writeSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		void writeCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		void writeCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, const Fluxions::Vector3f& cameraPosition);
		void writeSkySCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		bool writeSphlGenSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, unsigned lightIndex);
		bool writeSphlVizSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, int sourceLightIndex, int receivingLightIndex);

		CoronaConfig currentConfig{ DefaultMedQualityMaxRayDepth, DefaultLowQualityPassLimit, false, false, true, false, false };
		CoronaConfig REF{ DefaultHighQualityMaxRayDepth, DefaultHighQualityPassLimit, true, true, true, true, false };
		CoronaConfig VIZ{ DefaultMedQualityMaxRayDepth, DefaultLowQualityPassLimit, false, false, true, false, true };
		CoronaConfig GEN{ DefaultMedQualityMaxRayDepth, DefaultLowQualityPassLimit, false, false, true, false, false };
		CoronaConfig SKY{ DefaultMedQualityMaxRayDepth, DefaultLowQualityPassLimit, false, false, true, false, false };

		int lastFrame{ 0 };
		std::string lastSCN;
		std::string lastCubeSCN;
		std::string lastSkySCN;
		std::string lastSphlVizSCN;
	private:
		CoronaConfig stash_;
		void _copyConf(const CoronaConfig& config, Fluxions::XmlSceneGraphWriter& writer);
	};
} // namespace Uf

#endif
