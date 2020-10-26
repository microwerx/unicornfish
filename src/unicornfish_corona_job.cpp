#include "unicornfish_pch.hpp"
#include <fluxions_base.hpp>
#include <fluxions_file_system.hpp>
#include <fluxions_gte_spherical_harmonic.hpp>
#include <unicornfish_corona_job.hpp>
#include <unicornfish_corona_scene_file.hpp>

namespace Uf {
	//static const std::string corona_conf_prefix("../corona_conf/");
	//static const std::string corona_export_prefix("corona_export/");
	//static const std::string corona_output_prefix("corona_output/");

	const std::string CoronaJob::exportPathPrefix = "corona_export/";
	const std::string CoronaJob::outputPathPrefix = "corona_output/";
	const std::string CoronaJob::confPathPrefix = "../corona_conf/";

	/////////////////////////////////////////////////////////////////////
	// CoronaJob ////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////

	CoronaJob::CoronaJob(const std::string& basename, const std::string& name, Type jobtype, int arg1, int arg2) {
		export_path = exportPathPrefix;

		scene_name = basename;
		scene_path = export_path + name + ".scn";
		output_path_exr = outputPathPrefix + name + ".exr";
		output_path_ppm = outputPathPrefix + name + ".ppm";
		output_path_png = outputPathPrefix + name + ".png";
		conf_path = export_path + name + ".conf";
		hq_output_path_exr = outputPathPrefix + name + "_hq.exr";
		hq_output_path_ppm = outputPathPrefix + name + "_hq.ppm";
		hq_conf_path = export_path + name + "_hq.conf";

		type = jobtype;

		switch (jobtype) {
		case Type::GEN:
			sendLight = Fluxions::SphlSunIndex;
			recvLight = arg1;
			conf_path = export_path + "sphlgen.conf";
			hq_conf_path = export_path + "sphlgen_hq.conf";
			break;
		case Type::VIZ:
			sendLight = arg1;
			recvLight = arg2;
			conf_path = export_path + "sphlviz.conf";
			hq_conf_path = export_path + "sphlviz_hq.conf";
			break;
		case Type::REF:
			conf_path = export_path + "ground_truth.conf";
			hq_conf_path = export_path + "ground_truth_hq.conf";
			break;
		case Type::REF_CubeMap:
			break;
		case Type::Sky:
			break;
		}
	}

	CoronaJob::~CoronaJob() {}

	void CoronaJob::Start(CoronaSceneFile& coronaScene, Fluxions::SimpleSceneGraph& ssg) {
		if (state != State::Ready) {
			return;
		}

		if (usePreviousRun_) {
			Fluxions::FilePathInfo fpi(isHQ ? hq_output_path_exr : output_path_exr);
			if (fpi.exists()) {
				state = State::Finished;
				return;
			}
		}

		//std::string tonemapconf = exportPathPrefix + scene_name + "_tonemap.conf";
		//if (1)
		//{
		//	float tonemap = ssg.environment.toneMapExposure();
		//	if (type == Type::VIZ)
		//	{
		//		tonemap = 0.0f;
		//	}
		//	if (type == Type::GEN)
		//	{
		//		tonemap = 0.0f;
		//	}
		//	HFLOGINFO("Writing tonemap conf %s", tonemapconf.c_str());

		//	std::ofstream fout(tonemapconf);
		//	fout << "Float colorMap.simpleExposure = " << tonemap << "\n";
		//	if (type == Type::REF)
		//	{
		//		fout << "Int image.width = " << imageWidth << "\n";
		//		fout << "Int image.height = " << imageHeight << "\n";
		//	}
		//	else
		//	{
		//		fout << "Int image.width = " << 6 * 128 << "\n";
		//		fout << "Int image.height = " << 128 << "\n";
		//	}
		//	if (!isHQ)
		//	{
		//		fout << "Int shading.maxRayDepth = " << maxRayDepth << "\n";
		//		fout << "Int progressive.passLimit = " << passLimit << "\n";
		//	}
		//	fout.close();
		//}

		Hf::StopWatch stopwatch;
		state = State::Running;
		bool result = true;
		coronaScene.stashConfig();
		switch (type) {
		case Type::REF:
			coronaScene.REF.resolution.reset(imageWidth, imageHeight);
			coronaScene.currentConfig = coronaScene.REF;
			coronaScene.writeSCN(scene_path, ssg);
			result = Run();
			break;
		case Type::REF_CubeMap:
			coronaScene.REF.resolution.reset(imageWidth, imageHeight);
			coronaScene.currentConfig = coronaScene.REF;
			coronaScene.writeCubeMapSCN(scene_path, ssg);
			result = Run();
			break;
		case Type::Sky:
			coronaScene.SKY.resolution.reset(imageWidth, imageHeight);
			coronaScene.currentConfig = coronaScene.SKY;
			coronaScene.writeSkySCN(scene_path, ssg);
			result = Run();
			break;
		case Type::GEN:
			coronaScene.GEN.resolution.reset(imageWidth, imageHeight);
			coronaScene.currentConfig = coronaScene.GEN;
			coronaScene.writeSphlGenSCN(scene_path, ssg, recvLight);
			result = Run();
			break;
		case Type::VIZ:
			coronaScene.VIZ.resolution.reset(imageWidth, imageHeight);
			coronaScene.currentConfig = coronaScene.VIZ;
			coronaScene.writeSphlVizSCN(scene_path, ssg, sendLight, recvLight);
			result = Run();
			break;
		default:
			break;
		}
		coronaScene.restoreConfig();
		elapsedTime = stopwatch.GetSecondsElapsed();
		state = result ? State::Finished : State::Error;

		// std::filesystem::remove(tonemapconf);
		// std::filesystem::remove(scene_path);
	}

	void CoronaJob::CopySPH(const Fluxions::Sph4f& sph_) {
		if (!IsFinished() && !IsGEN() && !IsVIZ())
			return;
		memset(sph, 0, sizeof(float) * 484);
		for (unsigned i = 0; i < sph_.size(); i++) {
			sph[121 * 0 + i] = sph_.r().getCoefficient(i);
			sph[121 * 1 + i] = sph_.g().getCoefficient(i);
			sph[121 * 2 + i] = sph_.g().getCoefficient(i);
			sph[121 * 3 + i] = sph_.a().getCoefficient(i);
		}
	}

	void CoronaJob::CopySPHToSph4f(Fluxions::Sph4f& sph_) {
		sph_.resize(Fluxions::MaxSphlDegree);
		for (unsigned i = 0; i < sph_.size(); i++) {
			sph_.r().setCoefficient(i, sph[121 * 0 + i]);
			sph_.g().setCoefficient(i, sph[121 * 1 + i]);
			sph_.b().setCoefficient(i, sph[121 * 2 + i]);
			sph_.a().setCoefficient(i, sph[121 * 3 + i]);
		}
	}

	std::string CoronaJob::MakeCoronaCommandLine() {
		const std::string path1 = "C:\\Program Files\\Corona\\Standalone\\Corona.exe";
		const std::string path2 = "C:\\Program Files\\Corona\\Corona Renderer for 3ds Max\\Standalone\\Corona.exe";
		std::string path;
		if (std::filesystem::exists(path1)) path = path1;
		else if (std::filesystem::exists(path2)) path = path2;
		else HFLOGERROR("Corona.exe not found");

		std::ostringstream cmd;
		cmd << "\"" << path << "\" " << scene_path << " -silent";

		if (isHDR) {
			cmd << " -oR " << (isHQ ? hq_output_path_exr : output_path_exr);
		}
		else {
			cmd << " -o " << (isHQ ? hq_output_path_exr : output_path_exr);
		}

		//if (isHQ)
		//{
		//	cmd << " -c " << hq_conf_path;
		//}
		//else
		//{
		//	cmd << " -c " << conf_path;
		//}

		//cmd << " -c " << exportPathPrefix + scene_name << "_tonemap.conf";

		HFLOGINFO("running %s", cmd.str().c_str());

		return cmd.str();
	}

	std::string CoronaJob::MakeConvertCommandLine() {
		std::ostringstream cmd;
		if (isHQ) {
			cmd << "magick " << hq_output_path_exr << " -compress none " << hq_output_path_ppm;
		}
		else {
			cmd << "magick " << output_path_exr << " -compress none " << output_path_ppm;
		}
		return cmd.str();
	}

	bool CoronaJob::Run() {
		bool result = true;
		if (!_runCorona())
			result = false;
		if (!_runMagickEXRtoPNG())
			result = false;
		if (!_runMagickEXRtoPPM())
			result = false;
		return result;
	}

	bool CoronaJob::_runCorona() {
		std::string commandLine = MakeCoronaCommandLine();
		const char* pcmd = commandLine.c_str();
		HFLOGINFO("running %s", pcmd);
		lastCoronaRetval = system(pcmd);
		if (lastCoronaRetval != 0) {
			HFLOGERROR("unable to run Corona");
			return false;
		}
		return true;
	}

	bool CoronaJob::_runMagickEXRtoPNG() {
#ifdef USE_MAGICK_TO_CONVERT
		std::ostringstream cmd;
		cmd << "magick " << output_path_exr << " " << output_path_png;
		const char* pcmd = cmd.str().c_str();
		HFLOGINFO("running %s", pcmd);
		lastConvertRetval = system(pcmd);
		if (lastConvertRetval != 0) {
			HFLOGERROR("unable to convert EXR to PNG");
			return false;
		}
#endif
		return true;
	}

	bool CoronaJob::_runMagickEXRtoPPM() {
#ifdef USE_MAGICK_TO_CONVERT
		std::ostringstream cmd;
		cmd << "magick " << output_path_png << " -compress none " << output_path_ppm;
		const char* pcmd = cmd.str().c_str();
		HFLOGINFO("running %s", pcmd);
		lastConvertRetval = system(pcmd);
		if (lastConvertRetval != 0) {
			HFLOGERROR("unable to convert PNG to PPM");
			return false;
		}
#endif
		return true;
	}

	std::string CoronaJob::ToString() const {
		std::ostringstream ostr;
		ostr << imageWidth << "\n";
		ostr << imageHeight << "\n";
		ostr << usePreviousRun_ << "\n";
		ostr << maxRayDepth << "\n";
		ostr << passLimit << "\n";
		ostr << elapsedTime << "\n";
		ostr << finished << "\n";
		ostr << working << "\n";
		ostr << isHQ << "\n";
		ostr << isHDR << "\n";
		ostr << sendLight << "\n";
		ostr << recvLight << "\n";
		std::string s_type;
		ostr << CoronaJob::TypeToString(type, s_type) << "\n";
		ostr << scene_name << "\n";
		ostr << output_path_exr << "\n";
		ostr << output_path_ppm << "\n";

		return ostr.str();
	}

	void CoronaJob::FromString(const std::string& str) {
		std::istringstream istr(str);
		istr >> imageWidth;
		istr >> imageHeight;
		istr >> usePreviousRun_;
		istr >> maxRayDepth;
		istr >> passLimit;
		istr >> elapsedTime;
		istr >> finished;
		istr >> working;
		istr >> isHQ;
		istr >> sendLight;
		istr >> recvLight;
		std::string s_type;
		istr >> s_type;
		type = CoronaJob::TypeFromString(s_type);
		istr >> scene_name;
		istr >> output_path_exr;
		istr >> output_path_ppm;
	}

	std::string& CoronaJob::TypeToString(CoronaJob::Type type, std::string& s_type) noexcept {
		if (type == CoronaJob::Type::VIZ)
			s_type = "VIZ";
		else if (type == CoronaJob::Type::GEN)
			s_type = "GEN";
		else if (type == CoronaJob::Type::REF)
			s_type = "REF";
		else if (type == CoronaJob::Type::REF_CubeMap)
			s_type = "REF_CubeMap";
		else if (type == CoronaJob::Type::Sky)
			s_type = "Sky";
		return s_type;
	}

	CoronaJob::Type CoronaJob::TypeFromString(const std::string& s_type) noexcept {
		Type type = Type::GEN;
		if (s_type == "VIZ")
			type = Type::VIZ;
		else if (s_type == "GEN")
			type = Type::GEN;
		else if (s_type == "REF")
			type = Type::REF;
		else if (s_type == "REF_CubeMap")
			type = Type::REF_CubeMap;
		else if (s_type == "Sky")
			type = Type::Sky;
		return type;
	}
} // namespace Uf
