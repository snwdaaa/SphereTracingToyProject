#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// 모델 로드를 위한 Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// MVP(Model-View-Projection) 행렬 구현을 위한 라이브러리
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp로 모델 로드하고 Vertex 목록 채우는 함수
void LoadModel(const std::string& path, std::vector <glm::vec3>& vertices) {
    Assimp::Importer importer;

    // Importer 플래그
    // aiProcsss_Triangulate: 모든 면을 삼각형으로 분할
    // aiProcess_JoinIndenticalVertices: 중복되는 정점 합침
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);

    // 오류 발생시
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
        return;
    }

    // 재귀적으로 노드 순회하면서 Mesh 데이터 추출
    std::function<void(aiNode*, const aiScene*)> processNode = 
        [&](aiNode* node, const aiScene* scene) {
        // 현재 노드의 모든 Mesh 처리
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                glm::vec3 vertex;
                vertex.x = mesh->mVertices[j].x;
                vertex.y = mesh->mVertices[j].y;
                vertex.z = mesh->mVertices[j].z;
                vertices.push_back(vertex);
            }
        }

        // 자식 노드들 재귀적으로 정리
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    };

    processNode(scene->mRootNode, scene);
    std::cout << "Successfully loaded model: " << path << " with "
        << vertices.size() << " vertices." << std::endl;
}

// 셰이더 파일의 코드를 string으로 읽어오는 함수
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    return buffer.str();
}

// 셰이더를 GPU가 이해할 수 있게 컴파일 & 에러 체크
unsigned int compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // 컴파일 에러 체크
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

// GPU에서 실행될 셰이더 프로그램(vert + frag)을 생성하는 함수
unsigned int createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// ---------- 초기화 ------------

unsigned int shaderProgram;

int camPosLocation;
int camDirLocation;

glm::vec3 camPos;
glm::vec3 camFront;
glm::vec3 camUp = glm::vec3(0, 1, 0);
float camSpeed = 0.1f;

void InitCamera() {
    // 카메라 실시간 위치
    camPos = glm::vec3(0, 0, 3);
    camFront = glm::vec3(0, 0, -1); // 원점
    camPosLocation = glGetUniformLocation(shaderProgram, "u_camPos");
    camDirLocation = glGetUniformLocation(shaderProgram, "u_camDir");
    glUniform3f(camPosLocation, camPos.x, camPos.y, camPos.z);
    glUniform3f(camDirLocation, camFront.x, camFront.y, camFront.z);
}

// ---------- 초기화 ------------

// ---------- 카메라 ------------

void UpdateCamera() {
    glUniform3f(camPosLocation, camPos.x, camPos.y, camPos.z);
    glUniform3f(camDirLocation, camFront.x, camFront.y, camFront.z);
}

// ---------- 카메라 ------------

// ------------ 입력 ------------

void MoveKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W) {
        camPos += camSpeed * camFront;
    }
    else if (key == GLFW_KEY_S) {
        camPos -= camSpeed * camFront;
    }
    else if (key == GLFW_KEY_A) {
        camPos -= glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    }
    else if (key == GLFW_KEY_D) {
        camPos += glm::normalize(glm::cross(camFront, camUp)) * camSpeed;
    }
    else if (key == GLFW_KEY_Q) {
        camPos += camSpeed * camUp;
    }
    else if (key == GLFW_KEY_E) {
        camPos -= camSpeed * camUp;
    }
    else if (key == GLFW_KEY_SPACE) {
        camPos = glm::vec3(0, 0, 3);
    }
}

// ------------ 입력 ------------

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Construct the window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Sphere Tracer", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create the GLFW window\n";
        glfwTerminate();
    }

    // 생성된 창을 OpenGL 명령 내릴 대상으로 지정
    glfwMakeContextCurrent(window);

    // GLAD 라이브러리가 런타임 시점에 드라이버 주소 결정
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // 모델 로드
    std::vector<glm::vec3> vertices;
    LoadModel("../res/Sphere.obj", vertices);

    // ----- VBO 생성 코드 -----
    // Vertex의 위치, 색상, 텍스처 좌표 등 대량의 데이터를 담는 GPU 버퍼 생성

    unsigned int VBO; // VBO 식별 ID
    glGenBuffers(1, &VBO); // VBO 1개 생성 후 ID 받음

    // GL_ARRAY_BUFFER 타입의 버퍼로 VBO 바인딩
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // vertices 벡터의 데이터를 GPU로 복사
    // GL_STATIC_DRAW -> 데이터가 거의 변하지 않을 때 사용
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    // ----- VBO 생성 코드 -----

    // ----- VAO 코드 -----
    // VBO에 담긴 데이터를 GPU가 어떻게 해석해야 할지 지정
    // 모던 OpenGL은 무언가 그리려면 반드시 하나 이상의 VAO(Vertex Array Object) 필요

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // VAO 바인딩

    // Vertex Attribute Pointer 설정
    // VBO 데이터의 구조를 직접적으로 지정하는 거라고 보면 됨
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // 0번 attribute로 Float 3개인 위치 정보를 GPU로 전송
    glEnableVertexAttribArray(0); // 0번 location의 attribute 활성화

    // 설정 끝났으므로 VAO와 VBO 바인딩 해제
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ----- VAO 코드 -----

    // 초기 크기 지정
    glViewport(0, 0, 800, 600);

    // 창 크기 바뀔 때 OpenGL 뷰포트 업데이트 (= glutReshapeFunc)
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    });
 
    // 셰이더 로딩
    std::string vertexShaderSource = readShaderFile("../shader/basic.vert");
    std::string fragmentShaderSource = readShaderFile("../shader/raymarcher.frag");

    // 셰이더 프로그램 생성
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // 렌더 루프
    while (!glfwWindowShouldClose(window))
    {
        ProcessInput(window);

        // 배경색 설정
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 사용할 셰이더 프로그램 지정
        glUseProgram(shaderProgram);

        // ----- MVP(Model-View-Projection) 행렬 계산 및 전달 -----
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Model 행렬: 단위 행렬 사용
        glm::mat4 model = glm::mat4(1.0f);
        // View 행렬: 카메라 (0,0,3)에서 원점 바라봄
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // Projection 행렬: 45도 FOV로 원근 투영
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
            (float)width / (float)height, 0.1f, 100.0f);

        // 최종 MVP 행렬 계산
        glm::mat4 mvp = projection * view * model;

        // 셰이더의 u_mvp 유니폼 위치 찾아서 행렬 전달
        int mvpLocation = glGetUniformLocation(shaderProgram, "u_mvp");
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
        // ----- MVP(Model-View-Projection) 행렬 계산 및 전달 -----

        // 화면에 모델 그리기
        glBindVertexArray(VAO); // 설정이 저장된 VAO 바인딩
        // VBO에 있는 정점들 사용해 삼각형 그리기
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        
        glfwSwapBuffers(window); // 더블 버퍼링
        glfwPollEvents(); // 키보드, 마우스 이벤트 체크
    }

    // 리소스 정리
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}