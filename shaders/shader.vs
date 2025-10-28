#version 430 core
layout (location = 0) in uint aPosPacked;
layout (location = 1) in uint aNormalPacked;
 
out vec3 fragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const uint POSITION_BITS = 10u;
const float POS_Q_MAX = float((1u << POSITION_BITS) - 1u);

vec3 decode_pos(uint p) {
    uint x = (p & 1023u);
    uint y = ((p >> 10) & 1023u);
    uint z = ((p >> 20) & 1023u);
    vec3 q = vec3(x,y,z) / POS_Q_MAX;
    // q = min(q, 1.0 - 1.0/1024.0); // attempt to fix inconsistent rounding across chunks
    return q * vec3(64.0);
}

vec3 oct_to_vec3(uint oct) {
    const uint NB = 11u;
    uint u =  (oct        & ((1u<<NB)-1u));
    uint v = ((oct >> NB) & ((1u<<NB)-1u));
    vec2 e = vec2(float(u), float(v)) / float((1u<<NB)-1u); // [0,1]
    e = e * 2.0 - 1.0;

    vec3 n = vec3(e.x, e.y, 1.0 - abs(e.x) - abs(e.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.x += (n.x >= 0.0 ? -t : t);
    n.y += (n.y >= 0.0 ? -t : t);
    return normalize(n);
}

void main()
{
    vec3 localPos = decode_pos(aPosPacked);
    vec3 N = oct_to_vec3(aNormalPacked);
    vec4 worldPos = model * vec4(localPos, 1.0f);
    fragPos = worldPos.xyz;

    normal = normalize(mat3(model) * N);

    gl_Position = projection * view * worldPos;
}
