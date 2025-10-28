#version 430 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

layout(binding = 0) uniform sampler2D noiseTex;

const vec3 grassLight = vec3(0.58606106, 0.68867920, 0.0);
const vec3 grassDark = vec3(0.13900483, 0.42452830, 0.0);
const vec3 rockLight = vec3(0.59433960, 0.36826882, 0.19344072);
const vec3 rockDark = vec3(0.24528301, 0.13069123, 0.09140263);

const float noiseScale1 = 32.0;
const float noiseScale2 = 64.0;

const float ambientStrength = 0.30;
const float spec_power = 32.0;
const float spec_strength = 0.10;

const float base_threshold = 0.28;
const float thresh_width = 0.06;
const float noise_bias = 0.10;

const float uTest = 0.0;

vec4 triplanarOffset(in vec3 vertPos, in vec3 nrm, in float scale, sampler2D tex, vec2 offset)
{
    vec3  scaledPos = vertPos / scale;
    vec4  colX = texture(tex, scaledPos.zy + offset);
    vec4  colY = texture(tex, scaledPos.xz + offset);
    vec4  colZ = texture(tex, scaledPos.xy + offset);

    vec3 blend = nrm * nrm;
    blend /= max(dot(blend, vec3(1.0)), 1e-6);

    return colX * blend.x + colY * blend.y + colZ * blend.z;
}


void main()
{
vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos  - fragPos);
    vec3 H = normalize(L + V);

    // Unity's "steepness": 0=flat up, 0.5=vertical, 1=flat down.
    // We're on a plane world (not spherical), so use world up (0,1,0):
    float angle01   = dot(vec3(0.0,1.0,0.0), N) * 0.5 + 0.5;
    float steepness = 1.0 - angle01;

    // Two triplanar noise samples like Unity
    vec4 noise1 = triplanarOffset(fragPos, N, noiseScale1, noiseTex, vec2(0.0));
    vec4 noise2 = triplanarOffset(fragPos, N, noiseScale2, noiseTex, vec2(0.0));

    // Colors modulated by noise channels (match Unity's r usage)
    vec3 grassCol = mix(grassLight, grassDark, noise1.r);

    // Quantize rock noise a bit like their "(int)(noise2.r*r)/float(r)"
    const float QUANT = 10.0;
    float q = floor(noise2.r * QUANT) / QUANT;
    vec3 rockCol  = mix(rockLight, rockDark, q);

    // Nudge threshold by noise and user control (_Test)
    float nudge = (noise1.r - 0.4) * uTest;

    // Unity used smoothstep(0.24+n, 0.241+n, steepness) (very sharp).
    // Keep that extremely narrow blend:
    float rockW = smoothstep(0.24 + nudge, 0.241 + nudge, steepness);

    // Final albedo
    vec3 albedo = mix(grassCol, rockCol, rockW);

    // Blinn-Phong lighting
    vec3 ambient  = ambientStrength * vec3(0.4, 0.45, 0.5);
    float NdotL   = max(dot(N,L), 0.0);
    float NdotH   = max(dot(N,H), 0.0);
    vec3 diffuse  = lightColor * NdotL;
    

    vec3 specular = lightColor * pow(NdotH, spec_power) * spec_strength;

    vec3 lit = (ambient + diffuse + specular) * albedo;
    FragColor = vec4(lit, 1.0);

}
