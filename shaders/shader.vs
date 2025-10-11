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
    vec4 worldPos = model * vec4(aPos, 1.0f);
    fragPos = worldPos.xyz;

    // normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    normal = normalize(mat3(model) * aNormal);

    gl_Position = projection * view * worldPos;
}
