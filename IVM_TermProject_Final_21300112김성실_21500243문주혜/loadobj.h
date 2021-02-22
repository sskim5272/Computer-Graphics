#pragma once

#include <GL/glew.h>
#include "tiny_obj_loader.h"

bool load_obj(
	const char* filename,
	const char* basedir,
	std::vector<tinyobj::real_t>& vertices_out,
	std::vector<tinyobj::real_t>& normals_out,
	std::vector<size_t>& vertex_map,
	tinyobj::attrib_t& attrib,
	std::vector<tinyobj::shape_t>& shapes,
	std::vector<tinyobj::material_t>& materials,
	tinyobj::real_t scale = 2.0f);

bool load_tex(
	const char* basedir,
	std::vector<tinyobj::real_t>& texcoords_out,
	std::map<std::string, size_t>& texmap_out,
	const std::vector<tinyobj::real_t>& texcoords,
	const std::vector<tinyobj::shape_t>& shapes,
	const std::vector<tinyobj::material_t>& materials,
	GLint min_filter = GL_LINEAR_MIPMAP_LINEAR,
	GLint mag_filter = GL_LINEAR_MIPMAP_LINEAR);