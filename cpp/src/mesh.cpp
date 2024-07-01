#include "mesh.h"
#include <iostream>

#define PI 3.14159265358979323846

Mesh::Mesh(int positions, int uvs, int posSize, int uvSize) : m_PosSize(posSize)
{
    float *posPtr = reinterpret_cast<float *>(positions);
    float *uvPtr = reinterpret_cast<float *>(uvs);
    heapPosPtr = posPtr;
    heapUvPtr = uvPtr;

    if (posSize != uvSize)
    {
        std::cout << "posSize and uvSize are NOT equal" << std::endl;
    }

    for (int i = 0; i < posSize; i += 3)
    {
        Vertex vertex;
        vertex.pos = glm::vec3(posPtr[i], posPtr[i + 1], posPtr[i + 2]);
        int j = i / 3 * 2;
        vertex.uv = glm::vec2(uvPtr[j], uvPtr[j + 1]);
        v.push_back(vertex);
    }
    for (int i = 0; i < v.size(); i += 3)
    {
        Face face;
        face.vi[0] = i;
        face.vi[1] = i + 1;
        face.vi[2] = i + 2;
        //face.three2two = glm::mat4(1.0);
        f.push_back(face);
    }
    std::cout << "debug mesh class: position size:" << posSize << std::endl;
    updateUVScaling();
    updateBB();
    std::cout << "debug mesh class, bounding sphere radius: " << boundingSphere.radius << std::endl;
    setTimingWithV(0.4);
    updateRotoTransl();
}

static float ComputeArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    float l1 = glm::length(p1 - p2);
    float l2 = glm::length(p2 - p3);
    float l3 = glm::length(p3 - p1);
    float s = (l1 + l2 + l3) / 2.0f;
    return sqrt(s * (s - l1) * (s - l2) * (s - l3));
}

glm::vec3 uv2xzy(glm::vec2 v)
{
    return glm::vec3(v.x, 0.0, v.y);
}

static float sigmoid(float t) // ease in ease out
{
    return (glm::sin(t * PI - PI / 2) + 1) / 2;
}

void Mesh::interpolate(int tPercent) const //TODO: refector
{
    float t = tPercent / 100.0;

    for (int i = 0; i < v.size(); i++)
    {
        Vertex vertex;
        glm::vec3 targetUV = uv2xzy(v[i].uv);
        /*
        if (toFlip)
        {
            targetUV.x = -targetUV.x + 1.0f;
        }*/

        // float uvScaling = f[faceIndex].uvScaling;
        glm::mat4 toCenterMat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5, -0.5, -0.5));
        targetUV = glm::vec3(toCenterMat * glm::vec4(targetUV, 1.0));
        targetUV = targetUV * averageScaling;

        float interpolationValue = (t - v[i].tStart) / (v[i].tEnd - v[i].tStart);
        interpolationValue = glm::clamp(interpolationValue, 0.0f, 1.0f);
        interpolationValue = sigmoid(interpolationValue);

        vertex.pos = glm::mix(
            this->v[i].pos /** bestRotation*/,
            targetUV,
            interpolationValue);
        int arrayIndex = i * 3;
        heapPosPtr[arrayIndex] = vertex.pos.x;
        heapPosPtr[arrayIndex + 1] = vertex.pos.y;
        heapPosPtr[arrayIndex + 2] = vertex.pos.z;
    }
}

void Mesh::interpolatePerTriangle(int tPercent) const
{
    float t = tPercent / 100.0;

    decltype(f[0].three2two) I;
    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        for (int j = 0; j < 3; j++)
        {
            glm::vec3 originV = v[face.vi[j]].pos;
            auto T = mix(I, face.three2two, t);
            glm::vec3 targetV = T.apply(originV);            
            //glm::vec3 targetV = face.three2two.apply(originV);            
            
            int heapIndex = (i * 9) + (j * 3);
            heapPosPtr[heapIndex] = targetV.x;
            heapPosPtr[heapIndex + 1] = targetV.y;
            heapPosPtr[heapIndex + 2] = targetV.z;
        }
    }
}

void Mesh::updateRotoTransl()
{
    // traslazione
    // con baricentro
    glm::mat4 toCenterMat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5, -0.5, -0.5));
    for (Face &fi : f)
    {
        glm::vec3 a3 = v[fi.vi[0]].pos;
        glm::vec3 b3 = v[fi.vi[1]].pos;
        glm::vec3 c3 = v[fi.vi[2]].pos;

        glm::vec2 a2 = v[fi.vi[0]].uv;
        a2 = glm::vec2(toCenterMat * glm::vec4(a2, 0.0, 1.0));
        a2 = a2 * averageScaling;

        glm::vec2 b2 = v[fi.vi[1]].uv;
        b2 = glm::vec2(toCenterMat * glm::vec4(b2, 0.0, 1.0));
        b2 = b2 * averageScaling;

        glm::vec2 c2 = v[fi.vi[2]].uv;
        c2 = glm::vec2(toCenterMat * glm::vec4(c2, 0.0, 1.0));
        c2 = c2 * averageScaling;
        
        fi.three2two.fromTo(a3, b3, c3, a2, b2, c2);
    }
}

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

    for (int i = 0; i < n * 2; i += 2)
    {
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
        -10.0, -0.5, -10.0, 0.0, 1.0,
        -10.0, -0.5, 10.0, 0.0, 0.0,
        10.0, -0.5, 10.0, 1.0, 0.0,
        10.0, -0.5, 10.0, 1.0, 0.0,
        10.0, -0.5, -10.0, 1.0, 1.0,
        -10.0, -0.5, -10.0, 0.0, 1.0};
    float uvs[] = {
        0.0, 1.0,
        0.0, 0.0,
        1.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0};
    v.clear();
    Vertex vertex;
    for (int i = 0; i < sizeof(vertices) / sizeof(float); i += 5)
    {
        vertex.pos.x = vertices[i];
        vertex.pos.y = vertices[i + 1];
        vertex.pos.z = vertices[i + 2];
        vertex.uv.x = vertices[i + 3];
        vertex.uv.y = vertices[i + 4];

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
    glm::vec3 minExtents = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    glm::vec3 maxExtents = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};

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
        (minExtents.z + maxExtents.z) / 2};

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

void Mesh::updateUVScaling()
{
    float scalingSum = 0.0;
    for (unsigned int i = 0; i < v.size(); i += 3)
    {
        // scaling UV
        glm::vec3 v1 = v[i].pos;
        glm::vec3 v2 = v[i + 1].pos;
        glm::vec3 v3 = v[i + 2].pos;

        float areaMesh = ComputeArea(v1, v2, v3);

        v1 = glm::vec3(v[i].uv, 0.0f);
        v2 = glm::vec3(v[i + 1].uv, 0.0f);
        v3 = glm::vec3(v[i + 2].uv, 0.0f);

        float areaUV = ComputeArea(v1, v2, v3);
        if (areaUV > 0)
        {
            float ratio = sqrt(areaMesh / areaUV);
            scalingSum += ratio;
        }
    }

    if (scalingSum != 0)
        averageScaling = scalingSum / (v.size() / 3);
    else
        averageScaling = 1.0;
}

void Mesh::setTimingWithVertexIndex(float k)
{
    int n = v.size();
    for (int i = 0; i < n; i++)
    {
        v[i].tStart = (1 - k) * i / (n - 1);
        v[i].tEnd = v[i].tStart + k;
    }
}
void Mesh::setTimingInsideOut(float k)
{
    for (Vertex &vi : v)
    {
        float d = glm::length(vi.pos - boundingSphere.center);
        d = d / boundingSphere.radius;
        vi.tStart = (1 - k) * (1 - d * d * d);
        vi.tEnd = vi.tStart + k;
    }
}

void Mesh::setTimingWithUVdir(float flightTime, glm::vec2 dirUV)
{
    dirUV = glm::normalize(dirUV);
    float mind = 99999;
    float maxd = -99999;
    for (Vertex &vi : v)
    {
        float d = glm::dot(vi.uv, dirUV);
        mind = std::min(mind, d);
        maxd = std::max(maxd, d);
    }

    for (Vertex &vi : v)
    {
        float d = glm::dot(vi.uv, dirUV);
        d = (d - mind) / (maxd - mind);
        vi.tStart = (1 - flightTime) * d;
        vi.tEnd = vi.tStart + flightTime;
    }
}

/*static glm::mat3 updateInitRotation()
{
    glm::mat3 result = glm::mat3();
    for (int i = 0; i < mesh.f.size(); i++)
    {
        Face face = mesh.f[i];
        glm::vec3 vi = glm::vec3(0.0);
        glm::vec3 wi = glm::vec3(0.0);

        vi = mesh.v[face.vi[0]].pos - mesh.centroid3D;
        wi = glm::vec3(mesh.v[face.vi[0]].uv, 0.0) - mesh.centroid2D;
        glm::mat3 outer = glm::outerProduct(vi, wi);
        result += outer;
        vi = mesh.v[face.vi[1]].pos - mesh.centroid3D;
        wi = glm::vec3(mesh.v[face.vi[1]].uv, 0.0) - mesh.centroid2D;
        outer = glm::outerProduct(vi, wi);
        result += outer;
        vi = mesh.v[face.vi[2]].pos - mesh.centroid3D;
        wi = glm::vec3(mesh.v[face.vi[2]].uv, 0.0) - mesh.centroid2D;
        outer = glm::outerProduct(vi, wi);
        result += outer;
    }
    result = result / static_cast<float>(mesh.f.size() * 3);

    // SVD
    Eigen::MatrixXd A = glmToEigen(result);
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix3d R = svd.matrixU() * svd.matrixV().transpose();

    if (R.determinant() < 0) {
        R *= -1;
    }
    result = eigenToGlm(R);

    return result;
}*/
