#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform CameraData {
    mat4 u_ViewProjection;
    vec3 u_CameraPos;
};

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

out vec3 v_FragPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_FragPos = worldPos.xyz;
    v_Normal = u_NormalMatrix * a_Normal;
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 gNormal;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gMetallicRoughness;

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

// PBR Material Mapping
struct PBRMaterial {
    sampler2D AlbedoMap;             // Slot 0
    sampler2D MetallicRoughnessMap;  // Slot 1 (G=Roughness, B=Metallic)
    sampler2D NormalMap;             // Slot 2
    sampler2D AOMap;                 // Slot 3
};

uniform PBRMaterial u_Material;

// Global PBR Material Factors UBO (Binding slot 2, std140)
layout (std140, binding = 2) uniform MaterialData {
    vec3 u_AlbedoFactor;
    float matPadding1;
    float u_MetallicFactor;
    float u_RoughnessFactor;
    float u_AOFactor;
    float matPadding2;
};

vec3 GetNormalFromMap() {
    // 1. Sample tangent space normal map
    vec3 tangentNormal = texture(u_Material.NormalMap, v_TexCoord).xyz * 2.0 - 1.0;

    // 2. Reconstruct tangent space TBN matrix using high-speed screen-space derivatives (no vertex tangents needed!)
    vec3 Q1  = dFdx(v_FragPos);
    vec3 Q2  = dFdy(v_FragPos);
    vec2 st1 = dFdx(v_TexCoord);
    vec2 st2 = dFdy(v_TexCoord);

    vec3 N   = normalize(v_Normal);
    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    // 3. Map to world space normal
    return normalize(TBN * tangentNormal);
}

void main() {
    // 1. Normal (Mapped world-space Normal)
    gNormal = vec4(GetNormalFromMap(), 1.0);
    
    // 2. Albedo (Sampled + Factor)
    vec3 albedo = texture(u_Material.AlbedoMap, v_TexCoord).rgb * u_AlbedoFactor;
    gAlbedo = vec4(albedo, 1.0);
    
    // 3. Metallic & Roughness (Sampled + Factor)
    vec3 mrSample = texture(u_Material.MetallicRoughnessMap, v_TexCoord).rgb;
    float roughness = mrSample.g * u_RoughnessFactor;
    float metallic  = mrSample.b * u_MetallicFactor;
    gMetallicRoughness = vec4(metallic, roughness, 0.0, 1.0);
}
