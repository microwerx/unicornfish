#include "unicornfish_pch.hpp"
#include <fluxions_simple_geometry_mesh.hpp>
#include <unicornfish_corona_database.hpp>

template class std::vector<Fluxions::SimpleGeometryMesh::Vertex>;
template class std::vector<Fluxions::SimpleGeometryMesh::Surface>;
template class std::map<std::string, Fluxions::SimpleGeometryMesh>;

namespace Uf {
	CoronaDatabase::CoronaDatabase() {}

	CoronaDatabase::~CoronaDatabase() {}

	void CoronaDatabase::loadOBJ(const std::string& objname, const std::string& filename) {
		models[objname].loadOBJ(filename);
	}

	//void CoronaDatabase::BuildBuffers() {
	//	renderer.reset();
	//	std::map<std::string, Fluxions::SimpleGeometryMesh>::iterator modelIt;
	//	for (modelIt = models.begin(); modelIt != models.end(); modelIt++) {
	//		modelIt->second.Render(renderer);
	//	}
	//	renderer.BuildBuffers();
	//}

	//void CoronaDatabase::Render() {
	//	renderer.Render();
	//}

	//void CoronaDatabase::RenderZOnly() {
	//	renderer.RenderZOnly();
	//}

} // namespace Uf
