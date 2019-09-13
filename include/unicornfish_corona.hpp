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
#ifndef FLUXIONS_CORONA_HPP
#define FLUXIONS_CORONA_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fluxions_gte.hpp>
#include <fluxions_simple_renderer.hpp>
#include <fluxions_bbox.hpp>
#include <fluxions_precompiled_models.hpp>

namespace Fluxions
{
	class CoronaDatabase
	{
	private:
	public:
		SimpleRenderer_GLuint renderer;
		std::map<std::string, SimpleStaticMesh> models;

		CoronaDatabase();
		~CoronaDatabase();

		void LoadOBJ(const std::string& objname, const std::string& filename);
		void BuildBuffers();
		void Render();
		void RenderZOnly();
	};
} // namespace Fluxions

#endif
