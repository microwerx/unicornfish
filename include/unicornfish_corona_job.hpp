#ifndef UNICORNFISH_CORONA_JOB_HPP
#define UNICORNFISH_CORONA_JOB_HPP

#include <unicornfish_base.hpp>
#include <fluxions_gte.hpp>
#include <fluxions_gte_spherical_harmonic.hpp>
#include <fluxions_ssg_scene_graph.hpp>
#include <unicornfish_corona_scene_file.hpp>

namespace Uf {
	class CoronaJob {
	public:
		static const std::string exportPathPrefix;
		static const std::string outputPathPrefix;
		static const std::string confPathPrefix;

		enum class Type {
			REF,
			REF_CubeMap,
			Sky,
			VIZ,
			GEN
		};

		static Type TypeFromString(const std::string& str) noexcept;
		static std::string& TypeToString(Type type, std::string& s_type) noexcept;

		CoronaJob() {}
		CoronaJob(const std::string& basename, const std::string& name, Type jobtype, int arg1 = 0, int arg2 = 0);
		~CoronaJob();

		void EnableHQ() { isHQ = true; }
		void DisableHQ() { isHQ = false; }
		constexpr bool IsHQ() const { return isHQ; }

		constexpr void SetMaxRayDepth(int depth) { maxRayDepth = Fluxions::clamp(depth, 1, 50); }
		constexpr void SetPassLimit(int limit) { passLimit = Fluxions::clamp(limit, 1, 100); }

		void usePreviousRun(bool bState) { usePreviousRun_ = bState; }
		const bool usePreviousRun() const { return usePreviousRun_; }

		void EnableHDR() { isHDR = true; }
		void DisableHDR() { isHDR = false; }
		constexpr bool IsHDR() const { return isHDR; }

		constexpr bool IsReady() const { return state == State::Ready; }
		constexpr bool IsRunning() const { return state == State::Running; }
		constexpr bool IsFinished() const { return state == State::Finished; }

		constexpr bool IsREF() const { return type == Type::REF; }
		constexpr bool IsREF_CubeMap() const { return type == Type::REF_CubeMap; }
		constexpr bool IsSky() const { return type == Type::Sky; }
		constexpr bool IsGEN() const { return type == Type::GEN; }
		constexpr bool IsVIZ() const { return type == Type::VIZ; }

		constexpr void SetImageDimensions(int w, int h) {
			imageWidth = Fluxions::clamp(w, 0, 8192);
			imageHeight = Fluxions::clamp(h, 0, 8192);
		}

		constexpr double GetElapsedTime() const { return elapsedTime; }

		constexpr const std::string& GetOutputPath(bool exrPathInstead = false) const {
			if (exrPathInstead) {
				if (isHQ)
					return hq_output_path_exr;
				else
					return output_path_exr;
			}
			else {
				if (isHQ)
					return hq_output_path_ppm;
				else
					return output_path_ppm;
			}
		}
		constexpr const std::string& GetName() const { return scene_name; }

		constexpr int GetGENLightIndex() const {
			if (IsGEN())
				return recvLight;
			return -1;
		}
		constexpr int GetVIZSendLightIndex() const {
			if (IsVIZ())
				return sendLight;
			return -1;
		}
		constexpr int GetVIZRecvLightIndex() const {
			if (IsVIZ())
				return recvLight;
			return -1;
		}

		void Start(CoronaSceneFile& coronaScene, Fluxions::SimpleSceneGraph& ssg);
		void CopySPH(const Fluxions::Sph4f& sph);
		void CopySPHToSph4f(Fluxions::Sph4f& sph);
		const int GetCoronaRetval() const { return lastCoronaRetval; }
		const int GetConvertRetval() const { return lastConvertRetval; }

		std::string MakeCoronaCommandLine();
		std::string MakeConvertCommandLine();

		constexpr bool IsJobFinished() const { return finished; }
		constexpr void MarkJobFinished() { finished = true; }
		constexpr void MarkJobUnfinished() {
			finished = false;
			working = false;
		}
		constexpr bool IsJobWorking() const { return working; }
		constexpr void MarkJobWorking() { working = true; }

		std::string ToString() const;
		void FromString(const std::string& str);

	private:
		enum class State {
			Error = -1,
			Ready = 0,
			Running,
			Finished,
		};
		State state = State::Ready;
		Type type = Type::REF;
		std::string export_path;
		std::string scene_name;
		std::string scene_path;
		std::string output_path_exr;
		std::string output_path_ppm; // the ppm is created using ImageMagick's convert command
		std::string output_path_png;
		std::string hq_output_path_exr;
		std::string hq_output_path_ppm; // the ppm is created using ImageMagick's convert command
		std::string conf_path;
		std::string hq_conf_path;

		float sph[121 * 4] = { 0 };

		int imageWidth = 1280;
		int imageHeight = 720;
		int maxRayDepth = 5;
		int passLimit = 1;
		bool usePreviousRun_{ true };

		double elapsedTime;
		bool finished = false;
		bool working = false;

		bool isHQ = false;
		bool isHDR = false;

		// If GEN, then recvLight is the
		int sendLight = -1;
		int recvLight = -1;

		int lastCoronaRetval = 0;
		int lastConvertRetval = 0;

		bool Run();
	private:
		bool _runCorona();
		bool _runMagickEXRtoPNG();
		bool _runMagickEXRtoPPM();
	};
} // namespace Fluxions


#endif // FLUXIONS_CORONA_JOB_HPP