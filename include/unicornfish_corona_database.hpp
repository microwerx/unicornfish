#ifndef UNICORNFISH_CORONA_DATABASE_HPP
#define UNICORNFISH_CORONA_DATABASE_HPP

#include <unicornfish_base.hpp>
#include <fluxions_stdcxx.hpp>
#include <fluxions_gte.hpp>

namespace Uf {
	class CoronaDatabase {
	public:
		//Fluxions::SimpleRenderer_GLuint renderer;
		std::map<std::string, Fluxions::SimpleGeometryMesh> models;

		CoronaDatabase();
		~CoronaDatabase();

		void loadOBJ(const std::string& objname, const std::string& filename);
		//void BuildBuffers();
		//void Render();
		//void RenderZOnly();
	};
} // namespace Fluxions

#endif
