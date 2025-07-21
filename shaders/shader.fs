#version 430 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

void main()
{
  vec3 norm = normalize(normal);

  // color on steepness
  vec3 grassColor = vec3(0.3, 0.6, 0.2);
  vec3 rockColor = vec3(0.4, 0.35, 0.3);
  vec3 snowColor = vec3(0.9, 0.9, 0.95);

  float slope = norm.y; // smoothstep(0.7, 0.05, normal.y);
  float heightFactor = clamp((fragPos.y - 5.0) / 20.0, 0.0, 1.0);
  vec3 terrainColor = mix(rockColor, grassColor, smoothstep(0.4, 0.7, slope));
  terrainColor = mix(terrainColor, snowColor, heightFactor * smoothstep(0.6, 0.8, slope));

  // lighting
  // ambient lighting
  vec3 ambientColor = vec3(0.4, 0.45, 0.5);
  float ambientStrength = 0.3;
  vec3 ambient = ambientStrength * ambientColor;

  // diffuse lighting
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular lighting
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = 0.1 * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * terrainColor;

  FragColor = vec4(result, 1.0f);

  // FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}

