#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
 
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vec4 fragPos4 = model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
  fragPos = vec3(fragPos4.x, fragPos4.y, fragPos4.z);
  normal = mat3(transpose(inverse(model))) * aNormal;
  // gl_Position = projection * view * vec4(fragPos.x, fragPos.y, fragPos.z, 1.0f);

  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
}
