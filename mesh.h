#pragma once

#include "cpp_src/glm-1.0.0-light/glm/vec3.hpp"
#include "cpp_src/glm-1.0.0-light/glm/vec2.hpp"
#include "cpp_src/glm-1.0.0-light/glm/mat3x3.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtc/matrix_transform.hpp"
#include "cpp_src/glm-1.0.0-light/glm/glm.hpp"
#include "cpp_src/glm-1.0.0-light/glm/gtc/type_ptr.hpp"
#include <vector>
#include <string>
#include <emscripten/bind.h>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Face
{
	int vi[3];
	float uvScaling;
	glm::vec3 centroid3D;
	glm::vec3 centroid2D;
};

struct BoundingSphere
{
	glm::vec3 center;
	float radius;
};

struct Mesh
{
	std::vector<Vertex> v;
	std::vector<Face> f;
	glm::vec3 centroid3D;
	glm::vec3 centroid2D;
	float averageScaling;
	glm::mat3 bestRotation;
	BoundingSphere boundingSphere;
	bool toFlip = false;
	int debugInt;
	float* heapPosPtr;
	float* heapUvPtr;

	Mesh();
	Mesh(int positions, int uvs, int posSize, int uvSize);
	void interpolate(int t) const;
	void buildCylinder();
	void buildPlane();
	void updateBB();
	void updateToFlipBool();
	int getDebugInt() const {return debugInt;}
};

// Binding code
EMSCRIPTEN_BINDINGS(my_class_example) {
  emscripten::class_<Mesh>("Mesh")
    .constructor<int, int, int, int>()
	.property("debugInt", &Mesh::getDebugInt)
    .function("interpolate", &Mesh::interpolate)
    ;
}