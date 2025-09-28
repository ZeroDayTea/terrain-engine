#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
 
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vec3 worldPos = (model * vec4(aPos, 1.0f)).xyz;
  fragPos = worldPos;
  // mat3 normalMatrix = mat3(transpose(inverse(model)));
  // normal = normalize(normalMatrix * aNormal);
  normal = aNormal;

  gl_Position = projection * view * vec4(worldPos, 1.0);
}
