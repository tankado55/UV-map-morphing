#include "mesh.h"
#include <iostream>

#define PI 3.14159265358979323846

Mesh::Mesh(int positions, int uvs, int posCount, int uvCount) : m_PosCount(posCount), glued(true)
{
    int nv = posCount / 3;
    heapPosPtr = reinterpret_cast<glm::vec3 *>(positions);
    heapUvPtr = reinterpret_cast<glm::vec2 *>(uvs);
    
    if (nv * 2 != uvCount || nv * 3 != posCount)
    {
        std::cout << "posCount and uvCount are NOT compatible" << std::endl;
    }

    v.clear();
    for (int i = 0; i < nv; i++)
    {
        Vertex vertex;
        vertex.pos = heapPosPtr[i];
        vertex.uv = heapUvPtr[i];
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
    std::cout << "debug mesh class: position count:" << posCount << std::endl;
    std::cout << "debug mesh class: uv count:" << uvCount << std::endl;
    updateUVScaling();
    updateBB();
    std::cout << "debug mesh class, bounding sphere radius: " << boundingSphere.radius << std::endl;
    setTimingWithV(0.4);
    updateAverageQuaternionRotationAreaWeighted();
    updateRotoTransl();
    updateCopyOf(true);
}

inline bool operator< (const glm::vec<3, float>& el,
                        const glm::vec<3, float>& el2) {
    if (el.x < el2.x) return true;
    if (el2.x < el.x) return false;
    if (el.y < el2.y) return true;
    if (el2.y < el.y) return false;

    return el.z < el2.z;
}

inline bool operator< (const glm::vec<2, float>& el,
                        const glm::vec<2, float>& el2) {
    if (el.x < el2.x) return true;
    if (el2.x < el.x) return false;

    return el.y < el2.y;
}

struct XYZUV{
    glm::vec3 first;
    glm::vec2 second;
    int path;
    XYZUV() { };
    XYZUV(glm::vec3 vec3, glm::vec2 vec2, int p): first(vec3), second(vec2), path(p) {} ;

    friend bool operator< (const XYZUV& el,
                           const XYZUV& el2) {
        if (el.first < el2.first) return true;
        if (el2.first < el.first) return false;
        if (el.second < el2.second) return true;
        if (el2.second < el.second) return false;
        return el.path < el2.path;
    };
};

void Mesh::updateCopyOf(bool pathDependent)
{
    std::map<XYZUV, int> map;
    for (int i = 0; i < v.size(); i++)
    {
        XYZUV key;
        if (pathDependent)
        {
            int faceIndex = i / 3;
            key = XYZUV(v[i].pos, v[i].uv, f[faceIndex].pathVerse);
        }
        else
        {
            key = XYZUV(v[i].pos, v[i].uv, 1);
        }
        if (map.contains(key))
        {
            v[i].copyOf = map[key];
        }
        else
        {
            map[key] = i;
            v[i].copyOf = i;
        }
    }
    for (Face fi : f)
    {
        int i = fi.vi[0];
    }

}

void Mesh::glueTriangles() const
{
    std::vector<glm::vec3> sum(v.size(), glm::vec3(0.0,0.0,0.0));
    std::vector<int> count(v.size(), 0);

    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        sum[j] += heapPosPtr[i];
        count[j]++;
    }
    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        heapPosPtr[i] = sum[j] / float(count[j]);
    }
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

void Mesh::interpolatePerTriangle(int tPercent, bool spitResidual, bool linear, bool shortestPath) const
{
    float t = tPercent / 100.0;
    

    decltype(f[0].three2two) I;
    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        for (int j = 0; j < 3; j++)
        {
            glm::vec3 originV = v[face.vi[j]].pos;
            decltype(f[0].three2two) T;
            if (linear)
            {
                T = mixLinear(I, face.three2two, t);
            } else
            {
                T = mix(I, face.three2two, t, spitResidual, shortestPath);
            }
            glm::vec3 resultV = T.apply(originV);

            // Write in the Heap 
            int heapIndex = i * 3 + j;
            heapPosPtr[heapIndex] = resultV;
        }
    }
    if (glued)
        glueTriangles();
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
        fi.pathVerse = glm::dot(fi.three2two.dqTransf.dualQuaternion.real, glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) < 0.0f;
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

void Mesh::updateAverageQuaternionRotationAreaWeighted()
{
    float areaSum = 0.0;
    DualQuatTransform dqSum;
    for (Face fi : f)
    {
        DualQuatTransform dq = fi.three2two.dqTransf.dualQuaternion;
        float area = ComputeArea(v[fi.vi[0]].pos, v[fi.vi[1]].pos, v[fi.vi[2]].pos);

        dq.dualQuaternion *= area;
        dqSum = mix(dqSum, dq, 0.5, true);
    }
        dqSum = DualQuatTransform(dqSum.dualQuaternion / areaSum);
        dqSum = DualQuatTransform(glm::normalize(dqSum.dualQuaternion));
        initialTranform = dqSum;

    // Apply
    for (int i = 0; i < v.size(); ++i)
    {
        v[i].pos = initialTranform.apply(v[i].pos);
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
