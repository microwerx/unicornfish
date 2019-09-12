// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com
#ifndef FLUXIONS_CORONA_SCENE_FILE_HPP
#define FLUXIONS_CORONA_SCENE_FILE_HPP

#include <fluxions_stdcxx.hpp>
#include <fluxions_simple_scene_graph.hpp>

namespace Fluxions
{
	class CoronaSceneFile
	{
	public:
		CoronaSceneFile();
		~CoronaSceneFile();

		/// SetCameraType(newCameraType)
		/// newCameraType = { "perspective" | "cubemap" }
		void SetCameraType(const std::string &newCameraType);
		std::string GetCameraType() const { return cameraType; }

		void SetPerspectiveCamera(const Vector3f &origin, const Vector3f &target, const Vector3f &roll, const float horizontalFieldOfViewInDegrees);
		void SetCubeMapCamera(const Vector3f &origin, const Vector3f &target, const Vector3f &roll);

		void WriteSCN(const std::string &filename, const SimpleSceneGraph &ssg);
		void WriteCubeMapSCN(const std::string &filename, const SimpleSceneGraph &ssg);
		void WriteCubeMapSCN(const std::string &filename, const SimpleSceneGraph &ssg, const Vector3f &cameraPosition);
		void WriteSkySCN(const std::string &filename, const SimpleSceneGraph &ssg);
		bool WriteSphlVizSCN(const std::string &filename, const SimpleSceneGraph &ssg, int sourceLightIndex, int receivingLightIndex);

		void ClearCache();
		void WriteCache(const SimpleSceneGraph &ssg);
		void WriteMaterials(const SimpleSceneGraph &ssg, bool enableKs);

		bool enableKs = false;

	private:
		// cache
		std::vector<std::tuple<std::string, std::string, Matrix4f>> geometryGroups;

		std::string cameraType = "perspective";
		Vector3f cameraOrigin;
		Vector3f cameraTarget;
		Vector3f cameraRoll;
		Matrix4f cameraMatrix;
		float cameraHorizontalFieldOfViewInDegrees;

		void writeCamera(std::ostream &ostr);
		void writeSun(std::ostream &ostr, const SimpleSceneGraph &ssg);
		void writeGeometryGroups(std::ostream &ostr, const SimpleSceneGraph &ssg);
	};

} // namespace Fluxions

#endif
