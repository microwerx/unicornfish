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
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fluxions_obj_static_model.hpp>
#include <unicornfish_corona_database.hpp>

template class std::vector<Fluxions::OBJStaticModel::Vertex>;
template class std::vector<GLuint>;
template class std::vector<Fluxions::OBJStaticModel::Surface>;
template class std::map<std::string, Fluxions::OBJStaticModel>;

namespace Uf
{

	CoronaDatabase::CoronaDatabase() {}

	CoronaDatabase::~CoronaDatabase() {}

	void CoronaDatabase::LoadOBJ(const std::string& objname, const std::string& filename) {
		models[objname].LoadOBJ(filename);
	}

	void CoronaDatabase::BuildBuffers() {
		renderer.Reset();
		std::map<std::string, Fluxions::OBJStaticModel>::iterator modelIt;
		for (modelIt = models.begin(); modelIt != models.end(); modelIt++) {
			modelIt->second.Render(renderer);
		}
		renderer.BuildBuffers();
	}

	void CoronaDatabase::Render() {
		renderer.Render();
	}

	void CoronaDatabase::RenderZOnly() {
		renderer.RenderZOnly();
	}

} // namespace Uf
