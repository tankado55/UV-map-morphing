#pragma once

#include <map>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <emscripten/bind.h>
#include "linearTransform.h"
#include "dualQuatTransform.h"
#include "smartTransform.h"
#include <unordered_set>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	float tStart = 0.0;
	float tEnd = 1.0;
	int copyOf;
	int islandId = 0;
	int islandRank = 0;
	int pathVerse = 1;
	float area3D = 0.0;
	float area2D = 0.0;

};

struct Face
{
	int vi[3];
	SmartTransform three2two;
	int pathVerse = 1; // Individual choice for the shortest path for this triangle
	std::unordered_set<int> neighbors;
};

struct BoundingSphere
{
	glm::vec3 center;
	float radius;
};

struct Mesh
{
	std::vector<Vertex> v;
	int uniqueVerticesCount;
	std::vector<Face> f;
	glm::vec3 centroid3D;
	glm::vec3 centroid2D;
	float averageScaling;
	glm::mat3 bestRotation;
	BoundingSphere boundingSphere;
	bool toFlip = false;
	int m_PosCount;
	glm::vec3* heapPosPtr;
	glm::vec2* heapUvPtr;
	float* pathVersePtr;
	bool glued;
	bool gluedWeighted;
	glm::dualquat initialTranform;
	float gluingThreshold = 0.0;
	std::vector<std::vector<glm::vec3>> bakedVertices;

	Mesh();
	Mesh(int positions, int uvs, int pathVerse, int posSize, int uvSize);
	void interpolate(int t) const;
	void interpolatePerTriangle(int tPercent, bool spitResidual, bool linear, bool shortestPath);
	void buildCylinder();
	void updateBB();
	void updateToFlipBool();
	void updateUVScaling();
	void updateRotoTransl();
	int getPosSize() const {return m_PosCount;}
	std::vector<float> getBBCenter() const {return {boundingSphere.center.x, boundingSphere.center.y, boundingSphere.center.z};}
	void setTimingWithVertexIndex(float flightTime);
	void setTimingInsideOut(float flightTime);
	void setTimingWithUVdir(float flightTime, glm::vec2 dirUV);
	void setTimingWithU(float flightTime) {setTimingWithUVdir(flightTime, glm::vec2(1.0, 0.0));}
	void setTimingWithV(float flightTime) {setTimingWithUVdir(flightTime, glm::vec2(0.0, 1.0));}
	void resetTiming();
	void updateCopyOf(bool pathDependent);
	void glueTriangles() const;
	void glueTrianglesWeighted();
	std::vector<glm::vec3> glueTrianglesWeightedRet();
	void glueTriangleArap();
	void updateAverageQuaternionRotationAreaWeighted();
	void updatePathVerse(int verse);
	void updatePathVersePerIsland();
	int findIsland(int i) const;
	void unionIsland(int x, int y);
	void initIsland();
	void updateIsland();
	int countIslands();
	void updateAverageTimingPerFace();
	void updateAverageTimingPerIsland();
	void updateFacesNeighbors();
	bool uniformQuaternionSigns();
	void updateAreaPerVertex();
	void updateCopyOfUsingThreshold(bool pathDependent);
	void bake(int sampleCount, bool splitResidual, bool linear);
	std::vector<glm::vec3> interpolateConst(int tPercent, bool spitResidual, bool linear) const;
	void applyBaked(int t);
};

static float sigmoid(float t);

// Binding code
EMSCRIPTEN_BINDINGS(my_class_example) {
  emscripten::class_<Mesh>("Mesh")
    .constructor<int, int, int, int, int>()
	.property("posSize", &Mesh::getPosSize)
    .function("interpolatePerTriangle", &Mesh::interpolatePerTriangle)
    .function("applyBaked", &Mesh::applyBaked)
    .function("updateCopyOf", &Mesh::updateCopyOf)
    .function("updatePathVerse", &Mesh::updatePathVerse)
    .function("updatePathVersePerIsland", &Mesh::updatePathVersePerIsland)
    .function("setTimingWithU", &Mesh::setTimingWithU)
    .function("setTimingWithV", &Mesh::setTimingWithV)
    .function("setTimingInsideOut", &Mesh::setTimingInsideOut)
    .function("resetTiming", &Mesh::resetTiming)
    .function("bake", &Mesh::bake)
	.property("boundingSphere", &Mesh::getBBCenter)
	.property("glued", &Mesh::glued)
	.property("gluedWeighted", &Mesh::gluedWeighted)
	.property("gluingThreshold", &Mesh::gluingThreshold)
    ;
}