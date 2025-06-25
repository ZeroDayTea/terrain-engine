#version 330 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
  // color on steepness
  vec3 grassColor = vec3(0.3, 0.6, 0.2);
  vec3 slopeColor = vec3(0.4, 0.35, 0.3);
  float slope = smoothstep(0.7, 0.05, normal.y);
  vec3 objColor = mix(slopeColor, grassColor, slope);

  // lighting
  float ambientStrength = 0.2;
  vec3 ambient = ambientStrength * lightColor;
  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  FragColor = vec4((ambient + diffuse) * objColor, 1.0f);

  // FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}

