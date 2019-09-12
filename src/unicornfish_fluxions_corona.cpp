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
#include "pch.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fluxions_corona.hpp>
#include <fluxions.hpp>

template class std::vector<Fluxions::OBJStaticModel::Vertex>;
template class std::vector<GLuint>;
template class std::vector<Fluxions::OBJStaticModel::Surface>;
template class std::map<std::string, Fluxions::OBJStaticModel>;

namespace Fluxions
{

CoronaDatabase::CoronaDatabase()
{
}

CoronaDatabase::~CoronaDatabase()
{
}

void CoronaDatabase::LoadOBJ(const std::string &objname, const std::string &filename)
{
    models[objname].LoadOBJ(filename);
}

void CoronaDatabase::BuildBuffers()
{
    renderer.Reset();
    std::map<std::string, OBJStaticModel>::iterator modelIt;
    for (modelIt = models.begin(); modelIt != models.end(); modelIt++)
    {
        modelIt->second.Render(renderer);
    }
    renderer.BuildBuffers();
}

void CoronaDatabase::Render()
{
    renderer.Render();
}

void CoronaDatabase::RenderZOnly()
{
    renderer.RenderZOnly();
}

OBJStaticModel::OBJStaticModel()
{
}

OBJStaticModel::~OBJStaticModel()
{
}

bool OBJStaticModel::LoadOBJ(const std::string &filename)
{
    bool cacheDoesNotExist = true;
    std::string cache_filename = filename + ".cache";

    FilePathInfo fpi_orig(filename);
    FilePathInfo fpi(cache_filename);

    if (fpi.DoesNotExist())
    {
        cacheDoesNotExist = true;
    }
    else
    {
        // Is the original file newer than the cache?
        if (fpi_orig.ctime > fpi.ctime)
        {
            cacheDoesNotExist = true;
        }
        else
        {
            cacheDoesNotExist = false;
            Hf::Log.info("%s(): loading cached OBJ %s", __FUNCTION__, fpi.fname.c_str());
        }
    }

    if (cacheDoesNotExist)
    {
        Hf::Log.info("%s(): loading OBJ %s", __FUNCTION__, fpi_orig.fname.c_str());
        int curSurface = 0;
        std::string surfaceName;
        std::string objectName;
        std::string materialLibrary;
        float v[3];
        int iv[3];
        std::vector<Vector3f> vList;
        std::vector<Vector3f> vnList;
        std::vector<Vector2f> vtList;
        std::vector<TVector3<int>> faceList;
        std::map<long long, int> indexMap; // this maps the unique combo of of vertices
        std::map<int, Vertex> vertexMap;
        long lastIndex = 0;
        int newIndex = 0;
        int first = 0;
        // int count = 0;
        int firstVertex = 0;
        const double size = 1.0;

        bool optimizeIndexing = false;

        BoundingBox.Reset();

        std::ifstream fin(filename.c_str());
        std::string str;
        std::istringstream istr;

        if (!fin)
            return false;

        CreateSimpleModel(0, 0, 1);
        int linecount = 0;
        int totallinecount = 0;
        while (1)
        {
            getline(fin, str);
            linecount++;
            totallinecount++;
            if (!fin)
            {
                // flush remaining polygons to last surface (if any)
                if (Surfaces.size() != 0)
                {
                    Surfaces[curSurface].count = (int)faceList.size() * 3;

                    Hf::Log.info("%s(): %s ... adding %d new faces starting at %d to %s", __FUNCTION__, fpi.fname.c_str(), faceList.size(), first, Surfaces[curSurface].surfaceName.c_str());

                    // 2. add indices (triangles)
                    for (auto it = faceList.begin(); it != faceList.end(); it++)
                    {
                        Indices.push_back(it->x + firstVertex);
                        Indices.push_back(it->y + firstVertex);
                        Indices.push_back(it->z + firstVertex);
                    }
                    first = (int)Indices.size();
                    faceList.clear();
                }
                break;
            }
            istr.clear();
            istr.str(str);

            istr >> str;
            if (str == "o")
            {
                linecount = 0;
                istr >> objectName;
                Hf::Log.info("%s(): %s ... adding new object %s", __FUNCTION__, fpi_orig.fname.c_str(), objectName.c_str());
            }
            else if (str == "g")
            {
                linecount = 0;
                istr >> surfaceName;
                Hf::Log.info("%s(): %s ... changing surface name to %s", __FUNCTION__, fpi_orig.fname.c_str(), surfaceName.c_str());
                Surfaces[curSurface].surfaceName = surfaceName;
            }
            else if (str == "usemtl")
            {
                bool createSurface = false;

                // flush old polygons to last surface (if any)
                if (Surfaces.size() != 0 && !faceList.empty())
                {
                    Surfaces[curSurface].count = (int)faceList.size() * 3;

                    Hf::Log.info("%s(): %s ... adding %d new faces startnig at %d to %s", __FUNCTION__, fpi_orig.fname.c_str(), faceList.size(), first, Surfaces[curSurface].surfaceName.c_str());

                    // 2. add indices (triangles)
                    for (auto it = faceList.begin(); it != faceList.end(); it++)
                    {
                        Indices.push_back(it->x + firstVertex);
                        Indices.push_back(it->y + firstVertex);
                        Indices.push_back(it->z + firstVertex);
                    }
                    first = (int)Indices.size();
                    faceList.clear();
                    createSurface = true;
                }

                if (createSurface)
                {
                    // create new surface
                    Surface surface;
                    surface.first = first;
                    surface.mode = GL_TRIANGLES;
                    Surfaces.push_back(surface);
                    curSurface = (int)Surfaces.size() - 1;
                }

                istr >> str;
                Surfaces[curSurface].materialName = str;
                Hf::Log.info("%s(): %s ... using material %s", __FUNCTION__, fpi_orig.fname.c_str(), str.c_str());
            }
            else if (str == "mtllib")
            {
                istr >> materialLibrary;
                Hf::Log.info("%s(): %s ... ignoring mtllib %s", __FUNCTION__, fpi_orig.fname.c_str(), materialLibrary.c_str());
            }
            else if (str == "v")
            {
                istr >> v[0] >> v[1] >> v[2];
                vList.push_back(Vector3f(v));
            }
            else if (str == "vn")
            {
                istr >> v[0] >> v[1] >> v[2];
                vnList.push_back(Vector3f(v));
            }
            else if (str == "vt")
            {
                istr >> v[0] >> v[1];
                vtList.push_back(Vector2f(v));
            }
            else if (str == "f")
            {
                int i1 = 0, i2 = 0, i3 = 0;
                long long longIndex;
                char trashChar;
                for (int k = 0; k < 3; k++)
                {
                    istr >> i1;
                    istr >> trashChar;
                    if (trashChar != '/')
                    {
                        // single position (ptr only)
                        istr.putback(trashChar);
                        i2 = i3 = -1;
                    }
                    else
                    {
                        istr >> trashChar;
                        if (trashChar == '/')
                        {
                            // ptr//vn case
                            i2 = -1;
                            istr >> i3;
                        }
                        else
                        {
                            istr.putback(trashChar);
                            istr >> i2 >> trashChar;
                            if (trashChar != '/')
                            {
                                istr.putback(trashChar);
                                i3 = i2;
                                i2 = -1;
                            }
                            else
                            {
                                istr >> i3;
                            }
                        }
                    }
                    if (i1 < 0)
                        i1 = (int)vList.size() + i1;
                    if (i2 < 0)
                        i2 = (int)vtList.size() + i2;
                    if (i3 < 0)
                        i3 = (int)vnList.size() + i3;

                    if (!optimizeIndexing)
                    {
                        // BEGIN NOT TRYING TO INDEX THESE THINGS
                        iv[k] = lastIndex + k;
                        Vertex vtx;
                        vtx.position = vList[i1 - 1];
                        if (i3 >= 0)
                            vtx.normal = vnList[i3 - 1];
                        if (i2 >= 0)
                            vtx.texcoord = vtList[i2 - 1];
                        vertexMap[iv[k]] = vtx;
                        BoundingBox += vtx.position;
                        // END NOT TRYING TO INDEX THESE THINGS
                    }
                    else
                    {
                        longIndex = i1;
                        if (i2 >= 0)
                            longIndex |= (long long)(i2) << 48;
                        if (i3 >= 0)
                            longIndex |= (long long)(i3) << 24;
                        if (indexMap.find(longIndex) == indexMap.end())
                        {
                            indexMap[longIndex] = newIndex;
                            newIndex++;
                        }
                        iv[k] = indexMap[longIndex];

                        // create vertex if needed
                        Vertex vtx;
                        if (vertexMap.find(iv[k]) == vertexMap.end())
                        {
                            vtx.position = vList[i1 - 1];
                            if (i3 >= 0)
                                vtx.normal = vnList[i3 - 1];
                            if (i2 >= 0)
                                vtx.texcoord = vtList[i2 - 1];
                            vertexMap[iv[k]] = vtx;
                            BoundingBox += vtx.position;
                        }
                    }
                }
                if (optimizeIndexing)
                {
                    // create face
                    faceList.push_back(TVector3<int>(iv));
                }
                else
                {
                    faceList.push_back(TVector3<int>(lastIndex, lastIndex + 1, lastIndex + 2));
                    lastIndex += 3;
                }
            }
        }
        fin.close();

        float scale = 1;
        if (size < 0)
            scale = (float)(-2 * size / BoundingBox.MaxSize());

        Hf::Log.info("%s(): %s ... scale is %f", __FUNCTION__, fpi_orig.fname.c_str(), scale);
        for (auto it = vertexMap.begin(); it != vertexMap.end(); it++)
        {
            if (size < 0)
                it->second.position -= BoundingBox.Center();

#define MAKE_FINITE(x)  \
    if (!isfinite((x))) \
        (x) = 0.0;
            //it->second.position *= scale;

            MAKE_FINITE(it->second.position.x);
            MAKE_FINITE(it->second.position.y);
            MAKE_FINITE(it->second.position.z);
            MAKE_FINITE(it->second.normal.x);
            MAKE_FINITE(it->second.normal.y);
            MAKE_FINITE(it->second.normal.z);
            MAKE_FINITE(it->second.texcoord.x);
            MAKE_FINITE(it->second.texcoord.y);
            MAKE_FINITE(it->second.binormal.x);
            MAKE_FINITE(it->second.binormal.y);
            MAKE_FINITE(it->second.binormal.z);
            MAKE_FINITE(it->second.tangent.x);
            MAKE_FINITE(it->second.tangent.y);
            MAKE_FINITE(it->second.tangent.z);

            Vertices.push_back(it->second);
        }
        Hf::Log.info("%s(): %s ... max uniform scale is %f", __FUNCTION__, fpi_orig.fname.c_str(), BoundingBox.MaxSize());

        ComputeTangentVectors();
    }
    else
    {
        // Load Cache instead
        std::ifstream fin(cache_filename, std::ios::binary);
        if (!fin) return false;
        long vertexCount = 0;
        long indexCount = 0;
        long surfaceCount = 0;

        fin.read(reinterpret_cast<char *>(&vertexCount), sizeof(long));
        fin.read(reinterpret_cast<char *>(&indexCount), sizeof(long));
        fin.read(reinterpret_cast<char *>(&surfaceCount), sizeof(long));

        Vertices.resize(vertexCount);
        Indices.resize(indexCount);
        Surfaces.resize(surfaceCount);
        BoundingBox.Reset();

        fin.read(reinterpret_cast<char *>(&Vertices[0]), sizeof(Vertex) * vertexCount);
        fin.read(reinterpret_cast<char *>(&Indices[0]), sizeof(GLuint) * indexCount);

        for (int i = 0; i < surfaceCount; i++)
        {
            long mode = 0;
            long first = 0;
            long count = 0;
            std::string mtlName;
            std::string surfaceName;
            long mtlNameSize = 0;
            long surfaceNameSize = 0;

            fin.read(reinterpret_cast<char *>(&mode), sizeof(long));
            fin.read(reinterpret_cast<char *>(&first), sizeof(long));
            fin.read(reinterpret_cast<char *>(&count), sizeof(long));
            fin.read(reinterpret_cast<char *>(&mtlNameSize), sizeof(long));
            mtlName.resize(mtlNameSize);
            fin.read(reinterpret_cast<char *>(&mtlName[0]), mtlNameSize);
            fin.read(reinterpret_cast<char *>(&surfaceNameSize), sizeof(long));
            surfaceName.resize(surfaceNameSize);
            fin.read(reinterpret_cast<char *>(&surfaceName[0]), surfaceNameSize);

            Surfaces[i].mode = mode;
            Surfaces[i].first = first;
            Surfaces[i].count = count;
            Surfaces[i].materialName = mtlName;
            Surfaces[i].surfaceName = surfaceName;
        }

        for (auto &v : Vertices)
        {
            BoundingBox += v.position;
        }

        fin.close();
    }

    if (cacheDoesNotExist)
    {
        // save a cache
		Hf::Log.infofn(__FUNCTION__, "Writing cache %s", cache_filename.c_str());

        std::ofstream fout(cache_filename, std::ios::binary);

        long vertexCount = (long)Vertices.size();
        long indexCount = (long)Indices.size();
        long surfaceCount = (long)Surfaces.size();

        fout.write(reinterpret_cast<char *>(&vertexCount), sizeof(long));
        fout.write(reinterpret_cast<char *>(&indexCount), sizeof(long));
        fout.write(reinterpret_cast<char *>(&surfaceCount), sizeof(long));

        fout.write(reinterpret_cast<char *>(&Vertices[0]), sizeof(Vertex) * vertexCount);
        fout.write(reinterpret_cast<char *>(&Indices[0]), sizeof(GLuint) * indexCount);

        for (int i = 0; i < surfaceCount; i++)
        {
            long mode = Surfaces[i].mode;
            long first = Surfaces[i].first;
            long count = Surfaces[i].count;
            size_t mtlNameSize = Surfaces[i].materialName.size();
            const char *mtlName = Surfaces[i].materialName.c_str();
            size_t surfaceNameSize = Surfaces[i].surfaceName.size();
            const char *surfaceName = Surfaces[i].surfaceName.c_str();

            fout.write(reinterpret_cast<char *>(&mode), sizeof(long));
            fout.write(reinterpret_cast<char *>(&first), sizeof(long));
            fout.write(reinterpret_cast<char *>(&count), sizeof(long));
            fout.write(reinterpret_cast<char *>(&mtlNameSize), sizeof(long));
            fout.write(mtlName, mtlNameSize);
            fout.write(reinterpret_cast<char *>(&surfaceNameSize), sizeof(long));
            fout.write(surfaceName, surfaceNameSize);
        }

        fout.close();
        Hf::Log.info("%s(): %s is finished loading.", __FUNCTION__, fpi_orig.fname.c_str());
    }

    return true;
}

bool OBJStaticModel::SaveOBJ(const std::string &filename)
{
	Hf::Log.infofn(__FUNCTION__, "Writing OBJ %s", filename.c_str());

    std::ofstream fout(filename.c_str());
    // 1. Output Vertices
    for (auto it = Vertices.begin(); it != Vertices.end(); it++)
    {
        fout << "v ";
        WriteVector3f(fout, it->position);
        fout << std::endl;
        fout << "vn ";
        WriteVector3f(fout, it->normal);
        fout << std::endl;
        fout << "vt ";
        WriteVector2f(fout, it->texcoord);
        fout << std::endl;
    }

    // 2. Output Surfaces
    for (auto it = Surfaces.begin(); it != Surfaces.end(); it++)
    {
        fout << "usemtl " << it->materialName << std::endl;
        int triangle = 2;
        for (int i = 0; i < it->count; i++)
        {
            if (++triangle == 3)
            {
                fout << std::endl;
                if (it->count - i >= 3)
                    fout << "f ";
                triangle = 0;
            }
            fout << Indices[it->first + i] << "/";
            fout << Indices[it->first + i] << "/";
            fout << Indices[it->first + i] << " ";
        }
        fout << std::endl;
    }

    fout.close();
    return true;
}

void OBJStaticModel::SavePrecompiled(const std::string &filename, const std::string objname)
{
	Hf::Log.infofn(__FUNCTION__, "Writing precompiled %s", filename.c_str());

    std::string outFilename = filename + ".cpp";
    std::ofstream fout(outFilename.c_str());

    std::string prefix = "OBJ_" + objname + "_";

    fout << "extern const int " << prefix << "IndicesSize;\n";
    fout << "extern const int " << prefix << "VerticesSize;\n";
    fout << "extern const int " << prefix << "SurfacesSize;\n";
    fout << "extern const int " << prefix << "Indices[];\n";
    fout << "extern const float " << prefix << "Vertices[];\n";
    fout << "extern const OBJSurface " << prefix << "Surfaces[];\n";

    fout << "#include <vector>\n";
    fout << "#include \"fluxions_precompiled_models.hpp\"\n\n";
    fout << "namespace Fluxions {\n";
    fout << "using namespace std;\n\n";

    fout << "const int " << prefix << "IndicesSize = " << Indices.size() << ";\n";
    fout << "const int " << prefix << "VerticesSize = " << Vertices.size() << ";\n";
    fout << "const int " << prefix << "SurfacesSize = " << Surfaces.size() << ";\n\n";

    // size_t IndicesSize = Indices.size();
    // size_t VerticesSize = Vertices.size();
    // size_t SurfacesSize = Surfaces.size();

    size_t totalCount = Indices.size();
    size_t newLineCount = 0;
    fout << "const int " << prefix << "Indices[] = {\n\t";
    for (auto it = Indices.begin(); it != Indices.end(); it++)
    {
        fout << *it;
        if (--totalCount != 0)
            fout << ", ";
        if (newLineCount++ > 10)
        {
            fout << "\n\t";
            newLineCount = 0;
        }
    }
    fout << "\n};\n\n";

    newLineCount = 0;
    totalCount = (int)Vertices.size();
    fout << "const float " << prefix << "Vertices[] = {\n";
    for (auto it = Vertices.begin(); it != Vertices.end(); it++)
    {
        fout << "\t";
        fout << std::fixed << std::setprecision(9) << it->position.x << "f, ";
        fout << std::fixed << std::setprecision(9) << it->position.y << "f, ";
        fout << std::fixed << std::setprecision(9) << it->position.z << "f, ";
        fout << std::fixed << std::setprecision(9) << it->normal.x << "f, ";
        fout << std::fixed << std::setprecision(9) << it->normal.y << "f, ";
        fout << std::fixed << std::setprecision(9) << it->normal.z << "f, ";
        fout << std::fixed << std::setprecision(9) << it->texcoord.x << "f, ";
        fout << std::fixed << std::setprecision(9) << it->texcoord.y << "f, ";
        fout << std::fixed << std::setprecision(9) << it->tangent.x << "f, ";
        fout << std::fixed << std::setprecision(9) << it->tangent.y << "f, ";
        fout << std::fixed << std::setprecision(9) << it->tangent.z << "f, ";
        fout << std::fixed << std::setprecision(9) << it->binormal.x << "f, ";
        fout << std::fixed << std::setprecision(9) << it->binormal.y << "f, ";
        fout << std::fixed << std::setprecision(9) << it->binormal.z << "f";
        totalCount--;
        if (totalCount != 0)
            fout << ",\n";
        else
            fout << "\n";
    }
    fout << "};\n\n";

    totalCount = (int)Surfaces.size();
    fout << "const OBJSurface " << prefix << "Surfaces[] = {\n";
    for (auto it = Surfaces.begin(); it != Surfaces.end(); it++)
    {
        fout << "\t{ ";
        fout << "GL_TRIANGLES, ";
        fout << it->first << ", ";
        fout << it->count << ", ";
        fout << "\"" << it->materialName << "\", ";
        fout << "\"" << it->surfaceName << "\" ";
        fout << "}";
        if (--totalCount != 0)
            fout << ",\n";
        else
            fout << "\n";
    }
    fout << "};\n\n} // namespace Fluxions\n\n";
    fout.close();
}

void OBJStaticModel::LoadPrecompiled(const int numIndices, const int *indices, const int numVertices, const float *vertices, const int numSurfaces, const OBJSurface *surfaces)
{
    Clear();
    Resize(numVertices, numIndices, numSurfaces);
    for (int i = 0; i < numIndices; i++)
    {
        Indices[i] = indices[i];
    }

    for (int i = 0; i < numVertices; i++)
    {
        Vertices[i].position.x = vertices[i * 14 + 0];
        Vertices[i].position.y = vertices[i * 14 + 1];
        Vertices[i].position.z = vertices[i * 14 + 2];
        Vertices[i].normal.x = vertices[i * 14 + 3];
        Vertices[i].normal.y = vertices[i * 14 + 4];
        Vertices[i].normal.z = vertices[i * 14 + 5];
        Vertices[i].texcoord.x = vertices[i * 14 + 6];
        Vertices[i].texcoord.y = vertices[i * 14 + 7];
        Vertices[i].binormal.x = vertices[i * 14 + 8];
        Vertices[i].binormal.y = vertices[i * 14 + 9];
        Vertices[i].binormal.z = vertices[i * 14 + 10];
        Vertices[i].tangent.x = vertices[i * 14 + 11];
        Vertices[i].tangent.y = vertices[i * 14 + 12];
        Vertices[i].tangent.z = vertices[i * 14 + 13];
    }

    for (int i = 0; i < numSurfaces; i++)
    {
        Surfaces[i].mode = surfaces[i].mode;
        Surfaces[i].first = surfaces[i].first;
        Surfaces[i].count = surfaces[i].count;
        Surfaces[i].materialName = surfaces[i].materialName;
        Surfaces[i].surfaceName = surfaces[i].surfaceName;
    }
}

void OBJStaticModel::CreateSimpleModel(int vertexCount, int indexCount, int surfaceCount)
{
    Clear();
    Resize(vertexCount, indexCount, surfaceCount);
}

void OBJStaticModel::Resize(int vertexCount, int indexCount, int surfaceCount)
{
    if (vertexCount > 0)
        Vertices.resize(vertexCount);
    if (indexCount > 0)
        Indices.resize(indexCount);
    if (surfaceCount > 0)
        Surfaces.resize(surfaceCount);
}

void OBJStaticModel::Clear()
{
    Vertices.clear();
    Indices.clear();
    Surfaces.clear();
}

void OBJStaticModel::Transform(const Matrix4d &mat)
{
    std::vector<Vertex>::iterator it;
    for (it = Vertices.begin(); it != Vertices.end(); it++)
    {
        Vector3d vtx = it->position;
        it->position = (mat * Vector4d(vtx, 1.0)).xyz();
    }
}

void OBJStaticModel::ComputeTangentVectors()
{
    for (size_t i = 0; i < Vertices.size(); i++)
    {
        Vertices[i].tangent = Vector3f(0, 0, 0);
        Vertices[i].binormal = Vector3f(0, 0, 0);
    }

    for (size_t i = 0; i < Indices.size(); i += 3)
    {
        int indices[3];
        indices[0] = Indices[i];
        indices[1] = Indices[i + 1];
        indices[2] = Indices[i + 2];
        Vector3f v1, v2;
        v1 = Vertices[indices[1]].position - Vertices[indices[0]].position;
        v2 = Vertices[indices[2]].position - Vertices[indices[0]].position;
        Vector3f faceNormal = CrossProduct(v1, v2);
        faceNormal.normalize();

        float s1, t1, s2, t2;
        s1 = Vertices[indices[1]].texcoord.x - Vertices[indices[0]].texcoord.x;
        t1 = Vertices[indices[1]].texcoord.y - Vertices[indices[0]].texcoord.y;
        s2 = Vertices[indices[2]].texcoord.x - Vertices[indices[0]].texcoord.x;
        t2 = Vertices[indices[2]].texcoord.x - Vertices[indices[0]].texcoord.y;
        Vector3f Tx, Ty, Tz;
        float coef = (float)(1.0 / (s1 * t2 - s2 * t1));
        Tx.x = coef * (v1.x * t2 - v2.x * t1);
        Tx.y = coef * (v1.y * t2 - v2.y * t1);
        Tx.z = coef * (v1.z * t2 - v2.z * t1);
        Ty.x = coef * (v2.x * s1 - v1.x * s2);
        Ty.y = coef * (v2.y * s1 - v1.y * s2);
        Ty.z = coef * (v2.z * s1 - v1.z * s2);

        for (int j = 0; j < 3; j++)
        {
            Vertices[indices[j]].tangent += Tx;
            Vertices[indices[j]].binormal += Ty;
        }
    }

    for (size_t i = 0; i < Vertices.size(); i++)
    {
        Vertices[i].normal.normalize();
        Vertices[i].tangent.normalize();
        Vertices[i].binormal.normalize();

        MAKE_FINITE(Vertices[i].position.x);
        MAKE_FINITE(Vertices[i].position.y);
        MAKE_FINITE(Vertices[i].position.z);
        MAKE_FINITE(Vertices[i].normal.x);
        MAKE_FINITE(Vertices[i].normal.y);
        MAKE_FINITE(Vertices[i].normal.z);
        MAKE_FINITE(Vertices[i].texcoord.x);
        MAKE_FINITE(Vertices[i].texcoord.y);
        MAKE_FINITE(Vertices[i].binormal.x);
        MAKE_FINITE(Vertices[i].binormal.y);
        MAKE_FINITE(Vertices[i].binormal.z);
        MAKE_FINITE(Vertices[i].tangent.x);
        MAKE_FINITE(Vertices[i].tangent.y);
        MAKE_FINITE(Vertices[i].tangent.z);
        //Vector3f n = Vertices[i].normal;
        //Vector3f t = Vertices[i].tangent;
        //Vector3f b = Vertices[i].binormal;
        //t.Normalize();
        //b.Normalize();

        //float T2 = t.LengthSquared();
        //Vector3f tprime = (t - n * DotProduct(n, t));
        //Vector3f bprime = (b - n * DotProduct(n, n) - tprime * DotProduct(tprime, b) / T2);

        //Matrix3f M(tprime.X, tprime.y, tprime.z, bprime.X, bprime.y, bprime.z, n.X, n.y, n.z);

        //Vertices[i].tangent = tprime;
        //Vertices[i].tangent.w = M.Determinant();
        //Vertices[i].tangent.w = (DotProduct(CrossProduct(n, t), b) < 0.0f) ? -1.0f : 1.0f;
        //Vertices[i].binormal = bprime;
    }
}

void OBJStaticModel::Render(SimpleRenderer_GLuint &renderer)
{
    int curIndex;

    renderer.vertexCount += (int)Vertices.size();

    renderer.Begin(GL_TRIANGLES, true);
    curIndex = 0;
    for (auto vIt = Vertices.begin(); vIt != Vertices.end(); vIt++, curIndex++)
    {
        renderer.VertexAttrib4f(1, Vertices[curIndex].normal.x, Vertices[curIndex].normal.y, Vertices[curIndex].normal.z, 1.0f);
        renderer.VertexAttrib4f(2, Vertices[curIndex].texcoord.x, Vertices[curIndex].texcoord.y, 0.0f, 1.0f);
        renderer.VertexAttrib4f(6, Vertices[curIndex].tangent.x, Vertices[curIndex].tangent.y, Vertices[curIndex].tangent.z, 1.0f);
        renderer.VertexAttrib4f(7, Vertices[curIndex].binormal.x, Vertices[curIndex].binormal.y, Vertices[curIndex].binormal.z, 1.0f);
        renderer.VertexAttrib4f(0, Vertices[curIndex].position.x, Vertices[curIndex].position.y, Vertices[curIndex].position.z, 1.0f);
    }
    renderer.End();

    std::vector<Surface>::iterator surfIt;
    for (surfIt = Surfaces.begin(); surfIt != Surfaces.end(); surfIt++)
    {
        renderer.triangleCount += surfIt->count / 3;
        renderer.SetCurrentMtlName(surfIt->materialName);
        renderer.Begin(GL_TRIANGLES, true);
        for (int i = surfIt->first; i < surfIt->first + surfIt->count; i++)
        {
            renderer.Index(Indices[i]);
        }
        renderer.End();
    }
    renderer.SetCurrentMtlName("");
}

void OBJStaticModel::RenderGL11()
{
    std::vector<Surface>::iterator surfIt;
    for (surfIt = Surfaces.begin(); surfIt != Surfaces.end(); surfIt++)
    {
        //renderer.SetCurrentMtlName(surfIt->materialName);
        glBegin(GL_TRIANGLES);
        for (int i = surfIt->first; i < surfIt->first + surfIt->count; i++)
        {
            int curIndex = Indices[i];
            glTexCoord2f(Vertices[curIndex].texcoord.x, Vertices[curIndex].texcoord.y);
            glNormal3f(Vertices[curIndex].normal.x, Vertices[curIndex].normal.y, Vertices[curIndex].normal.z);
            glVertex3f(Vertices[curIndex].position.x, Vertices[curIndex].position.y, Vertices[curIndex].position.z);
        }
        glEnd();
    }
}
} // namespace Fluxions
