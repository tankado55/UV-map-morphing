#include "mesh.h"
#include <iostream>

#define PI 3.14159265358979323846

Mesh::Mesh(int positions, int uvs, int pathVerse, int posCount, int uvCount) : m_PosCount(posCount), glued(true)
{
    int verticesCount = posCount / 3;
    heapPosPtr = reinterpret_cast<glm::vec3 *>(positions);
    heapUvPtr = reinterpret_cast<glm::vec2 *>(uvs);
    pathVersePtr = reinterpret_cast<float *>(pathVerse);

    if (verticesCount * 2 != uvCount || verticesCount * 3 != posCount)
    {
        std::cout << "posCount and uvCount are NOT compatible" << std::endl;
    }

    v.clear();
    for (int i = 0; i < verticesCount; i++)
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
        f.push_back(face);
    }
    std::cout << "debug mesh class cpp, position count:" << posCount << std::endl;
    std::cout << "debug mesh class cpp, uv count:" << uvCount << std::endl;
    updateUVScaling();
    updateBB();
    std::cout << "debug mesh class cpp, bounding sphere radius: " << boundingSphere.radius << std::endl;
    updateRotoTransl();
    updateAverageQuaternionRotationAreaWeighted(); // Initial rotation
    updateRotoTransl();
    updateIsland();
    updateFacesNeighbors();
    updateAreaPerVertex();
    if (!uniformQuaternionSigns())
    {
        std::cout << "uniformQuaternionSigns False" << std::endl;
    }
    // bake(101, true, false);
}

void Mesh::bake(int sampleCount, bool splitResidual, bool linear)
{
    std::cout << "Baking ..." << std::endl;
    bakedVertices.clear();
    bakedVertices.resize(sampleCount);
    for (int i = 0; i < sampleCount; i++)
    {
        std::vector<glm::vec3> interpolations = interpolateConst(i, splitResidual, linear);
        bakedVertices[i] = std::move(interpolations);
    }
    // for sample count
    // compute copyOf per sample
    // compute gluing using copyOf per sample
    std::cout << "end of baking: " << bakedVertices.size() << std::endl;
}

std::vector<glm::vec3> Mesh::interpolateConst(int tPercent, bool splitResidual, bool linear) const
{
    std::vector<glm::vec3> result;
    result.reserve(f.size() * 3);

    float t = tPercent / 100.0;

    decltype(f[0].three2two) I;
    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        for (int j = 0; j < 3; j++)
        {
            float interpolationValue = (t - v[face.vi[j]].tStart) / (v[face.vi[j]].tEnd - v[face.vi[j]].tStart);
            interpolationValue = glm::clamp(interpolationValue, 0.0f, 1.0f);
            interpolationValue = sigmoid(interpolationValue);
            decltype(f[0].three2two) T;
            if (linear)
            {
                T = mixLinear(I, face.three2two, interpolationValue);
            }
            else
            {
                T = mix(I, face.three2two, interpolationValue, splitResidual, face.pathVerse);
            }
            glm::vec3 originV = v[face.vi[j]].pos;
            glm::vec3 resultV = T.apply(originV);

            result.push_back(resultV);
        }
    }
    return result;
}

void Mesh::interpolatePerTriangle(int tPercent, bool splitResidual, bool linear)
{
    std::vector<glm::vec3> newPositions = interpolateConst(tPercent, splitResidual, linear);

    std::memcpy(heapPosPtr, newPositions.data(), newPositions.size() * sizeof(newPositions[0]));
}

void Mesh::applyBaked(int t)
{
    if (t >= bakedVertices.size())
    {
        std::cout << "Missing baked data" << std::endl;
        return;
    }
    std::memcpy(heapPosPtr, bakedVertices[t].data(), bakedVertices[t].size() * sizeof(bakedVertices[t][0]));
}

void Mesh::setGluingThreshold(float threshold)
{
    gluingThreshold = threshold * boundingSphere.radius * 2;
}

inline bool operator<(const glm::vec<3, float> &el,
                      const glm::vec<3, float> &el2)
{
    if (el.x < el2.x)
        return true;
    if (el2.x < el.x)
        return false;
    if (el.y < el2.y)
        return true;
    if (el2.y < el.y)
        return false;

    return el.z < el2.z;
}

inline bool operator<(const glm::vec<2, float> &el,
                      const glm::vec<2, float> &el2)
{
    if (el.x < el2.x)
        return true;
    if (el2.x < el.x)
        return false;

    return el.y < el2.y;
}

struct XYZUV
{
    glm::vec3 first;
    glm::vec2 second;
    int path;
    XYZUV() {};
    XYZUV(glm::vec3 vec3, glm::vec2 vec2, int p) : first(vec3), second(vec2), path(p) {};

    friend bool operator<(const XYZUV &el,
                          const XYZUV &el2)
    {
        if (el.first < el2.first)
            return true;
        if (el2.first < el.first)
            return false;
        if (el.second < el2.second)
            return true;
        if (el2.second < el.second)
            return false;
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
}

void Mesh::updateCopyOfUsingThreshold(bool pathDependent)
{
    std::cout << "updating CopyOf..." << std::endl;
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
            if (glm::length(heapPosPtr[i] - heapPosPtr[map[key]]) < gluingThreshold)
            {
                v[i].copyOf = map[key];
            }
            else
            {
                map[key] = i;
                v[i].copyOf = i;
            }
        }
        else
        {
            map[key] = i;
            v[i].copyOf = i;
        }
    }
}

void Mesh::copyOfUsingThreshold(bool pathDependent, const std::vector<glm::vec3> &vertices, std::vector<int> &outCopyOf)
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
            if (glm::length(vertices[i] - vertices[map[key]]) < gluingThreshold)
            {
                outCopyOf[i] = map[key];
            }
            else
            {
                map[key] = i;
                outCopyOf[i] = i;
            }
        }
        else
        {
            map[key] = i;
            outCopyOf[i] = i;
        }
    }
}

static float ComputeArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    float l1 = glm::length(p1 - p2);
    float l2 = glm::length(p2 - p3);
    float l3 = glm::length(p3 - p1);
    float s = (l1 + l2 + l3) / 2.0f;
    float result = sqrt(s * (s - l1) * (s - l2) * (s - l3));
    if (result == 0)
    {
        std::cout << "Area is 0\n";
    }
    return sqrt(s * (s - l1) * (s - l2) * (s - l3));
}

void Mesh::updateAreaPerVertex()
{
    for (Face fi : f)
    {
        float area3D = 0.5 * length(glm::cross(v[fi.vi[0]].pos - v[fi.vi[1]].pos, v[fi.vi[0]].pos - v[fi.vi[2]].pos));
        float area2D = 0.5 * length(glm::cross(glm::vec3(v[fi.vi[0]].uv, 0.0) - glm::vec3(v[fi.vi[1]].uv, 0.0), glm::vec3(v[fi.vi[0]].uv, 0.0) - glm::vec3(v[fi.vi[2]].uv, 0.0)));

        for (int i : fi.vi)
        {
            v[i].area3D += area3D;
            v[i].area2D += area2D;
        }
    }
}

void Mesh::glueTriangles() const
{
    std::vector<glm::vec3> sum(v.size(), glm::vec3(0.0, 0.0, 0.0));
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

void Mesh::glueTrianglesWeighted()
{
    std::vector<glm::vec3> sum(v.size(), glm::vec3(0.0, 0.0, 0.0));
    std::vector<float> areaSum(v.size(), 0.0);

    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        sum[j] += heapPosPtr[i] * ((v[i].area3D + v[i].area2D) / 2.0f);
        areaSum[j] += (v[i].area3D + v[i].area2D) / 2.0f;
    }

    // Apply
    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        heapPosPtr[i] = sum[j] / areaSum[j];
    }
}

std::vector<glm::vec3> Mesh::glueTrianglesWeightedRet()
{
    std::vector<glm::vec3> sum(v.size(), glm::vec3(0.0, 0.0, 0.0));
    std::vector<float> areaSum(v.size(), 0.0);
    std::vector<glm::vec3> result(v.size());

    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        sum[j] += heapPosPtr[i] * ((v[i].area3D + v[i].area2D) / 2.0f);
        areaSum[j] += (v[i].area3D + v[i].area2D) / 2.0f;
    }

    // Result
    for (int i = 0; i < v.size(); ++i)
    {
        int j = v[i].copyOf;
        result[i] = sum[j] / areaSum[j];
    }

    return result;
}

void Mesh::glueTriangleArapNaive()
{
    std::vector<glm::vec3> deformed = glueTrianglesWeightedRet();
    for (int i = 0; i < f.size(); i++)
    {
        int j = i * 3;
        LinearTransform t;
        t.fromTo(heapPosPtr[j], heapPosPtr[j + 1], heapPosPtr[j + 2], deformed[j], deformed[j + 1], deformed[j + 2]);
        glm::vec3 transl = deformed[j] - heapPosPtr[j] + deformed[j + 1] - heapPosPtr[j + 1] + deformed[j + 2] - heapPosPtr[j + 2];
        // heapPosPtr[j] = t.apply(heapPosPtr[j]);
        // heapPosPtr[j+1] = t.apply(heapPosPtr[j+1]);
        // heapPosPtr[j+2] = t.apply(heapPosPtr[j+2]);
        heapPosPtr[j] = heapPosPtr[j] + transl;
        heapPosPtr[j + 1] = heapPosPtr[j + 1] + transl;
        heapPosPtr[j + 2] = heapPosPtr[j + 2] + transl;
        // heapPosPtr[j] = heapPosPtr[j] + glm::vec3(t.M[3]);
        // heapPosPtr[j+1] = heapPosPtr[j+1] + glm::vec3(t.M[3]);
        // heapPosPtr[j+2] = heapPosPtr[j+2] + glm::vec3(t.M[3]);
        // heapPosPtr[j] = deformed[j];
        // heapPosPtr[j+1] = deformed[j+1];
        // heapPosPtr[j+2] = deformed[j+2];
    }
}

void Mesh::glueTriangleArap()
{
    std::cout << "starting System Solving..." << std::endl;
    // Build System
    Eigen::SparseMatrix<double> A;
    Eigen::VectorXd b;
    A.resize(v.size() * 3, v.size() * 3);
    b.resize(v.size() * 3);
    //b.setZero();

    std::vector<Eigen::Triplet<double>> tripletList;

    for (int i = 0; i < v.size(); i++)
    {
        int j = i / 3; // face
        if (v[i].copyOf != i)
        {
            for (int k = 0; k < 3; k++)
            {
                tripletList.push_back({(i * 3) + k, (i * 3) + k, 1.0});
                tripletList.push_back({(i * 3) + k, (v[i].copyOf * 3) + k, -1.0});
            }
            glm::vec3 tempVec = heapPosPtr[v[i].copyOf] - heapPosPtr[i];
            b((i * 3) + 0) = tempVec.x;
            b((i * 3) + 1) = tempVec.y;
            b((i * 3) + 2) = tempVec.z;
            // glm::vec3 tempVec1 = heapPosPtr[i] - v[i].pos;
            // glm::vec3 tempVec2 = heapPosPtr[v[i].copyOf] - v[v[i].copyOf].pos;
            // glm::vec3 diff = tempVec1 - tempVec2;
            // b((i * 3) + 0) = diff.x;
            // b((i * 3) + 1) = diff.y;
            // b((i * 3) + 2) = diff.z;
        }
        else
        {
           
        }
    }
    A.setFromTriplets(tripletList.begin(), tripletList.end());

    // Eigen::MatrixXd denseA = A.toDense(); // Convert to dense if A is sparse
    // double det = denseA.determinant();

    // if (det == 0)
    // {
    //     std::cerr << "The matrix is singular (determinant is zero)." << std::endl;
    // }
    // else
    // {
    //     std::cout << "The matrix is not singular (determinant is non-zero)." << std::endl;
    // }

    // Solve
    // Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver;
    // Eigen::SparseLU<Eigen::SparseMatrix<double>> solver; I can't, it is singular
    Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> solver; // out of bound memory

    // solver.analyzePattern(A);
    // solver.factorize(A);

    // if (solver.info() != Eigen::Success)
    // {
    //     std::cerr << "Factorization failed: " << std::endl;
    //     // Handle the error (e.g., return, throw an exception, etc.)
    // }
    solver.compute(A);
    Eigen::VectorXd x = solver.solve(b);
    if (solver.info() != Eigen::Success)
    {
        std::cerr << "Solving failed!" << std::endl;
        // return -1;
    }

    // Apply
    for (int i = 0; i < v.size(); i++)
    {
        Eigen::Vector3d translation = x.segment<3>(i * 3);
        heapPosPtr[i] = heapPosPtr[i] + glm::vec3(translation.x(), translation.y(), translation.z());
    }
    std::cout << "System Done." << std::endl;

    // here apply the local, rotation?
    // from original to deformed but
    for (Face &fi : f)
    {
        glm::vec3 a3 = v[fi.vi[0]].pos;
        glm::vec3 b3 = v[fi.vi[1]].pos;
        glm::vec3 c3 = v[fi.vi[2]].pos;

        glm::vec3 a2 = heapPosPtr[fi.vi[0]];
        a2 = a2 * averageScaling;

        glm::vec2 b2 = v[fi.vi[1]].uv;
        b2 = glm::vec2(toCenterMat * glm::vec4(b2, 0.0, 1.0));
        b2 = b2 * averageScaling;

        glm::vec2 c2 = v[fi.vi[2]].uv;
        c2 = glm::vec2(toCenterMat * glm::vec4(c2, 0.0, 1.0));
        c2 = c2 * averageScaling;

        fi.three2two.fromTo(a3, b3, c3, a2, b2, c2);
        if (isnan(fi.three2two.dqTransf.dualQuaternion.real.x))
            std::cout << "ERROR! NaN" << std::endl;
    }

}

// void Mesh::glueTriangleArap()
// {
//     std::cout << "starting System Solving..." << std::endl;
//     // Build System
//     Eigen::SparseMatrix<double> A;
//     Eigen::VectorXd b;
//     A.resize(v.size() * 3, v.size() * 3);
//     b.resize(v.size() * 3);
//     b.setZero();

//     std::vector<Eigen::Triplet<double>> tripletList;

//     for (int i = 0; i < v.size(); i++)
//     {
//         int j = i / 3; // face
//         if (v[i].copyOf != i)
//         {
//             for (int k = 0; k < 3; k++)
//             {
//                 tripletList.push_back({(i * 3) + k, (i * 3) + k, 1.0});
//                 tripletList.push_back({(i * 3) + k, (v[i].copyOf * 3) + k, -1.0});
//             }
//             glm::vec3 tempVec = heapPosPtr[v[i].copyOf] - heapPosPtr[i];
//             b((i * 3) + 0) = tempVec.x;
//             b((i * 3) + 1) = tempVec.y;
//             b((i * 3) + 2) = tempVec.z;
//             // glm::vec3 tempVec1 = heapPosPtr[i] - v[i].pos;
//             // glm::vec3 tempVec2 = heapPosPtr[v[i].copyOf] - v[v[i].copyOf].pos;
//             // glm::vec3 diff = tempVec1 - tempVec2;
//             // b((i * 3) + 0) = diff.x;
//             // b((i * 3) + 1) = diff.y;
//             // b((i * 3) + 2) = diff.z;
//         }
//         else
//         {
//             // find the next index in the same triangle
//             int offset = i % 3;
//             int nextV = f[j].vi[(offset + 1) % 3];
//             glm::vec3 tempVec = heapPosPtr[i] - heapPosPtr[nextV];
//             float tempArr[] = {tempVec.x, tempVec.y, tempVec.z};
//             for (int k = 0; k < 3; k++)
//             {
//                 tripletList.push_back({(i * 3) + k, (i * 3) + k, -tempArr[k]});
//                 tripletList.push_back({(i * 3) + k, nextV * 3 + k, tempArr[k]});
//             }

//             b((i * 3) + 0) = 0;
//             b((i * 3) + 1) = 0;
//             b((i * 3) + 2) = 0;
//         }
//     }
//     A.setFromTriplets(tripletList.begin(), tripletList.end());

//     Eigen::MatrixXd denseA = A.toDense(); // Convert to dense if A is sparse
//     double det = denseA.determinant();

//     if (det == 0)
//     {
//         std::cerr << "The matrix is singular (determinant is zero)." << std::endl;
//     }
//     else
//     {
//         std::cout << "The matrix is not singular (determinant is non-zero)." << std::endl;
//     }

//     // Solve
//     // Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver;
//     // Eigen::SparseLU<Eigen::SparseMatrix<double>> solver; I can't, it is singular
//     Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> solver; // out of bound memory

//     // solver.analyzePattern(A);
//     // solver.factorize(A);

//     // if (solver.info() != Eigen::Success)
//     // {
//     //     std::cerr << "Factorization failed: " << std::endl;
//     //     // Handle the error (e.g., return, throw an exception, etc.)
//     // }
//     solver.compute(A);
//     Eigen::VectorXd x = solver.solve(b);
//     if (solver.info() != Eigen::Success)
//     {
//         std::cerr << "Solving failed!" << std::endl;
//         // return -1;
//     }

//     // Apply
//     for (int i = 0; i < v.size(); i++)
//     {
//         Eigen::Vector3d translation = x.segment<3>(i * 3);
//         heapPosPtr[i] = heapPosPtr[i] + glm::vec3(translation.x(), translation.y(), translation.z());
//     }
//     std::cout << "System Done." << std::endl;
// }

glm::vec3 uv2xzy(glm::vec2 v)
{
    return glm::vec3(v.x, 0.0, v.y);
}

static float sigmoid(float t) // ease in ease out
{
    return (glm::sin(t * PI - PI / 2) + 1) / 2;
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
        if (isnan(fi.three2two.dqTransf.dualQuaternion.real.x))
            std::cout << "ERROR! NaN" << std::endl;
    }
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

void Mesh::resetTiming()
{
    for (Vertex &vi : v)
    {
        vi.tStart = 0.0;
        vi.tEnd = 1.0;
    }
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
    updateAverageTimingPerIsland();
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
    updateAverageTimingPerIsland();
}

void Mesh::updateAverageTimingPerFace()
{
    for (Face fi : f)
    {
        float averageStart = 0.0;
        float averageEnd = 0.0;
        for (int i : fi.vi)
        {
            averageStart += v[i].tStart;
            averageEnd += v[i].tEnd;
        }
        averageStart /= 3;
        averageEnd /= 3;
        std::cout << sizeof(fi.vi) << std::endl;
        for (int i : fi.vi)
        {
            v[i].tStart = averageStart;
            v[i].tEnd = averageEnd;
        }
    }
}

void Mesh::updateAverageTimingPerIsland()
{
    std::vector<float> averageStarts(v.size(), 0);
    std::vector<float> averageEnds(v.size(), 0);
    std::vector<int> counts(v.size(), 0);

    for (Vertex &vi : v)
    {
        averageStarts[vi.islandId] += vi.tStart;
        averageEnds[vi.islandId] += vi.tEnd;
        counts[vi.islandId] += 1;
    }

    for (int i = 0; i < v.size(); i++)
    {
        averageStarts[i] /= counts[i];
        averageEnds[i] /= counts[i];
    }

    for (Vertex &vi : v)
    {
        vi.tStart = averageStarts[vi.islandId];
        vi.tEnd = averageEnds[vi.islandId];
    }
}

void Mesh::updateAverageQuaternionRotationAreaWeighted()
{
    float areaSum = 0.0;
    glm::dualquat dqSum = glm::dualquat();
    for (Face fi : f)
    {
        glm::dualquat dq = fi.three2two.dqTransf.dualQuaternion;
        // float area = ComputeArea(v[fi.vi[0]].pos, v[fi.vi[1]].pos, v[fi.vi[2]].pos);
        float area = 0.5 * length(cross(v[fi.vi[0]].pos - v[fi.vi[1]].pos, v[fi.vi[0]].pos - v[fi.vi[2]].pos));
        area /= averageScaling;
        if (area <= 0.000001f)
            continue;
        areaSum += area;

        dqSum = sum(dqSum, dq * area);
    }
    if (areaSum > 0.0)
    {
        initialTranform = dqSum / areaSum;
        initialTranform = myNormalized(initialTranform);
    }

    // Apply
    // DualQuatTransform t = DualQuatTransform(glm::dualquat(initialTranform.real, glm::quat())); // remove translation
    DualQuatTransform t = DualQuatTransform(initialTranform);
    for (int i = 0; i < v.size(); ++i)
    {
        v[i].pos = t.apply(v[i].pos);
    }
    boundingSphere.center = t.apply(boundingSphere.center);
}

void Mesh::updatePathVerse(int mode)
{
    for (Face &fi : f)
    {

        if (mode == -1 && glm::dot(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), fi.three2two.dqTransf.dualQuaternion.real) <= 0.0f)
        {
            fi.pathVerse = -1;
        }
        else if (mode == 1 && glm::dot(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), fi.three2two.dqTransf.dualQuaternion.real) >= 0.0f)
        {
            fi.pathVerse = -1;
        }
        else if (mode == 0)
        {
            fi.pathVerse = 1;
        }
        else if (mode == 2) // neutral
        {
            fi.pathVerse = -1;
        }
        else
        {
            fi.pathVerse = 1;
        }
    }
    // update pathVerse per vertex for using it in shaders TODO: refactoring
    for (Face &fi : f)
    {
        for (int i : fi.vi)
        {
            v[i].pathVerse = fi.pathVerse;
        }
    }
    for (int i = 0; i < v.size(); i++)
    {
        pathVersePtr[i] = v[i].pathVerse;
    }
}

void Mesh::updatePathVersePerIsland()
{
    // il voto serve per isola
    // tutti i vertici di una faccia stanno nella stessa isola
    updatePathVerse(-1);

    std::vector<int> votation(v.size(), 0);

    for (Face &fi : f)
    {
        int island = v[fi.vi[0]].islandId;
        votation[island] += fi.pathVerse;
    }

    for (Face &fi : f)
    {
        int votationResult = votation[v[fi.vi[0]].islandId];
        fi.pathVerse = std::max(-1, std::min(votationResult, 1));
        if (fi.pathVerse == 0)
        {
            fi.pathVerse = 1;
        }
    }
    // update pathVerse per vertex for using it in shaders TODO: refactoring
    for (Face &fi : f)
    {
        for (int i : fi.vi)
        {
            v[i].pathVerse = fi.pathVerse;
        }
    }
    for (int i = 0; i < v.size(); i++)
    {
        pathVersePtr[i] = v[i].pathVerse;
    }

    std::cout << "island:" << countIslands() << std::endl;
}

int Mesh::findIsland(int i) const
{
    if (v[i].islandId == i)
        return i;
    else
    {
        return findIsland(v[i].islandId);
    }
}

void Mesh::unionIsland(int x, int y)
{
    int xset = findIsland(x);
    int yset = findIsland(y);

    if (xset == yset)
        return;

    // Put smaller ranked item under
    // bigger ranked item if ranks are
    // different
    if (v[xset].islandRank < v[yset].islandRank)
    {
        v[xset].islandId = yset;
    }
    else if (v[xset].islandRank > v[yset].islandRank)
    {
        v[yset].islandId = xset;
    }
    else
    {
        v[yset].islandId = xset;
        v[xset].islandRank = v[xset].islandRank + 1;
    }
}

void Mesh::updateIsland()
{
    updateCopyOf(false);
    initIsland();
    for (int i = 0; i < f.size(); i++)
    {
        unionIsland(f[i].vi[0], f[i].vi[1]);
        unionIsland(f[i].vi[0], f[i].vi[2]);
    }
    for (int i = 0; i < v.size(); i++)
    {
        unionIsland(i, v[i].copyOf);
    }
    for (int i = 0; i < v.size(); i++)
    {
        v[i].islandId = findIsland(v[i].islandId);
    }
    std::cout << "island:" << countIslands() << std::endl;
}

void Mesh::initIsland()
{
    for (int i = 0; i < v.size(); i++)
    {
        v[i].islandId = i;
    }
}

int Mesh::countIslands()
{
    int count = 0;
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i].islandId == i)
            count++;
    }
    return count;
}

// void Mesh::updateFacesNeighbors()
// {
//     for (int i = 0; i < f.size(); i++)
//     {
//         for (int j = i + 1; j < f.size(); j++)
//         {
//             int count = 0;
//             for (int k = 0; k < 3; k++)
//             {
//                 for (int l = 0; l < 3; l++)
//                 {
//                     if (v[f[i].vi[k]].copyOf == v[f[j].vi[l]].copyOf)
//                     {
//                         count++;
//                     }
//                 }
//             }
//             if (count > 1)
//             {
//                 f[i].neighbors.insert(j);
//                 f[j].neighbors.insert(i);
//             }
//         }
//     }
// }

void Mesh::updateFacesNeighbors()
{
    updateCopyOf(false);
    std::map<std::pair<int, int>, std::vector<int>> edgeMap;

    for (int i = 0; i < f.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            int v1 = v[f[i].vi[j]].copyOf;
            int v2 = v[f[i].vi[(j + 1) % 3]].copyOf;
            if (v1 < v2)
            {
                int temp = v1;
                v1 = v2;
                v2 = temp;
            }
            std::pair<int, int> edge(v1, v2);
            edgeMap[edge].push_back(i);
        }
    }

    for (const auto &[key, values] : edgeMap)
    {
        for (int i : values)
        {
            for (int j : values)
            {
                if (i == j)
                    continue;
                f[i].neighbors.insert(j);
            }
        }
    }
}

bool Mesh::uniformQuaternionSigns()
{
    bool success = true;
    std::vector<bool> processed(f.size(), false);
    for (int seedi = 0; seedi < f.size(); seedi++)
    {
        if (processed[seedi])
            continue;
        std::vector<int> processingQueue;
        processingQueue.push_back(seedi);
        int k = 0; // indices of the processing queue
        while (k < processingQueue.size())
        {
            int i = processingQueue[k++];
            if (processed[i])
                continue;
            processed[i] = true;
            for (int j : f[i].neighbors)
            {
                processingQueue.push_back(j);
                if (glm::dot(f[i].three2two.dqTransf.dualQuaternion.real, f[j].three2two.dqTransf.dualQuaternion.real) < 0.0)
                {
                    if (processed[j])
                    {
                        success = false;
                    }
                    else
                    {
                        f[j].three2two.dqTransf.dualQuaternion = -f[j].three2two.dqTransf.dualQuaternion;
                    }
                }
            }
        }
    }
    return success;
}