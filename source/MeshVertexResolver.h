#include "util/common.h"

/// <summary>
/// pmp::SurfaceMesh 객체에 있는 정점 정보를 Vertex Shader에 전달하기 위해
/// std::vector로 변환하는 클래스
/// </summary>
class MeshVertexResolver {
public:
	MeshVertexResolver(pmp::SurfaceMesh mesh)
		: mesh(mesh)
	{}

	std::vector<glm::vec3> GetVertices();
private:
	pmp::SurfaceMesh mesh;
};