#type vertex
#version 450 core

// Fullscreen quad trick using gl_VertexID
out vec2 v_TexCoords;

void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    v_TexCoords = vec2((x+1.0)*0.5, (y+1.0)*0.5);
    gl_Position = vec4(x, y, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoords;

// G-Buffer samplers
layout(binding = 0) uniform sampler2D gDepth; // Standard depth sampler replacing gPosition
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gAlbedo;
layout(binding = 3) uniform sampler2D gMetallicRoughness;
layout(binding = 4) uniform sampler2DShadow u_ShadowMap;

const float PI = 3.14159265359;

// ---- PBR Structural Definitions ----

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    float padding1;
    vec3 color;
    float intensity;
    
    float constant;
    float linear;
    float quadratic;
    float padding2;
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
    float padding;
};

// Global Lights SSBO (Binding slot 1, std430)
layout (std430, binding = 1) readonly buffer LightData {
    DirectionalLight u_DirLight;
    uint u_PointLightCount;
    uint u_SpotLightCount;
    PointLight u_PointLights[256];
    SpotLight u_SpotLights[256];
};

uniform vec3 u_ViewPos;
uniform mat4 u_LightSpaceMatrix;
uniform mat4 u_InverseVP; // Inverse View-Projection Matrix for screen space position reconstruction
uniform float u_ShadowBiasConstant;
uniform int u_PCFKernelSize;

// High-speed screen space depth reconstruction of World Space Position
vec3 ReconstructWorldPos(vec2 uv, float depthVal) {
    float z = depthVal * 2.0 - 1.0; // Map from [0, 1] depth to [-1, 1] NDC Space
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 worldSpacePosition = u_InverseVP * clipSpacePosition;
    worldSpacePosition /= worldSpacePosition.w; // Perspective divide
    return worldSpacePosition.xyz;
}

// ---- Modular PBR BRDF Functions ----

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---- Shadow Mapping ----
float CalculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0) return 0.0;

    float bias = max(u_ShadowBiasConstant * (1.0 - max(dot(normal, lightDir), 0.0)), u_ShadowBiasConstant * 0.1);
    float compareDepth = projCoords.z - bias;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    
    int range = clamp(u_PCFKernelSize, 0, 3);
    int count = 0;
    for(int x = -range; x <= range; ++x) {
        for(int y = -range; y <= range; ++y) {
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

void main() {
    // 1. Unpack G-Buffer
    float depthVal = texture(gDepth, v_TexCoords).r;
    vec3 fragPos = ReconstructWorldPos(v_TexCoords, depthVal);
    vec3 normal = texture(gNormal, v_TexCoords).xyz;
    vec3 albedo = texture(gAlbedo, v_TexCoords).rgb;
    vec2 metallicRoughness = texture(gMetallicRoughness, v_TexCoords).rg;
    
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    // Background/Sky
    if(length(normal) < 0.1) {
        color = vec4(albedo, 1.0); // or skybox color
        return;
    }

    vec3 N = normalize(normal);
    vec3 V = normalize(u_ViewPos - fragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    
    // Process Directional Light
    {
        vec3 L = normalize(-u_DirLight.direction);
        vec3 H = normalize(V + L);
        
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);
        
        vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(fragPos, 1.0);
        float shadow = CalculateShadow(fragPosLightSpace, N, L);

        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * u_DirLight.color * u_DirLight.intensity * NdotL;
    }
    
    // Process Point Lights
    for (uint i = 0; i < u_PointLightCount; ++i) {
        PointLight light = u_PointLights[i];
        
        vec3 L = normalize(light.position - fragPos);
        vec3 H = normalize(V + L);
        
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        vec3 radiance = light.color * light.intensity * attenuation;
        
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Process Spot Lights
    for (uint i = 0; i < u_SpotLightCount; ++i) {
        SpotLight light = u_SpotLights[i];
        
        vec3 L = normalize(light.position - fragPos);
        vec3 H = normalize(V + L);
        
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        // Spotlight cone factor
        float theta = dot(L, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        
        vec3 radiance = light.color * light.intensity * attenuation * intensity;
        
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    vec3 ambient = vec3(0.03) * albedo; // Simplified ambient
    vec3 finalColor = ambient + Lo;
    
    // HDR Tonemapping
    finalColor = finalColor / (finalColor + vec3(1.0));
    // Gamma Correction
    finalColor = pow(finalColor, vec3(1.0/2.2)); 
    
    color = vec4(finalColor, 1.0);
}
