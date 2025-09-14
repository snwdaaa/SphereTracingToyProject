#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

    // 초기 크기 지정
    glViewport(0, 0, 800, 600);

    // 창 크기 바뀔 때 OpenGL 뷰포트 업데이트 (= glutReshapeFunc)
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    });

    // 셰이더 로딩 및 VAO 생성
    // 모던 OpenGL은 무언가 그리려면 반드시 하나 이상의 VAO(Vertex Array Object) 필요

    // 셰이더 파일 읽기
    std::string vertexShaderSource = readShaderFile("../shader/basic.vert");
    std::string fragmentShaderSource = readShaderFile("../shader/raymarcher.frag");

    // 셰이더 프로그램 생성
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // 빈 VAO 생성 (버퍼 없는 렌더링)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // This is the render loop
    while (!glfwWindowShouldClose(window))
    {
        ProcessInput(window);

        // 사용할 셰이더 프로그램 지정
        glUseProgram(shaderProgram);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // 유니폼 변수 전달
        // 매 프레임마다 창 크기 가져와서 u_resolution이라는 이름으로 셰이더에 전달
        int resolutionLocation = glGetUniformLocation(shaderProgram, "u_resolution");
        glUniform2f(resolutionLocation, (float)width, (float)height); // 해상도 값 업데이트

        // 화면 전체를 덮는 사각형 그리기
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 배경 그리기 명령
        
        glfwSwapBuffers(window); // 더블 버퍼링
        glfwPollEvents(); // 키보드, 마우스 이벤트 체크
    }

    // 리소스 정리
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}