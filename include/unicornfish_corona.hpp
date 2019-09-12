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

class OBJStaticModel
{
  public:
	struct Vertex
	{
		Vector3f position;
		Vector3f normal;
		Vector2f texcoord;
		Vector3f tangent;
		Vector3f binormal;
	};
	struct Surface
	{
		GLenum mode = GL_TRIANGLES;
		GLint first = 0;
		GLsizei count = 0;
		std::string materialName;
		std::string surfaceName;
	};

  public:
	OBJStaticModel();
	~OBJStaticModel();

	bool LoadOBJ(const std::string &filename);
	bool SaveOBJ(const std::string &filename);
	void SavePrecompiled(const std::string &filename, const std::string objname);
	void LoadPrecompiled(const int numIndices, const int *indices, const int numVertices, const float *vertices, const int numSurfaces, const OBJSurface *surfaces);
	void ComputeTangentVectors();
	void Clear();
	void Resize(int vertexCount, int indexCount, int surfaceCount = 1);
	void CreateSimpleModel(int vertexCount, int indexCount, int surfaceCount = 1);
	void Transform(const Matrix4d &mat);

	void Render(SimpleRenderer_GLuint &renderer);
	void RenderGL11();

	std::vector<Vertex> Vertices;
	std::vector<GLuint> Indices;
	std::vector<Surface> Surfaces;
	BoundingBoxf BoundingBox;
};

class CoronaDatabase
{
  private:
  public:
	SimpleRenderer_GLuint renderer;
	std::map<std::string, OBJStaticModel> models;

	CoronaDatabase();
	~CoronaDatabase();

	void LoadOBJ(const std::string &objname, const std::string &filename);
	void BuildBuffers();
	void Render();
	void RenderZOnly();
};
} // namespace Fluxions

#endif
