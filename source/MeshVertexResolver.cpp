#include "MeshVertexResolver.h"

/// <summary>
/// pmp::SurfaceMesh 안에 있는 정점 정보들을 vector로 변환해 리턴
/// </summary>
/// <returns>Vertex 리스트</returns>
std::vector<glm::vec3> MeshVertexResolver::GetVertices() {
    std::vector<glm::vec3> vertices;

    for (auto v : mesh.vertices()) {
	auto vertPos = mesh.position(v);
	vertices.push_back(glm::vec3(
	    vertPos.data()[0],
	    vertPos.data()[1],
	    vertPos.data()[2]
	));
    }

    return vertices;
}