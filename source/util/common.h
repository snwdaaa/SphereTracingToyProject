#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// PMP 라이브러리
#include <pmp/surface_mesh.h>
#include <pmp/io/io.h>

// MVP(Model-View-Projection) 행렬 구현을 위한 라이브러리
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>