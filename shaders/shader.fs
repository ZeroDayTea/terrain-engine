#version 430 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

layout(binding = 0) uniform sampler2D noiseTex;

// terrain colors
const vec3 grassLight = vec3(0.58606106, 0.68867920, 0.0);
const vec3 grassDark = vec3(0.13900483, 0.42452830, 0.0);
const vec3 rockLight = vec3(0.59433960, 0.36826882, 0.19344072);
const vec3 rockDark = vec3(0.24528301, 0.13069123, 0.09140263);

// triplanar
const float noiseScale1 = 32.0;
const float noiseScale2 = 64.0;

// lighting
const float ambientStrength = 0.30;
const float spec_power = 32.0;
const float spec_strength = 0.10;

// terrain blending
const float steepness_threshold = 0.28;
const float steepness_blend_width = 0.06;
const float noise_bias = 0.10;

vec4 triplanarSample(vec3 worldPos, vec3 N, float scale, sampler2D tex) {
    vec3 scaledPos = worldPos / scale;

    vec4 colX = texture(tex, scaledPos.yz);
    vec4 colY = texture(tex, scaledPos.xz);
    vec4 colZ = texture(tex, scaledPos.xy);

    vec3 blendWeight = N * N;
    blendWeight /= max(dot(blendWeight, vec3(1.0)), 1e-6);

    return colX * blendWeight.x + colY * blendWeight.y + colZ * blendWeight.z;
}

void main() {
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos - fragPos);
    vec3 H = normalize(L + V);

    // Calculate steepness (0 = flat, 1 = vertical/overhang)
    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    float upDot = dot(worldUp, N);
    // 0=flat up, 0.5=vertical, 1=flat down
    float angle01 = upDot * 0.5 + 0.5;
    float steepness = 1.0 - angle01;

    vec4 noise1 = triplanarSample(fragPos, N, noiseScale1, noiseTex);
    vec4 noise2 = triplanarSample(fragPos, N, noiseScale2, noiseTex);

    vec3 grassColor = mix(grassLight, grassDark, noise1.r);

    const float quantSteps = 10.0;
    float quantizedNoise = floor(noise2.r * quantSteps) / quantSteps;
    vec3 rockColor = mix(rockLight, rockDark, quantizedNoise);

    // rock weight based on steepness
    float noiseOffset = (noise1.r - 0.5) * noise_bias;
    float thresholdMin = steepness_threshold + noiseOffset;
    float thresholdMax = thresholdMin + steepness_blend_width;

    float rockWeight = smoothstep(thresholdMin, thresholdMax, steepness);

    vec3 albedo = mix(grassColor, rockColor, rockWeight);

    // blinn-phong lighting
    vec3 ambient = ambientStrength * vec3(0.4, 0.45, 0.5);

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = lightColor * NdotL;

    float NdotH = max(dot(N, H), 0.0);
    vec3 specular = lightColor * pow(NdotH, spec_power) * spec_strength;

    // rock is slightly more shiny
    float specularMod = mix(1.0, 1.3, rockWeight);
    specular *= specularMod;

    vec3 finalColor = (ambient + diffuse + specular) * albedo;
    FragColor = vec4(finalColor, 1.0);
}
