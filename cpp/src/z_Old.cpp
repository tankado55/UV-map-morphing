void Mesh::enforceArea() const // not work, probably useless
{
    std::cout << "posSize Debug: " << m_PosSize << " " << f.size() << std::endl;
    for (int i = 0; i < f.size(); i++)
    {
        Face face = f[i];
        float targetArea = face.area;
        int posIndex = i * 3 * 3;
        glm::vec3 p1 = glm::vec3(heapPosPtr[posIndex], heapPosPtr[posIndex + 1], heapPosPtr[posIndex + 2]);
        glm::vec3 p2 = glm::vec3(heapPosPtr[posIndex + 3], heapPosPtr[posIndex + 4], heapPosPtr[posIndex + 5]);
        glm::vec3 p3 = glm::vec3(heapPosPtr[posIndex + 6], heapPosPtr[posIndex + 7], heapPosPtr[posIndex + 8]);
        float currentArea = ComputeArea(p1, p2, p3);
        float scalingFactor = std::sqrt(targetArea / currentArea);

        glm::vec3 centroid = (p1 + p2 + p3) / 3.0f;

        p1 = centroid + (p1 - centroid) * scalingFactor;
        p2 = centroid + (p2 - centroid) * scalingFactor;
        p3 = centroid + (p3 - centroid) * scalingFactor;

        heapPosPtr[posIndex] = p1.x;
        heapPosPtr[posIndex + 1] = p1.y;
        heapPosPtr[posIndex + 2] = p1.z;
        heapPosPtr[posIndex + 3] = p2.x;
        heapPosPtr[posIndex + 4] = p2.y;
        heapPosPtr[posIndex + 5] = p2.z;
        heapPosPtr[posIndex + 6] = p3.x;
        heapPosPtr[posIndex + 7] = p3.y;
        heapPosPtr[posIndex + 8] = p3.z;
    }
}

function plusOne() {
	object.traverse(function (child) {
		if (child.isMesh) {
			let arr = child.geometry.attributes.position.array;
			Module.ccall(
				"plusOne", // The name of C++ function
				null, // The return type
				["number", "number"], // The argument types
				[heapGeometryPointer, arr.length] // The arguments
			);
			child.geometry.setAttribute('position', new THREE.BufferAttribute(Module["HEAPF32"].slice(heapGeometryPointer >> 2, (heapGeometryPointer >> 2) + arr.length), 3));
		}

	});
}