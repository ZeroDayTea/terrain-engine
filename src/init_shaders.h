#pragma once

std::string readShaderSource(const std::string& path) {
    std::ifstream shaderFile;
    shaderFile.open(path);
    if (!shaderFile.is_open()) {
        std::cout << "Failed to read shader source from file " << path << std::endl;
        exit(1);
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

// shader compilation for compute shaders
unsigned int compileComputeShader(const std::string& shaderCode, const std::string& name) {
    const char* cShaderSource = shaderCode.c_str();

    unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderSource, NULL);
    glCompileShader(compute);

    int success;
    char infoLog[512];
    glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(compute, 512, NULL, infoLog);
        std::cout << "ERROR: Compute Shader Compilation Failed (" << name << ")\n" << infoLog << std::endl;
        exit(1);
    }

    unsigned int computeProgram = glCreateProgram();
    glAttachShader(computeProgram, compute);
    glLinkProgram(computeProgram);

    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(computeProgram, 512, NULL, infoLog);
        std::cout << "ERROR: Compute Shader Program Linking Failed (" << name << ")\n" << infoLog << std::endl;
        exit(1);
    }

    glDeleteShader(compute);
    return computeProgram;
}

// single file compute shader
unsigned int generate_compute_program(const char* computePath) {
    std::string shaderCode = readShaderSource(computePath);
    return compileComputeShader(shaderCode, computePath);
}

// computer shader with include file
unsigned int generate_compute_program(const char* includePath, const char* computePath) {
    std::string includeShaderCode = readShaderSource(includePath);
    std::string computeShaderCode = readShaderSource(computePath);
    std::string combinedCode = "#version 430 core\n" + includeShaderCode + "\n" + computeShaderCode;
    return compileComputeShader(combinedCode, std::string(includePath) + " + " + computePath);
}

// vertex and fragment shader compile
unsigned int compileShader(GLenum shaderType, const std::string& source, const std::string& typeName) {
    const char* shaderSource = source.c_str();
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR: " << typeName << " Shader Compilation Failed\n" << infoLog << std::endl;
        exit(1);
    }

    return shader;
}

unsigned int generate_shader_program() {
    std::string vertexSource = readShaderSource("../shaders/shader.vs");
    std::string fragmentSource = readShaderSource("../shaders/shader.fs");

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, "Vertex");
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, "Fragment");

    // Link shaders into shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR: Shader Program Linking Failed\n" << infoLog << std::endl;
        exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
