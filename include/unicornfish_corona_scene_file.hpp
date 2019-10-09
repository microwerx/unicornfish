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

namespace Uf
{
	class CoronaSceneFile
	{
	public:
		CoronaSceneFile();
		~CoronaSceneFile();

		/// SetCameraType(newCameraType)
		/// newCameraType = { "perspective" | "cubemap" }
		void SetCameraType(const std::string& newCameraType);
		std::string GetCameraType() const { return cameraType; }

		void SetPerspectiveCamera(const Fluxions::Vector3f& origin, const Fluxions::Vector3f& target, const Fluxions::Vector3f& roll, const float horizontalFieldOfViewInDegrees);
		void SetCubeMapCamera(const Fluxions::Vector3f& origin, const Fluxions::Vector3f& target, const Fluxions::Vector3f& roll);

		void WriteSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		void WriteCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		void WriteCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, const Fluxions::Vector3f& cameraPosition);
		void WriteSkySCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg);
		bool WriteSphlVizSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, int sourceLightIndex, int receivingLightIndex);

		void ClearCache();
		void WriteCache(const Fluxions::SimpleSceneGraph& ssg);
		void WriteMaterials(const Fluxions::SimpleSceneGraph& ssg, bool enableKs);

		bool enableKs = false;

	private:
		// cache
		std::vector<std::tuple<std::string, std::string, Fluxions::Matrix4f>> geometryGroups;

		std::string cameraType = "perspective";
		Fluxions::Vector3f cameraOrigin;
		Fluxions::Vector3f cameraTarget;
		Fluxions::Vector3f cameraRoll;
		Fluxions::Matrix4f cameraMatrix;
		float cameraHorizontalFieldOfViewInDegrees = 45.0f;

		void writeCamera(std::ostream& ostr);
		void writeSun(std::ostream& ostr, const Fluxions::SimpleSceneGraph& ssg);
		void writeGeometryGroups(std::ostream& ostr, const Fluxions::SimpleSceneGraph& ssg);
	};

} // namespace Uf

#endif
