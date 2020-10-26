#include "unicornfish_pch.hpp"
#include <fluxions_base.hpp>
#include <fluxions_file_system.hpp>
#include <fluxions_ssg_writer.hpp>
#include <fluxions_ssg_ssphh_renderer_plugin.hpp>
#include <unicornfish_corona_scene_file.hpp>
#include <unicornfish_corona_job.hpp>

namespace Uf {
	namespace Fx = Fluxions;

	CoronaSceneFile::CoronaSceneFile() {}
	 
	CoronaSceneFile::~CoronaSceneFile() {}

	void CoronaSceneFile::writeSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		Fluxions::XmlSceneGraphWriter writer;
		writer.setPerspectiveCamera(ssg.camera.origin(),
									ssg.camera.target(),
									ssg.camera.roll(),
									ssg.camera.actualFov);
		Fx::FilePathInfo fpi(filename);
		writer.export_path_prefix = fpi.parentPath();
		writer.extra_tags.push_back({ "conffile", Uf::CoronaJob::confPathPrefix + "export_corona_ground_truth.conf" });
		currentConfig.confname = fpi.stem() + ".conf";
		_copyConf(currentConfig, writer);
		ssg.Save(fpi.filename(), &writer);
		currentConfig.write(fpi.parentPath() + currentConfig.confname, "perspective");
	}

	void CoronaSceneFile::writeCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		Fluxions::Vector3f cameraPosition = -ssg.camera.actualViewMatrix.col4().xyz();
		writeCubeMapSCN(filename, ssg, cameraPosition);
	}

	void CoronaSceneFile::writeCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, const Fluxions::Vector3f& cameraPosition) {
		Fluxions::XmlSceneGraphWriter writer;
		writer.setCubeMapCamera(cameraPosition,
								cameraPosition + Fluxions::Vector3f(0.0f, 0.0f, -1.0f),
								Fluxions::Vector3f(0.0f, 1.0f, 0.0f));
		Fx::FilePathInfo fpi(filename);
		writer.export_path_prefix = fpi.parentPath();
		writer.extra_tags.push_back({ "conffile", Uf::CoronaJob::confPathPrefix + "export_corona_ground_truth_cube.conf" });
		currentConfig.confname = fpi.stem() + ".conf";
		_copyConf(currentConfig, writer);
		ssg.Save(fpi.filename(), &writer);
		currentConfig.write(fpi.parentPath() + currentConfig.confname, "cubemap");
	}

	void CoronaSceneFile::writeSkySCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		Fluxions::XmlSceneGraphWriter writer;
		writer.setCubeMapCamera(Fluxions::Vector3f(0.0f, 0.0f, 0.0f),
								Fluxions::Vector3f(0.0f, 0.0f, 1.0f),
								Fluxions::Vector3f(0.0f, 1.0f, 0.0f));
		Fx::FilePathInfo fpi(filename);
		writer.export_path_prefix = fpi.parentPath();
		writer.extra_tags.push_back({ "conffile", Uf::CoronaJob::confPathPrefix + "ssphh_sky.conf" });
		writer.write_geometry = false;
		SKY.confname = "ssphh_sky.conf";
		_copyConf(SKY, writer);
		ssg.Save(fpi.filename(), &writer);
		SKY.write(fpi.parentPath() + SKY.confname, "cubemap");
	}

	bool CoronaSceneFile::writeSphlGenSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, unsigned lightIndex) {
		if (!ssg.anisoLights.isAHandle(lightIndex + 1))
			return false;
		const Fluxions::SimpleAnisoLight& light = ssg.anisoLights[lightIndex + 1];

		Fluxions::XmlSceneGraphWriter writer;
		Fx::FilePathInfo fpi(filename);
		writer.export_path_prefix = fpi.parentPath();

		Fluxions::Vector3f origin = light.ublock.position.xyz();
		Fluxions::Vector3f target = origin - Fluxions::Vector3f(0.0f, 0.0f, 1.0f);
		Fluxions::Vector3f roll = { 0.0f, 1.0f, 0.0f };
		writer.setCubeMapCamera(origin, target, roll);

		GEN.confname = fpi.stem() + ".conf";
		;
		_copyConf(GEN, writer);
		ssg.Save(fpi.filename(), &writer);
		GEN.write(fpi.parentPath() + GEN.confname, "cubemap");
		return true;
	}

	bool CoronaSceneFile::writeSphlVizSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, int sourceLightIndex, int receivingLightIndex) {
		if (!ssg.anisoLights.isAHandle(sourceLightIndex + 1))
			return false;
		if (!ssg.anisoLights.isAHandle(receivingLightIndex + 1))
			return false;
		const Fluxions::SimpleAnisoLight& src_sphl = ssg.anisoLights[sourceLightIndex + 1];
		const Fluxions::SimpleAnisoLight& rcv_sphl = ssg.anisoLights[receivingLightIndex + 1];

		Fluxions::XmlSceneGraphWriter writer;
		Fx::FilePathInfo fpi(filename);
		writer.export_path_prefix = fpi.parentPath();
		writer.extra_tags.push_back({ "conffile", Uf::CoronaJob::confPathPrefix + "sphlviz.conf" });

		Fluxions::Vector3f src_sphl_position = src_sphl.ublock.position.xyz();
		Fluxions::Vector3f src_sphl_target = src_sphl_position - Fluxions::Vector3f(0.0f, 0.0f, 1.0f);
		Fluxions::Vector3f src_sphl_roll = { 0.0f, 1.0f, 0.0f };
		writer.setCubeMapCamera(src_sphl_position,
								src_sphl_target,
								src_sphl_roll);

		writer.write_sun = false;

		Fluxions::Vector3f rcv_sphl_position = rcv_sphl.ublock.position.xyz();
		Fluxions::Matrix4f lightMatrix = Fluxions::Matrix4f::MakeTranslation(rcv_sphl_position);

		if (!fpi.exists()) {
			HFLOGINFO("Writing out sphlviz.mtl");
			std::ofstream svout(writer.export_path_prefix + "sphlviz.mtl");
			svout << "<mtlLib>"
				"<materialDefinition name = \"sphlSphereLight\">"
				"<material class = \"Native\">"
				"<emission>"
				"<color>100 100 100 </color>"
				"</emission>"
				"</material>"
				"</materialDefinition>"
				"</mtlLib>";
			svout.close();
		}

		std::ostringstream fout;
		Fx::XmlString(fout, "mtllib", "sphlviz.mtl", 1) << "\n";
		Fx::XmlBeginTag(fout, "geometryGroup", 1) << "\n";
		Fx::XmlBeginTag(fout, "object", "sphere", 2) << "\n";
		Fx::XmlInt(fout, "materialId", 0, 3) << "\n";
		Fx::XmlEndTag(fout, "object", 2) << "\n";
		Fx::XmlBeginTag(fout, "instance", 2) << "\n";
		Fx::XmlBeginTag(fout, "material", "Reference", 3) << "sphlSphereLight";
		Fx::XmlEndTag(fout, "material") << "\n";
		Fx::XmlAffineMatrix4f(fout, "transform", lightMatrix, 3) << "\n";
		Fx::XmlEndTag(fout, "instance", 2) << "\n";
		Fx::XmlEndTag(fout, "geometryGroup", 1) << "\n";
		writer.extra_tags.push_back({ "", fout.str() });

		VIZ.confname = fpi.stem() + ".conf";
		_copyConf(VIZ, writer);
		ssg.Save(fpi.filename(), &writer);
		VIZ.write(fpi.parentPath() + VIZ.confname, "cubemap");
		return true;
	}

	void CoronaSceneFile::_copyConf(const CoronaConfig& config, Fluxions::XmlSceneGraphWriter& writer) {
		writer.enableKs = config.enableSpecular;
		writer.clearCache = config.clearCache;
		writer.extra_tags.push_back({ "conffile", config.confname });
	}

	void CoronaConfig::write(const std::string& path, const std::string& cameraType) {
		if (path.empty())
			return;
		std::ofstream conf(path);
		if (resolution.lengthSquared() > 1) {
			int w = resolution.x;
			int h = resolution.y;
			if (cameraType == "cubemap") {
				w = h * 6;
			}
			if (enableHQ) {
				w *= 2;
				h *= 2;
			}
			conf << "       Int image.width  = " << w << "\n";
			conf << "       Int image.height = " << h << "\n";
		}
		conf << "     Float colorMap.simpleExposure = " << simpleExposure << "\n";
		conf << "       Int shading.maxRayDepth     = " << maxRayDepth << "\n";
		conf << "       Int progressive.passLimit   = " << passLimit << "\n";
	}
} // namespace Uf
