#pragma once

#include <glad/glad.h>

struct GLUniforms {
  GLint uModel, uView, uProj, uLightColor, uLightPos, uViewPos;
};

inline GLUniforms get_locations(GLuint prog) {
  GLUniforms U{};
  U.uModel = glGetUniformLocation(prog, "model");
  U.uView = glGetUniformLocation(prog, "view");
  U.uProj = glGetUniformLocation(prog, "projection");
  U.uLightColor = glGetUniformLocation(prog, "lightColor");
  U.uLightPos = glGetUniformLocation(prog, "lightPos");
  U.uViewPos = glGetUniformLocation(prog, "viewPos");

  return U;
}



#pragma once

#include <glad/glad.h>
#include <string>

unsigned int compile_compute_shader(const std::string& shaderCode, const std::string& name);
unsigned int generate_compute_program(const char* computePath);
unsigned int generate_compute_program(const char* includePath, const char* computePath);
unsigned int compile_shader(GLenum shaderType, const std::string& source, const std::string& typeName);
unsigned int generate_shader_program();
