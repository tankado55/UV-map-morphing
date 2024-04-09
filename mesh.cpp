#include "mesh.h"
#include <iostream>

Mesh::Mesh(int positions, int uvs, int posSize, int uvSize):
    debugInt(5)
{
    float* posPtr = reinterpret_cast<float*>(positions);
    float* uvPtr = reinterpret_cast<float*>(uvs);
    heapPosPtr = posPtr;
    heapUvPtr = uvPtr;

    for (int i = 0; i < posSize; i+=3)
    {
        Vertex vertex;
        vertex.pos = glm::vec3(posPtr[i], posPtr[i+1], posPtr[i+2]);
        int j = i/3*2;
        vertex.uv = glm::vec2(uvPtr[j], uvPtr[j+1]);
        v.push_back(vertex);
    }
    debugInt = posSize;
    std::cout << "debug mesh class: " << posSize << std::endl; 
}

void Mesh::interpolate(int t) const
{
    float interpolationValue = (float)t / 100.0;
    
    for (int i = 0; i < v.size(); i++)
    {
        Vertex vertex;
        glm::vec3 targetUV(this->v[i].uv, 0.0);
        /*
        if (toFlip)
        {
            targetUV.x = -targetUV.x + 1.0f;
        }*/

        //float uvScaling = f[faceIndex].uvScaling;
        vertex.pos = glm::mix(
            this->v[i].pos /** bestRotation*/, 
            targetUV /** averageScaling*/,
            interpolationValue
        );
        int arrayIndex = i * 3;
        heapPosPtr[arrayIndex] = vertex.pos.x;
        heapPosPtr[arrayIndex+1] = vertex.pos.y;
        heapPosPtr[arrayIndex+2] = vertex.pos.z;
    }

    
}

#define PI 3.14159265358979323846

void Mesh::buildCylinder()
{
    v.clear();
    int n = 10;
    float radius = 1.0f;
    float height = 2.0f;

    float segmentAngle = 2.0f * PI / n;

    for (int i = 0; i < n; i++)
    {
        float angle = i * segmentAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        Vertex vertex;

        // Top vertices 
        vertex.pos[0] = x;
        vertex.pos[1] = height / 2;
        vertex.pos[2] = z;
        vertex.uv[0] = ((float)i / n);
        vertex.uv[1] = 1.0f;
        v.push_back(vertex);

        // Bottom vertices
        vertex.pos[0] = x;
        vertex.pos[1] = -height / 2;
        vertex.pos[2] = z;
        vertex.uv[0] = ((float)i / n);
        vertex.uv[1] = 0.0f;
        v.push_back(vertex);
    }
    // Duplicate first and last vertices for texture wrapping
    for (int i = 0; i < 2; ++i)
    {
        float angle = i * segmentAngle;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        Vertex vertex;

        // Top vertices 
        vertex.pos[0] = x;
        vertex.pos[1] = height / 2;
        vertex.pos[2] = z;
        vertex.uv[0] = ((float)i / n);
        vertex.uv[1] = 1.0f;
        v.push_back(vertex);

        // Bottom vertices
        vertex.pos[0] = x;
        vertex.pos[1] = -height / 2;
        vertex.pos[2] = z;
        vertex.uv[0] = ((float)i / n);
        vertex.uv[1] = 0.0f;
        v.push_back(vertex);
    }

    for (int i = 0; i < n * 2; i += 2) {
        Face face;
        face.vi[0] = (i + 4);
        face.vi[1] = (i + 2);
        face.vi[2] = (i + 1);
        f.push_back(face);
        
        face.vi[0] = (i + 1);
        face.vi[1] = (i + 3);
        face.vi[2] = (i + 4);
        f.push_back(face);
    }
    averageScaling = 1.0;
    bestRotation = glm::mat3();
}

void Mesh::buildPlane()
{
    float vertices[] = {
        -10.0, -0.5, -10.0, 0.0,1.0,
        -10.0, -0.5, 10.0,  0.0,0.0,
        10.0, -0.5, 10.0,   1.0, 0.0,
        10.0, -0.5, 10.0,   1.0, 0.0,
        10.0, -0.5,-10.0,   1.0, 1.0,
        -10.0, -0.5, -10.0, 0.0, 1.0
    };
    float uvs[] = {
        0.0,1.0,
        0.0,0.0,
        1.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };
    v.clear();
    Vertex vertex;
    for (int i = 0; i < sizeof(vertices) / sizeof(float); i+=5)
    {
        vertex.pos.x = vertices[i];
        vertex.pos.y = vertices[i + 1];
        vertex.pos.z = vertices[i + 2];
        vertex.uv.x =  vertices[i + 3];
        vertex.uv.y =  vertices[i + 4];

        v.push_back(vertex);
    }
        Face face;
        face.vi[0] = 1;
        face.vi[1] = 2;
        face.vi[2] = 3;
        f.push_back(face);
        face.vi[0] = 4;
        face.vi[1] = 5;
        face.vi[2] = 6;
        f.push_back(face);
}

void Mesh::updateBB()
{
    glm::vec3 minExtents = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
    glm::vec3 maxExtents = { -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max() };

    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        for (int j = 0; j < 3; j++)
        {
            Vertex vertex = v[face.vi[j]];
            minExtents.x = std::min(minExtents.x, vertex.pos.x);
            minExtents.y = std::min(minExtents.y, vertex.pos.y);
            minExtents.z = std::min(minExtents.z, vertex.pos.z);
            maxExtents.x = std::max(maxExtents.x, vertex.pos.x);
            maxExtents.y = std::max(maxExtents.y, vertex.pos.y);
            maxExtents.z = std::max(maxExtents.z, vertex.pos.z);
        }
    }
    boundingSphere.center = {
        (minExtents.x + maxExtents.x) / 2,
        (minExtents.y + maxExtents.y) / 2,
        (minExtents.z + maxExtents.z) / 2
    };

    float maxRadiusSquared = 0.0f;
    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        for (int j = 0; j < 3; j++)
        {
            Vertex vertex = v[face.vi[j]];
            float distanceSquared = pow(vertex.pos.x - boundingSphere.center.x, 2) + pow(vertex.pos.y - boundingSphere.center.y, 2) + pow(vertex.pos.z - boundingSphere.center.z, 2);
            maxRadiusSquared = std::max(maxRadiusSquared, distanceSquared);
        }
    }

    boundingSphere.radius = sqrt(maxRadiusSquared);
}

