#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_FragPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoords;
layout(location = 3) out vec4 v_FragPosLightSpace;

// Global Camera UBO (Binding slot 0)
layout (std140, binding = 0) uniform CameraData {
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_ViewPos;
};

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;
uniform mat4 u_LightSpaceMatrix;

void main() {
    v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    v_Normal = u_NormalMatrix * a_Normal;
    v_TexCoords = a_TexCoords;
    v_FragPosLightSpace = u_LightSpaceMatrix * vec4(v_FragPos, 1.0);

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 v_FragPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoords;
layout(location = 3) in vec4 v_FragPosLightSpace;

const float PI = 3.14159265359;

// ---- PBR Structural Definitions ----

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    float cutOff;
    vec3 direction;
    float outerCutOff;
    
    vec3 color;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;
};

// Global Lights SSBO (Binding slot 1, std430)
layout (std430, binding = 1) readonly buffer LightData {
    DirectionalLight u_DirLight;
    uint u_PointLightCount;
    uint u_SpotLightCount;
    PointLight u_PointLights[256];
    SpotLight u_SpotLights[256];
};

// Metallic-Roughness PBR Material mapping
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

// Global Camera UBO (Binding slot 0)
layout (std140, binding = 0) uniform CameraData {
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_ViewPos;
};

uniform sampler2DShadow u_ShadowMap;
uniform float u_ShadowBiasConstant;
uniform int u_PCFKernelSize;

// ---- Modular PBR BRDF Functions ----

// 1. Trowbridge-Reitz GGX Normal Distribution Function (NDF)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / max(denom, 0.0000001); // Prevent division by zero
}

// 2. Schlick-GGX Geometry Smith Component
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // k for direct analytical lighting
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

// Smith's Joint Shadowing-Masking Geometry Function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// 3. Fresnel-Schlick Approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal Map Normal derivation helper
vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(u_Material.NormalMap, v_TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(v_FragPos);
    vec3 Q2  = dFdy(v_FragPos);
    vec2 st1 = dFdx(v_TexCoords);
    vec2 st2 = dFdy(v_TexCoords);

    vec3 N   = normalize(v_Normal);
    vec3 T   = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ---- Shadow Map Soft Filtering ----
float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform from [-1, 1] range to [0, 1] UV coordinates
    projCoords = projCoords * 0.5 + 0.5;
    
    // If coords are beyond the far plane of shadow map, return no shadow
    if (projCoords.z > 1.0) {
        return 0.0;
    }
    
    // Dynamic Slope-Scale Bias
    float bias = max(u_ShadowBiasConstant * (1.0 - max(dot(normal, lightDir), 0.0)), u_ShadowBiasConstant * 0.1);
    
    // Hardware-Accelerated PCF Filter using sampler2DShadow
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    
    int range = clamp(u_PCFKernelSize, 0, 3);
    int count = 0;
    
    float compareDepth = projCoords.z - bias;
    
    for (int x = -range; x <= range; ++x) {
        for (int y = -range; y <= range; ++y) {
            // texture(sampler2DShadow, vec3) performs hardware depth comparison and bilinear filtering.
            // Returns 1.0 (fully lit) to 0.0 (fully shadowed).
            float isLit = texture(u_ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, compareDepth));
            shadow += (1.0 - isLit);
            count++;
        }
    }
    
    if (count > 0) {
        shadow /= float(count);
    }
    
    return shadow;
}

// ---- Core Cook-Torrance Light Integration ----

vec3 ComputePBR(vec3 L, vec3 V, vec3 N, vec3 F0, vec3 albedo, float roughness, float metallic, vec3 radiance) {
    vec3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0000001);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // BRDF evaluations
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F  = FresnelSchlick(HdotV, F0);

    // Specular Cook-Torrance term
    vec3 nominator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = nominator / denominator;

    // Diffuse term under energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Direct lighting sum
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// ---- Film-grade ACES Tone Mapping ----

vec3 ACESFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

void main() {
    // 1. Gathers material values from dynamic-slice map structures
    vec3 albedo = texture(u_Material.AlbedoMap, v_TexCoords).rgb * u_AlbedoFactor;
    vec3 mrSample = texture(u_Material.MetallicRoughnessMap, v_TexCoords).rgb;
    
    // G channel stores Roughness, B channel stores Metallic
    float roughness = mrSample.g * u_RoughnessFactor;
    float metallic  = mrSample.b * u_MetallicFactor;
    float ao        = texture(u_Material.AOMap, v_TexCoords).r * u_AOFactor;

    // 2. Normal Mapping derivation
    vec3 N = GetNormalFromMap();
    vec3 V = normalize(u_ViewPos - v_FragPos);

    // 3. Define F0 (specular reflectance at normal incidence)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 4. Lighting Accumulation
    vec3 lo = vec3(0.0);

    // Directional Light pass (scaled by shadow mapping factor)
    {
        vec3 L = normalize(-u_DirLight.direction);
        vec3 radiance = u_DirLight.color * u_DirLight.intensity;
        
        float shadow = CalculateShadow(v_FragPosLightSpace, N, L);
        lo += (1.0 - shadow) * ComputePBR(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // Point Lights pass
    for (uint i = 0; i < u_PointLightCount; ++i) {
        PointLight light = u_PointLights[i];
        vec3 L = normalize(light.position - v_FragPos);
        
        float distance = length(light.position - v_FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        vec3 radiance = light.color * light.intensity * attenuation;

        lo += ComputePBR(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // Spot Lights pass
    for (uint i = 0; i < u_SpotLightCount; ++i) {
        SpotLight light = u_SpotLights[i];
        vec3 L = normalize(light.position - v_FragPos);
        
        float distance = length(light.position - v_FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        float theta = dot(L, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        
        vec3 radiance = light.color * light.intensity * attenuation * intensity;

        lo += ComputePBR(L, V, N, F0, albedo, roughness, metallic, radiance);
    }

    // 5. Ambient Term (scaled by AO)
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 colorOut = ambient + lo;

    // 6. Filmic ACES Tone Mapping & standard Gamma Correction
    vec3 mapped = ACESFilm(colorOut);
    mapped = pow(mapped, vec3(1.0 / 2.2));

    color = vec4(mapped, 1.0);
}
