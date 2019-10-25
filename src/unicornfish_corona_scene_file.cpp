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

#include <fluxions_gte.hpp>
#include <fluxions_fileio_iostream.hpp>
#include <fluxions_xml.hpp>
#include <unicornfish_corona_job.hpp>
#include <unicornfish_corona_scene_file.hpp>
#include <fluxions_ssg_ssphh_renderer_plugin.hpp>
#include <fluxions_simple_point_light.hpp>

namespace Uf
{
	using namespace Fluxions;

	//static const std::string corona_conf_prefix("corona_conf/");
	//static const std::string corona_export_prefix("corona_export/");
	//static const std::string corona_output_prefix("corona_output/");

	CoronaSceneFile::CoronaSceneFile() {}

	CoronaSceneFile::~CoronaSceneFile() {}

	void CoronaSceneFile::SetCameraType(const std::string& newCameraType) {
		if (newCameraType == "perspective")
			cameraType = newCameraType;
		if (newCameraType == "cubemap")
			cameraType = newCameraType;
	}

	void CoronaSceneFile::SetPerspectiveCamera(const Fluxions::Vector3f& origin, const Fluxions::Vector3f& target, const Fluxions::Vector3f& roll, const float horizontalFieldOfViewInDegrees) {
		cameraOrigin = origin;
		cameraTarget = target;
		cameraRoll = roll;
		cameraHorizontalFieldOfViewInDegrees = horizontalFieldOfViewInDegrees;
		cameraType = "perspective";
	}

	void CoronaSceneFile::SetCubeMapCamera(const Fluxions::Vector3f& origin, const Fluxions::Vector3f& target, const Fluxions::Vector3f& roll) {
		cameraOrigin = origin;
		cameraTarget = target;
		cameraRoll = roll;
		cameraType = "cubemap";
	}

	void CoronaSceneFile::WriteSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		SetCameraType("perspective");
		Fluxions::Vector3f origin = ssg.camera.viewMatrix.col4().xyz();
		Fluxions::Vector3f target = ssg.camera.viewMatrix.col4().xyz() + ssg.camera.viewMatrix.col3().xyz();
		Fluxions::Vector3f roll = ssg.camera.viewMatrix.col4().xyz() + ssg.camera.viewMatrix.col2().xyz();
		float hfovDegrees = (float)ssg.camera.fov;
		SetPerspectiveCamera(origin, target, roll, hfovDegrees);
		Fluxions::Matrix4f coronaMatrix(
			1.0, 0.0, 0.0, 0.0,
			0.0, 0.0, -1.0, 0.0,
			0.0, -1.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 1.0);
		cameraMatrix = ssg.camera.actualViewMatrix;

		Hf::Log.infofn(__FUNCTION__, "Writing Regular scene %s", filename.c_str());
		std::ofstream fout(filename);
		if (!fout)
			return;

		XmlBeginTag(fout, "scene") << std::endl;

		//fout << "<renderElement class=\"Components\" instanceName=\"CESSENTIAL_Direct\"><componentName>diffuseDirect</componentName></renderElement>" << std::endl;
		//fout << "<renderElement class=\"Components\" instanceName=\"CESSENTIAL_Reflect\"><componentName>diffuseDirect</componentName></renderElement>" << std::endl;
		//fout << "<renderElement class=\"Components\" instanceName=\"CESSENTIAL_Indirect\"><componentName>diffuseIndirect</componentName></renderElement>" << std::endl << std::endl;

		XmlString(fout, "conffile", "../" + CoronaJob::confPathPrefix + "export_corona_ground_truth.conf") << "\n\n";

		// Camera
		writeCamera(fout);

		// Sun
		writeSun(fout, ssg);

		// Geometry Groups
		writeGeometryGroups(fout, ssg);

		XmlEndTag(fout, "scene") << std::endl;

		fout.close();
	}

	void CoronaSceneFile::WriteCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		Fluxions::Vector3f cameraPosition = ssg.camera.actualViewMatrix.col4().xyz();
		WriteCubeMapSCN(filename, ssg, cameraPosition);
	}

	void CoronaSceneFile::WriteCubeMapSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, const Fluxions::Vector3f& cameraPosition) {
		SetCubeMapCamera(cameraPosition, cameraPosition + Fluxions::Vector3f(0.0f, 0.0f, -1.0f), Fluxions::Vector3f(0.0f, 1.0f, 0.0f));

		Hf::Log.infofn(__FUNCTION__, "Writing Cube Map SCN scene %s", filename.c_str());
		std::ofstream fout(filename);
		if (!fout)
			return;

		XmlBeginTag(fout, "scene") << std::endl;

		// Camera
		writeCamera(fout);

		// Sun
		writeSun(fout, ssg);

		// Geometry Groups
		writeGeometryGroups(fout, ssg);

		XmlEndTag(fout, "scene") << std::endl;

		fout.close();
	}

	void CoronaSceneFile::WriteSkySCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg) {
		Hf::Log.infofn(__FUNCTION__, "Writing Sky scene %s", filename.c_str());
		std::ofstream fout(filename);
		if (!fout)
			return;

		XmlBeginTag(fout, "scene") << std::endl;
		XmlString(fout, "conffile", "../" + CoronaJob::confPathPrefix + "ssphh_sky.conf", 1) << std::endl
			<< std::endl;
		SetCubeMapCamera(Fluxions::Vector3f(0.0f, 0.0f, 0.0f), Fluxions::Vector3f(0.0f, 1.0f, 0.0f), Fluxions::Vector3f(0.0f, 0.0f, 1.0f));
		writeCamera(fout);
		writeSun(fout, ssg);
		XmlEndTag(fout, "scene") << std::endl;

		fout.close();
	}

	bool CoronaSceneFile::WriteSphlVizSCN(const std::string& filename, const Fluxions::SimpleSceneGraph& ssg, int sourceLightIndex, int receivingLightIndex) {
		SSG_SSPHHRendererPlugin *ssphh = dynamic_cast<SSG_SSPHHRendererPlugin*>(ssg.userdata);
		if (!ssphh) {
			HFLOGERROR("ssphh pointer is null");
			return false;
		}

		int lastIndex = (int)ssphh->ssphhLights.size() - 1;
		if (!within(sourceLightIndex, -1, lastIndex) || !within(receivingLightIndex, -1, lastIndex)) {
			return false;
		}

		Hf::Log.infofn(__FUNCTION__, "Writing VIZ scene %s", filename.c_str());
		std::ofstream fout(filename);
		if (!fout)
			return false;

		XmlBeginTag(fout, "scene") << std::endl;
		XmlString(fout, "conffile", "../" + CoronaJob::confPathPrefix + "sphlviz.conf", 1) << std::endl
			<< std::endl;

		// Camera
		if (receivingLightIndex < 0) {
			Fluxions::Vector3f position = ssg.camera.actualViewMatrix.col4().xyz();
			SetCubeMapCamera(position, Fluxions::Vector3f(0.0f, 0.0f, 0.0f), Fluxions::Vector3f(0.0f, 1.0f, 0.0f));
		}
		else {
			const SimpleSSPHHLight& sphl2 = ssphh->ssphhLights[receivingLightIndex];
			Fluxions::Vector3f tmp = sphl2.position.xyz();
			Fluxions::Vector3f position(tmp.x, -tmp.z, tmp.y);
			Fluxions::Vector3f target(position.x, position.y - 1.0f, position.z);
			SetCubeMapCamera(position, target, Fluxions::Vector3f(0.0f, 0.0f, 1.0f));
		}
		writeCamera(fout);

		// Geometry Groups
		writeGeometryGroups(fout, ssg);

		if (sourceLightIndex < 0) {
			writeSun(fout, ssg);
		}
		else {
			const SimpleSSPHHLight& sphl1 = ssphh->ssphhLights[sourceLightIndex];
			Fluxions::Matrix4f lightMatrix(
				0.1f, 0.0f, 0.0f, sphl1.position.x,
				0.0f, 0.1f, 0.0f, -sphl1.position.z,
				0.0f, 0.0f, 0.1f, sphl1.position.y,
				0.0f, 0.0f, 0.0f, 1.0f);
			FilePathInfo fpi(CoronaJob::exportPathPrefix + "sphlviz.mtl");
			if (!fpi.Exists()) {
				Hf::Log.infofn(__FUNCTION__, "Writing out sphlviz.mtl");
				std::ofstream svout(CoronaJob::exportPathPrefix + "sphlviz.mtl");
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
			// int tab = 0;
			XmlString(fout, "mtllib", "sphlviz.mtl", 1) << std::endl;
			XmlBeginTag(fout, "geometryGroup", 1) << std::endl;
			XmlBeginTag(fout, "object", "sphere", 2) << std::endl;
			XmlInt(fout, "materialId", 0, 3) << std::endl;
			XmlEndTag(fout, "object", 2) << std::endl;
			XmlBeginTag(fout, "instance", 2) << std::endl;
			XmlBeginTag(fout, "material", "Reference", 3) << "sphlSphereLight";
			XmlEndTag(fout, "material") << std::endl;
			XmlMatrix4f(fout, "transform", lightMatrix, 3) << std::endl;
			XmlEndTag(fout, "instance", 2) << std::endl;
			XmlEndTag(fout, "geometryGroup", 1) << std::endl;
		}

		XmlEndTag(fout, "scene");
		fout.close();
		return true;
	}

	void CoronaSceneFile::writeCamera(std::ostream& ostr) {
		Fluxions::Matrix4f fixMatrix(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		XmlBeginTag(ostr, "camera", cameraType, 1) << std::endl;
		if (cameraType == "perspective") {
			Fluxions::Matrix4f fixed = cameraMatrix * fixMatrix;
			// Vector4f position = fixed.col4();
			XmlMatrix4f(ostr, "tm", fixed, 2) << std::endl;
			XmlFloat(ostr, "fov", cameraHorizontalFieldOfViewInDegrees, 2) << std::endl;
		}
		if (cameraType == "cubemap") {
			XmlVector3f(ostr, "origin", cameraOrigin, 2) << std::endl;
			XmlVector3f(ostr, "target", cameraTarget, 2) << std::endl;
			XmlVector3f(ostr, "roll", cameraRoll, 2) << std::endl;
		}
		XmlEndTag(ostr, "camera", 1) << std::endl
			<< std::endl;
	}

	void CoronaSceneFile::writeSun(std::ostream& ostr, const Fluxions::SimpleSceneGraph& ssg) {
		std::string lights_mtllib = "ssphh_lights.mtl";
		XmlString(ostr, "mtllib", lights_mtllib, 1) << std::endl;
		XmlBeginTag(ostr, "sun", 1) << std::endl;
		Fluxions::Vector3f sunDirTo(ssg.environment.curSunDirTo.x, -ssg.environment.curSunDirTo.z, ssg.environment.curSunDirTo.y);
		XmlVector3f(ostr, "dirTo", sunDirTo, 2) << std::endl;
		//XmlVector3f(ostr, "color", ssg.environment.color, 2) << std::endl;
		//XmlFloat(ostr, "sizeMult", 1.0f, 2) << std::endl;
		// Corona 1.6+
		XmlFloat(ostr, "turbidity", ssg.environment.pbsky.GetTurbidity(), 2) << std::endl;
		XmlEndTag(ostr, "sun", 1) << std::endl
			<< std::endl;

		ostr << "\t<environment>1 1 1<map class=\"Reference\">Skylight_environment</map></environment>" << std::endl
			<< std::endl;

		// <mtlLib>
		// <mapDefinition name="Skylight_environment">
		// 	<map class="Sky" originalMapClass="SkyShader">
		// 		<groundColor>0.2 0.2 0.2</groundColor>
		// 	</map>
		// </mapDefinition>
		// </mtlLib>

		// XmlString(ostr, "mtllib", "ssphh_lights.mtl", 1) << std::endl;

		const float size = 0.1f;
		for (unsigned i = 0; i < ssg.pointLights.size(); i++) {
			const SimplePointLight& pointLight = ssg.pointLights[i];
			Fluxions::Matrix4f lightMatrix(
				size, 0.0f, 0.0f, pointLight.position.x,
				0.0f, size, 0.0f, -pointLight.position.z,
				0.0f, 0.0f, size, pointLight.position.y,
				0.0f, 0.0f, 0.0f, 1.0f);
			// int tab = 0;
			std::ostringstream name;
			name << "PointLight" << i;

			XmlComment(ostr, name.str(), 1);
			XmlBeginTag(ostr, "geometryGroup", 1) << std::endl;
			XmlBeginTag(ostr, "object", "sphere", 2) << std::endl;
			XmlSize(ostr, "materialId", i, 3) << std::endl;
			XmlEndTag(ostr, "object", 2) << std::endl;
			XmlBeginTag(ostr, "instance", 2) << std::endl;
			XmlBeginTag(ostr, "material", "Reference", 3) << "PointLight" << i;
			XmlEndTag(ostr, "material") << std::endl;
			XmlMatrix4f(ostr, "transform", lightMatrix, 3) << std::endl;
			XmlEndTag(ostr, "instance", 2) << std::endl;
			XmlEndTag(ostr, "geometryGroup", 1) << std::endl
				<< std::endl;
		}

		// Generate lights MTLLIB

		std::string lights_mtllib_path = CoronaJob::exportPathPrefix + lights_mtllib;
		Hf::Log.infofn(__FUNCTION__, "Writing lights_mtllib %s", lights_mtllib_path.c_str());
		std::ofstream fout(lights_mtllib_path);
		XmlBeginTag(fout, "mtlLib") << std::endl;
		XmlBeginTag(fout, "mapDefinition", "name", "Skylight_environment", 1) << std::endl;
		fout << "\t<map class = \"Sky\" originalMapClass = \"SkyShader\">\n";
		XmlVector3f(fout, "groundColor", ssg.environment.pbsky.GetGroundAlbedo().ToVector3(), 2) << std::endl;
		XmlEndTag(fout, "map", 1) << std::endl;
		XmlEndTag(fout, "mapDefinition", 1) << std::endl
			<< std::endl;

		for (unsigned i = 0; i < ssg.pointLights.size(); i++) {
			std::ostringstream name;
			name << "PointLight" << i;
			XmlBeginTag(fout, "materialDefinition", "name", name.str(), 1) << std::endl;
			XmlBeginTag(fout, "material", "Native", 2) << std::endl;
			XmlBeginTag(fout, "emission", 3) << std::endl;
			XmlVector3f(fout, "color", Fluxions::Vector3f(ssg.pointLights[i].E0 / size), 4) << std::endl;
			XmlEndTag(fout, "emission", 3) << std::endl;
			XmlEndTag(fout, "material", 2) << std::endl;
			XmlEndTag(fout, "materialDefinition", 1) << std::endl
				<< std::endl;
		}

		XmlEndTag(fout, "mtlLib") << std::endl;
		fout.close();
	}

	void CoronaSceneFile::ClearCache() {
		geometryGroups.clear();
	}

	void CoronaSceneFile::WriteCache(const Fluxions::SimpleSceneGraph& ssg) {
		if (geometryGroups.empty()) {
			std::map<std::string, int> written_materials;
			std::map<std::string, std::string> written_maps;

			std::string MTLpath = CoronaJob::exportPathPrefix + "materials.mtl";
			std::string CoronaMTLpath = CoronaJob::exportPathPrefix + "materials_corona.mtl";
			Hf::Log.infofn(__FUNCTION__, "Writing out %s", MTLpath.c_str());
			Hf::Log.infofn(__FUNCTION__, "Writing out %s", CoronaMTLpath.c_str());
			std::ofstream mtl_fout(MTLpath);
			std::ofstream mtlxml_fout(CoronaMTLpath);
			XmlBeginTag(mtlxml_fout, "mtlLib") << std::endl;

			std::string last_mtllib;
			std::string mtl_name;
			int obj_count = 0;
			auto it = ssg.geometry.cbegin();
			for (unsigned i = 0; i < ssg.geometry.size(); i++, it++) {
				const SimpleGeometryGroup& sgo = it->second;
				const OBJStaticModel& obj = ssg.geometryObjects[sgo.objectId];

				if (obj.Surfaces.empty())
					continue;

				for (auto& surface : obj.Surfaces) {
					std::ostringstream obj_pathname;
					obj_pathname << "object_" << std::setw(3) << std::setfill('0') << obj_count << "_" << sgo.objectName << ".obj";
					std::string OBJpath = CoronaJob::exportPathPrefix + obj_pathname.str();
					Hf::Log.infofn(__FUNCTION__, "Writing out %s", OBJpath.c_str());
					std::ofstream obj_fout(OBJpath);

					mtl_name = sgo.objectName + "_" + surface.materialName;
					for (auto& c : mtl_name) {
						if (c == ' ')
							c = '_';
					}

					if (written_materials[mtl_name] == false) {
						const SimpleMaterial* mtl = ssg.materials.GetLibraryMaterial(sgo.mtllibName, surface.materialName);
						if (mtl) {
							written_materials[mtl_name] = (int)written_materials.size();
							mtl_fout << "newmtl " << mtl_name << std::endl;

							mtl_fout << "Kd ";
							WriteColor3f(mtl_fout, mtl->Kd);
							mtl_fout << std::endl;
							if (enableKs) {
								mtl_fout << "Ks ";
								WriteColor3f(mtl_fout, mtl->Ks);
								mtl_fout << std::endl;
							}
							else {
								mtl_fout << "Ks 0.0 0.0 0.0" << std::endl;
							}
							mtl_fout << std::endl;

							//ostream & fout = mtlxml_fout;

							if (!mtl->map_Kd.empty()) {
								written_maps[mtl->map_Kd] = ssg.materials.GetMapPath(mtl->map_Kd);
							}
							if (!mtl->map_bump.empty()) {
								written_maps[mtl->map_bump] = ssg.materials.GetMapPath(mtl->map_bump);
							}

							XmlBeginTag(mtlxml_fout, "materialDefinition", "name", mtl_name, 1) << std::endl;
							XmlBeginTag(mtlxml_fout, "material", "Native", 2) << std::endl;
							if (!mtl->map_Kd.empty()) {
								XmlBeginTag(mtlxml_fout, "diffuse", 3) << std::endl;
								XmlBeginTag(mtlxml_fout, "map", "Reference", 4);
								mtlxml_fout << mtl->map_Kd;
								XmlEndTag(mtlxml_fout, "map") << std::endl;
								XmlEndTag(mtlxml_fout, "diffuse", 3) << std::endl;
							}
							else {
								if (mtl->PBk > 0.0)
									XmlColor3f(mtlxml_fout, "diffuse", Color3f(), 3) << std::endl;
								else
									XmlColor3f(mtlxml_fout, "diffuse", mtl->Kd, 3) << std::endl;
							}

							if (enableKs) {
								XmlBeginTag(mtlxml_fout, "reflect", 3) << std::endl;
								XmlColor3f(mtlxml_fout, "color", mtl->Ks, 4) << std::endl;

								if (mtl->PBk > 0.0) {
									XmlFloat(mtlxml_fout, "ior", 999, 4) << std::endl;
								}
								else {
									XmlFloat(mtlxml_fout, "ior", mtl->PBior, 4) << std::endl;
								}
								// Corona uses glossiness which is the reverse of roughness
								// A surface is smooth if roughness is 0 or glossiness is 1
								// A surface is rough if roughness is 1 or glossiness is 0
								if (mtl->PBKsm < 0)
									XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f + mtl->PBKsm, 0.0f, 1.0f), 4) << std::endl;
								else
									XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f - mtl->PBKsm, 0.0f, 1.0f), 4) << std::endl;
								XmlEndTag(mtlxml_fout, "reflect", 3) << std::endl;
							}

							//if (!mtl->map_bump.empty())
							//{
							//	XmlString(mtlxml_fout, "bump", mtl->map_bump, 3) << std::endl;
							//}
							XmlEndTag(mtlxml_fout, "material", 2) << std::endl;
							XmlEndTag(mtlxml_fout, "materialDefinition", 1) << std::endl;
						}
					}
					if (0) {
						obj_fout << "mtllib "
							<< "materials_corona.mtl" << std::endl;
						obj_fout << "usemtl " << mtl_name << std::endl;
					}
					else {
						obj_fout << "usemtl " << written_materials[mtl_name] << std::endl;
					}
					// output vertices
					obj_fout << "# surface(" << surface.first << ", " << surface.count << ") \"" << surface.surfaceName << "\"" << std::endl;
					for (unsigned index = surface.first; index < surface.first + surface.count; index++) {
						obj_fout << "v ";
						WriteVector3f(obj_fout, obj.Vertices[index].position) << std::endl;
					}
					for (unsigned index = surface.first; index < surface.first + surface.count; index++) {
						obj_fout << "vn ";
						WriteVector3f(obj_fout, obj.Vertices[index].normal) << std::endl;
					}
					for (unsigned index = surface.first; index < surface.first + surface.count; index++) {
						obj_fout << "vt ";
						WriteVector2f(obj_fout, obj.Vertices[index].texcoord) << std::endl;
					}

					// output triangle faces
					unsigned count = 0;
					for (unsigned index = surface.first; index < surface.first + surface.count; index++, count++) {
						if (count % 3 == 0)
							obj_fout << std::endl
							<< "f ";
						int v_index = index - surface.first + 1;
						obj_fout << v_index << "/" << v_index << "/" << v_index << " ";
					}
					obj_fout.close();

					Fluxions::Matrix4f fixMatrix(
						1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 0.0f, -1.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);

					std::string objectPath = obj_pathname.str();
					Fluxions::Matrix4f transform = fixMatrix * sgo.transform;

					//XmlBeginTag(ostr, "geometryGroup", 1) << std::endl;
					//XmlBeginTag(ostr, "object", "file", 2);
					//ostr << obj_pathname.str();
					//XmlEndTag(ostr, "object") << std::endl;
					//XmlBeginTag(ostr, "instance", 2) << std::endl;
					//XmlBeginTag(ostr, "material", "Reference", 3);
					//ostr << mtl_name;
					//XmlEndTag(ostr, "material", 0) << std::endl;
					//XmlMatrix4f(ostr, "transform", fixMatrix * sgo.transform, 3) << std::endl;
					//XmlEndTag(ostr, "instance", 2) << std::endl;
					//XmlEndTag(ostr, "geometryGroup", 1) << std::endl;

					geometryGroups.push_back(std::tuple<std::string, std::string, Fluxions::Matrix4f>(objectPath, mtl_name, transform));

					obj_count++;
				}
			}

			mtl_fout.close();

			for (auto& mapIt : written_maps) {
				XmlCoronaMapTexture(mtlxml_fout, "mapDefinition", mapIt.first, mapIt.first, 1, 2.2f) << std::endl;

#ifdef _WIN32
				// CopyFile(dst, src, bOverWrite?)
				CopyFile(mapIt.second.c_str(), (CoronaJob::exportPathPrefix + mapIt.first).c_str(), TRUE);
#else
				// POSIX version does not exist
				// So we will call system instead
				std::string cp_args = mapIt.second + " " + CoronaJob::exportPathPrefix + mapIt.first;
				execl("/bin/cp", cp_args.c_str());
#endif
				//copy_file(mapIt.second, corona_export_prefix + mapIt.first);
			}
			XmlEndTag(mtlxml_fout, "mtlLib");
			mtlxml_fout.close();
		}
	}

	void CoronaSceneFile::WriteMaterials(const Fluxions::SimpleSceneGraph& ssg, bool enableKs_) {
		std::map<std::string, int> written_materials;
		std::map<std::string, std::string> written_maps;

		std::string MTLpath = CoronaJob::exportPathPrefix + "materials.mtl";
		std::string CoronaMTLpath = CoronaJob::exportPathPrefix + "materials_corona.mtl";
		Hf::Log.infofn(__FUNCTION__, "Writing out %s", MTLpath.c_str());
		Hf::Log.infofn(__FUNCTION__, "Writing out %s", CoronaMTLpath.c_str());
		std::ofstream mtl_fout(MTLpath);
		std::ofstream mtlxml_fout(CoronaMTLpath);
		XmlBeginTag(mtlxml_fout, "mtlLib") << std::endl;

		std::string last_mtllib;
		std::string mtl_name;
		// unsigned obj_count = 0;
		auto it = ssg.geometry.cbegin();
		for (unsigned i = 0; i < ssg.geometry.size(); i++, it++) {
			const SimpleGeometryGroup& sgo = it->second;
			const OBJStaticModel& obj = ssg.geometryObjects[sgo.objectId];

			if (obj.Surfaces.empty())
				continue;

			for (auto& surface : obj.Surfaces) {
				//std::ostringstream obj_pathname;
				//obj_pathname << corona_export_prefix << "object_" << std::setw(3) << std::setfill('0') << obj_count << "_" << sgo.objectName << ".obj";
				//ofstream obj_fout(obj_pathname.str());

				mtl_name = sgo.objectName + "_" + surface.materialName;
				for (auto& c : mtl_name) {
					if (c == ' ')
						c = '_';
				}

				if (written_materials[mtl_name] == false) {
					const SimpleMaterial* mtl = ssg.materials.GetLibraryMaterial(sgo.mtllibName, surface.materialName);
					if (mtl) {
						written_materials[mtl_name] = (int)written_materials.size();
						mtl_fout << "newmtl " << mtl_name << std::endl;

						mtl_fout << "Kd ";
						WriteColor3f(mtl_fout, mtl->Kd);
						mtl_fout << std::endl;
						if (enableKs_) {
							mtl_fout << "Ks ";
							WriteColor3f(mtl_fout, mtl->Ks);
							mtl_fout << std::endl;
						}
						else {
							mtl_fout << "Ks 0.0 0.0 0.0" << std::endl;
						}
						mtl_fout << std::endl;

						//ostream & fout = mtlxml_fout;

						if (!mtl->map_Kd.empty()) {
							written_maps[mtl->map_Kd] = ssg.materials.GetMapPath(mtl->map_Kd);
						}
						if (!mtl->map_bump.empty()) {
							written_maps[mtl->map_bump] = ssg.materials.GetMapPath(mtl->map_bump);
						}

						XmlBeginTag(mtlxml_fout, "materialDefinition", "name", mtl_name, 1) << std::endl;
						XmlBeginTag(mtlxml_fout, "material", "Native", 2) << std::endl;
						if (!mtl->map_Kd.empty()) {
							XmlBeginTag(mtlxml_fout, "diffuse", 3) << std::endl;
							XmlBeginTag(mtlxml_fout, "map", "Reference", 4);
							mtlxml_fout << mtl->map_Kd;
							XmlEndTag(mtlxml_fout, "map") << std::endl;
							XmlEndTag(mtlxml_fout, "diffuse", 3) << std::endl;
						}
						else {
							if (mtl->PBk > 0.0)
								XmlColor3f(mtlxml_fout, "diffuse", Color3f(), 3) << std::endl;
							else
								XmlColor3f(mtlxml_fout, "diffuse", mtl->Kd, 3) << std::endl;
						}

						if (enableKs_) {
							XmlBeginTag(mtlxml_fout, "reflect", 3) << std::endl;
							XmlColor3f(mtlxml_fout, "color", mtl->Ks, 4) << std::endl;

							if (mtl->PBk > 0.0) {
								XmlFloat(mtlxml_fout, "ior", 999, 4) << std::endl;
							}
							else {
								XmlFloat(mtlxml_fout, "ior", mtl->PBior, 4) << std::endl;
							}
							// Corona uses glossiness which is the reverse of roughness
							// A surface is smooth if roughness is 0 or glossiness is 1
							// A surface is rough if roughness is 1 or glossiness is 0
							if (mtl->PBKsm < 0)
								XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f + mtl->PBKsm, 0.0f, 1.0f), 4) << std::endl;
							else
								XmlFloat(mtlxml_fout, "glossiness", clamp(1.0f - mtl->PBKsm, 0.0f, 1.0f), 4) << std::endl;
							XmlEndTag(mtlxml_fout, "reflect", 3) << std::endl;
						}

						XmlEndTag(mtlxml_fout, "material", 2) << std::endl;
						XmlEndTag(mtlxml_fout, "materialDefinition", 1) << std::endl;
					}
				}
			}
		}

		mtl_fout.close();

		for (auto& mapIt : written_maps) {
			XmlCoronaMapTexture(mtlxml_fout, "mapDefinition", mapIt.first, mapIt.first, 1, 2.2f) << std::endl;

#ifdef _WIN32
			// CopyFile(dst, src, bOverWrite?)
			CopyFile(mapIt.second.c_str(), (CoronaJob::exportPathPrefix + mapIt.first).c_str(), TRUE);
#else
			// POSIX version does not exist
			// So we will call system instead
			std::string cp_args = mapIt.second + " " + CoronaJob::exportPathPrefix + mapIt.first;
			execl("/bin/cp", cp_args.c_str());
#endif

			//copy_file(mapIt.second, CoronaJob::exportPathPrefix + mapIt.first);
		}
		XmlEndTag(mtlxml_fout, "mtlLib");
		mtlxml_fout.close();
	}

	void CoronaSceneFile::writeGeometryGroups(std::ostream& ostr, const Fluxions::SimpleSceneGraph& ssg) {
		WriteCache(ssg);
		XmlBeginTag(ostr, "mtllib", 1);
		ostr << "materials_corona.mtl";
		XmlEndTag(ostr, "mtllib");
		ostr << std::endl;
		for (auto& gg : geometryGroups) {
			XmlBeginTag(ostr, "geometryGroup", 1) << std::endl;
			XmlBeginTag(ostr, "object", "file", 2) << std::get<0>(gg);
			XmlEndTag(ostr, "object") << std::endl;
			XmlBeginTag(ostr, "instance", 2) << std::endl;
			XmlBeginTag(ostr, "material", "Reference", 3) << std::get<1>(gg);
			XmlEndTag(ostr, "material", 0) << std::endl;
			XmlMatrix4f(ostr, "transform", std::get<2>(gg), 3) << std::endl;
			XmlEndTag(ostr, "instance", 2) << std::endl;
			XmlEndTag(ostr, "geometryGroup", 1) << std::endl;
		}
	}

	//	/////////////////////////////////////////////////////////////////////
	//	// CoronaJob ////////////////////////////////////////////////////////
	//	/////////////////////////////////////////////////////////////////////
	//
	//	CoronaJob::CoronaJob(const std::string &name, Type jobtype, int arg1, int arg2)
	//	{
	//		scene_name = name;
	//		scene_path = corona_export_prefix + name + ".scn";
	//		output_path_exr = corona_output_prefix + name + ".exr";
	//		output_path_ppm = corona_output_prefix + name + ".ppm";
	//		output_path_png = corona_output_prefix + name + ".png";
	//		conf_path = corona_conf_prefix + name + ".conf";
	//		hq_output_path_exr = corona_output_prefix + name + "_hq.exr";
	//		hq_output_path_ppm = corona_output_prefix + name + "_hq.ppm";
	//		hq_conf_path = corona_conf_prefix + name + "_hq.conf";
	//
	//		type = jobtype;
	//
	//		switch (jobtype)
	//		{
	//		case Type::GEN:
	//			sendLight = SphlSunIndex;
	//			recvLight = arg1;
	//			conf_path = corona_conf_prefix + "sphlgen.conf";
	//			hq_conf_path = corona_conf_prefix + "sphlgen_hq.conf";
	//			break;
	//		case Type::VIZ:
	//			sendLight = arg1;
	//			recvLight = arg2;
	//			conf_path = corona_conf_prefix + "sphlviz.conf";
	//			hq_conf_path = corona_conf_prefix + "sphlviz_hq.conf";
	//			break;
	//		case Type::REF:
	//			conf_path = corona_conf_prefix + "ssphh_ground_truth.conf";
	//			hq_conf_path = corona_conf_prefix + "ssphh_ground_truth_hq.conf";
	//			break;
	//		case Type::REF_CubeMap:
	//			break;
	//		case Type::Sky:
	//			break;
	//		}
	//	}
	//
	//	CoronaJob::~CoronaJob()
	//	{
	//	}
	//
	//	void CoronaJob::Start(CoronaSceneFile &coronaScene, Fluxions::SimpleSceneGraph &ssg)
	//	{
	//		if (state != State::Ready)
	//		{
	//			return;
	//		}
	//
	//		if (!ignoreCache)
	//		{
	//			FilePathInfo fpi(isHQ ? hq_output_path_exr : output_path_exr);
	//			if (fpi.Exists())
	//			{
	//				state = State::Finished;
	//				return;
	//			}
	//		}
	//
	//		std::string tonemapconf = corona_export_prefix + scene_name + "_tonemap.conf";
	//
	//		if (1)
	//		{
	//			float tonemap = ssg.environment.toneMapExposure;
	//			if (type == Type::VIZ)
	//			{
	//				tonemap = 0.0f;
	//			}
	//			if (type == Type::GEN)
	//			{
	//				tonemap = 0.0f;
	//			}
	//			Hf::Log.infofn(__FUNCTION__, "Writing tonemap conf %s", tonemapconf.c_str());
	//
	//			std::ofstream fout(tonemapconf);
	//			fout << "Float colorMap.simpleExposure = " << tonemap << std::endl;
	//			if (type == Type::REF)
	//			{
	//				fout << "Int image.width = " << imageWidth << std::endl;
	//				fout << "Int image.height = " << imageHeight << std::endl;
	//			}
	//			else
	//			{
	//				fout << "Int image.width = " << 6 * 128 << std::endl;
	//				fout << "Int image.height = " << 128 << std::endl;
	//			}
	//			if (!isHQ)
	//			{
	//				fout << "Int shading.maxRayDepth = " << maxRayDepth << std::endl;
	//				fout << "Int progressive.passLimit = " << passLimit << std::endl;
	//			}
	//			fout.close();
	//		}
	//
	//		double t0 = Hf::Log.getSecondsElapsed();
	//		state = State::Running;
	//		bool result = true;
	//		switch (type)
	//		{
	//		case Type::REF:
	//			coronaScene.WriteSCN(scene_path, ssg);
	//			result = Run();
	//			break;
	//		case Type::REF_CubeMap:
	//			coronaScene.WriteCubeMapSCN(scene_path, ssg);
	//			result = Run();
	//			break;
	//		case Type::Sky:
	//			coronaScene.WriteSkySCN(scene_path, ssg);
	//			result = Run();
	//			break;
	//		case Type::GEN:
	//			coronaScene.WriteSphlVizSCN(scene_path, ssg, -1, recvLight);
	//			result = Run();
	//			break;
	//		case Type::VIZ:
	//			coronaScene.WriteSphlVizSCN(scene_path, ssg, sendLight, recvLight);
	//			result = Run();
	//			break;
	//		default:
	//			break;
	//		}
	//		elapsedTime = Hf::Log.getSecondsElapsed() - t0;
	//		state = result ? State::Finished : State::Error;
	//
	//#ifdef _WIN32
	//		DeleteFile(tonemapconf.c_str());
	//		DeleteFile(scene_path.c_str());
	//#else
	//		execl("rm", tonemapconf.c_str());
	//		execl("rm", scene_path.c_str());
	//#endif
	//	}
	//
	//	void CoronaJob::CopySPH(const Sph4f &sph_)
	//	{
	//		if (!IsFinished() && !IsGEN() && !IsVIZ())
	//			return;
	//		memset(sph, 0, sizeof(float) * 484);
	//		for (unsigned i = 0; i < sph_.size(); i++)
	//		{
	//			sph[121 * 0 + i] = sph_.r().getCoefficient(i);
	//			sph[121 * 1 + i] = sph_.g().getCoefficient(i);
	//			sph[121 * 2 + i] = sph_.g().getCoefficient(i);
	//			sph[121 * 3 + i] = sph_.a().getCoefficient(i);
	//		}
	//	}
	//
	//	void CoronaJob::CopySPHToSph4f(Sph4f &sph_)
	//	{
	//		sph_.resize(MaxSphlDegree);
	//		for (unsigned i = 0; i < sph_.size(); i++)
	//		{
	//			sph_.r().setCoefficient(i, sph[121 * 0 + i]);
	//			sph_.g().setCoefficient(i, sph[121 * 1 + i]);
	//			sph_.b().setCoefficient(i, sph[121 * 2 + i]);
	//			sph_.a().setCoefficient(i, sph[121 * 3 + i]);
	//		}
	//	}
	//
	//	std::string CoronaJob::MakeCoronaCommandLine()
	//	{
	//		std::ostringstream cmd;
	//		cmd << "\"C:\\Program Files\\Corona\\Standalone\\Corona.exe\" " << scene_path << " -silent";
	//
	//		if (isHDR)
	//		{
	//			cmd << " -oR " << (isHQ ? hq_output_path_exr : output_path_exr);
	//		}
	//		else
	//		{
	//			cmd << " -o " << (isHQ ? hq_output_path_exr : output_path_exr);
	//		}
	//
	//		if (isHQ)
	//		{
	//			cmd << " -c " << hq_conf_path;
	//		}
	//		else
	//		{
	//			cmd << " -c " << conf_path;
	//		}
	//
	//		cmd << " -c " << corona_export_prefix + scene_name << "_tonemap.conf";
	//
	//		Hf::Log.infofn(__FUNCTION__, "running %s", cmd.str().c_str());
	//
	//		return cmd.str();
	//	}
	//
	//	std::string CoronaJob::MakeConvertCommandLine()
	//	{
	//		std::ostringstream cmd;
	//		if (isHQ)
	//		{
	//			cmd << "magick " << hq_output_path_exr << " -compress none " << hq_output_path_ppm;
	//		}
	//		else
	//		{
	//			cmd << "magick " << output_path_exr << " -compress none " << output_path_ppm;
	//		}
	//		return cmd.str();
	//	}
	//
	//	bool CoronaJob::Run()
	//	{
	//		int retval = 0;
	//		retval = lastCoronaRetval = system(MakeCoronaCommandLine().c_str());
	//		if (retval != 0)
	//		{
	//			Hf::Log.errorfn(__FUNCTION__, "unable to run corona");
	//			return false;
	//		}
	//		//retval = lastConvertRetval = system(MakeConvertCommandLine().c_str());
	//		//if (retval != 0) return false;
	//
	//		{
	//			std::ostringstream cmd;
	//			cmd << "magick " << output_path_exr << " " << output_path_png;
	//			retval = lastConvertRetval = system(cmd.str().c_str());
	//			if (retval != 0)
	//			{
	//				Hf::Log.errorfn(__FUNCTION__, "unable to convert EXR to PNG");
	//				return false;
	//			}
	//		}
	//		{
	//			std::ostringstream cmd;
	//			cmd << "magick " << output_path_png << " -compress none " << output_path_ppm;
	//			retval = lastConvertRetval = system(cmd.str().c_str());
	//			if (retval != 0)
	//			{
	//				Hf::Log.errorfn(__FUNCTION__, "unable to convert PNG to PPM");
	//				return false;
	//			}
	//		}
	//		return true;
	//	}
	//
	//	std::string CoronaJob::ToString() const
	//	{
	//		std::ostringstream ostr;
	//		//ostr << imageWidth << std::endl;
	//		//ostr << imageHeight << std::endl;
	//		//ostr << ignoreCache << std::endl;
	//		//ostr << maxRayDepth << std::endl;
	//		//ostr << passLimit << std::endl;
	//		//ostr << elapsedTime << std::endl;
	//		//ostr << finished << std::endl;
	//		//ostr << working << std::endl;
	//		//ostr << isHQ << std::endl;
	//		//ostr << isHDR << std::endl;
	//		//ostr << sendLight << std::endl;
	//		//ostr << recvLight << std::endl;
	//		//ostr << (type == Type::VIZ ? "VIZ" : (type == Type::GEN ? "GEN" : "REF")) << std::endl;
	//		//ostr << scene_name << std::endl;
	//		//ostr << output_path_ppm << std::endl;
	//
	//		return ostr.str();
	//	}
	//
	//	void CoronaJob::FromString(const std::string &str)
	//	{
	//	}
	//
	//	//////////////////////////////////////////////////////////////////
	//	// Static CoronaJob methods //////////////////////////////////////
	//	//////////////////////////////////////////////////////////////////
	//
	//	std::string CoronaJob::MakeREFName(const std::string &prefix, bool isCubeMap, bool isHDR, bool isHQ, bool ks, int MaxRayDepth, int PassLimit)
	//	{
	//		std::ostringstream ostr;
	//
	//		if (!prefix.empty())
	//			ostr << prefix << "_";
	//		ostr << "REF";
	//		if (isCubeMap)
	//			ostr << "_cubemap";
	//		if (isHDR)
	//			ostr << "_hdr";
	//		if (isHQ)
	//			ostr << "_hq";
	//		else
	//		{
	//			ostr << "_" << std::setw(2) << std::setfill('0') << MaxRayDepth;
	//			ostr << "_" << std::setw(2) << std::setfill('0') << PassLimit;
	//			if (ks)
	//				ostr << "_Ks";
	//		}
	//		return ostr.str();
	//	}
	//
	//	std::string CoronaJob::MakeVIZName(const std::string &prefix, int srcLightIndex, int recvLightIndex, bool isHDR, bool isHQ, bool ks, int MaxRayDepth, int PassLimit)
	//	{
	//		std::ostringstream ostr;
	//		if (!prefix.empty())
	//			ostr << prefix << "_";
	//		ostr << "VIZ" << std::setfill('0') << std::setw(2) << srcLightIndex + 1 << std::setw(2) << recvLightIndex + 1;
	//		if (isHDR)
	//			ostr << "_hdr";
	//		if (isHQ)
	//			ostr << "_hq";
	//		else
	//		{
	//			ostr << "_" << std::setw(2) << std::setfill('0') << MaxRayDepth;
	//			ostr << "_" << std::setw(2) << std::setfill('0') << PassLimit;
	//			if (ks)
	//				ostr << "_Ks";
	//		}
	//		return ostr.str();
	//	}
	//
	//	std::string CoronaJob::MakeGENName(const std::string &prefix, int recvLightIndex, bool isHDR, bool isHQ, bool ks, int MaxRayDepth, int PassLimit)
	//	{
	//		std::ostringstream ostr;
	//		if (!prefix.empty())
	//			ostr << prefix << "_";
	//		ostr << "GEN" << std::setfill('0') << std::setw(2) << recvLightIndex + 1;
	//		if (isHDR)
	//			ostr << "_hdr";
	//		if (isHQ)
	//			ostr << "_hq";
	//		else
	//		{
	//			if (MaxRayDepth >= 0)
	//				ostr << "_" << std::setw(2) << std::setfill('0') << MaxRayDepth;
	//			if (PassLimit >= 0)
	//				ostr << "_" << std::setw(2) << std::setfill('0') << PassLimit;
	//			if (ks)
	//				ostr << "_Ks";
	//		}
	//		return ostr.str();
	//	}
	//
	//	std::string CoronaJob::MakeHIERName(const std::string &prefix, int sendLightIndex, int MaxDegrees)
	//	{
	//		std::ostringstream ostr;
	//		if (!prefix.empty())
	//			ostr << prefix << "_";
	//		ostr << "HIER" << std::setfill('0') << std::setw(2) << sendLightIndex + 1;
	//		if (MaxDegrees >= 0)
	//			ostr << "_" << MaxDegrees;
	//		return ostr.str();
	//	}

} // namespace Fluxions