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
#include "linearTransform.h"
#include "quatTransform.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	float tStart;
	float tEnd;
};

struct Face
{
	int vi[3];
	float uvScaling;
	glm::vec3 centroid3D;
	glm::vec3 centroid2D;
	float area;
	LinearTransform three2two;
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
	int m_PosSize;
	float* heapPosPtr;
	float* heapUvPtr;

	Mesh();
	Mesh(int positions, int uvs, int posSize, int uvSize);
	void interpolate(int t) const;
	void interpolateUsingMat(int t) const;
	void interpolateUsingQuat(int t) const;
	void buildCylinder();
	void buildPlane();
	void updateBB();
	void updateToFlipBool();
	void updateUVScaling();
	void updateFacesArea();
	void updateRotoTranslMat();
	int getPosSize() const {return m_PosSize;}
	std::vector<float> getBBCenter() const {return {boundingSphere.center.x, boundingSphere.center.y, boundingSphere.center.z};}
	void setTimingWithVertexIndex(float flightTime);
	void setTimingInsideOut(float flightTime);
	void setTimingWithUVdir(float flightTime, glm::vec2 dirUV);
	void setTimingWithU(float flightTime) {setTimingWithUVdir(flightTime, glm::vec2(1.0, 0.0));}
	void setTimingWithV(float flightTime) {setTimingWithUVdir(flightTime, glm::vec2(0.0, 1.0));}
};

// Binding code
EMSCRIPTEN_BINDINGS(my_class_example) {
  emscripten::class_<Mesh>("Mesh")
    .constructor<int, int, int, int>()
	.property("posSize", &Mesh::getPosSize)
    .function("interpolate", &Mesh::interpolate)
    .function("interpolateUsingMat", &Mesh::interpolateUsingMat)
    .function("interpolateUsingQuat", &Mesh::interpolateUsingQuat)
	.property("boundingSphere", &Mesh::getBBCenter)
    ;
}