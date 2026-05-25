#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_FragPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoords;

// Global Camera UBO (Binding slot 0)
layout (std140, binding = 0) uniform CameraData {
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_ViewPos;
};

uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;

void main() {
    v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    v_Normal = u_NormalMatrix * a_Normal;
    v_TexCoords = a_TexCoords;

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 v_FragPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoords;

// Structural definitions matching C++ layout exactly
struct DirectionalLight {
    vec3 direction;
    float padding1;
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

// Global Lights UBO (Binding slot 1)
layout (std140, binding = 1) uniform LightData {
    DirectionalLight u_DirLight;
    uint u_PointLightCount;
    uint u_SpotLightCount;
    float padding1;
    float padding2;
    PointLight u_PointLights[32];
    SpotLight u_SpotLights[32];
};

struct Material {
    sampler2D Albedo;
    sampler2D Specular;
    float Shininess;
};

uniform Material u_Material;

// Camera uniform mapping from global UBO
layout (std140, binding = 0) uniform CameraData {
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_ViewPos;
};

// Calculation helpers
vec3 CalculateDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 specTex) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.Shininess);
    
    vec3 ambient = vec3(0.08) * albedo;
    vec3 diffuse = light.color * light.intensity * diff * albedo;
    vec3 specular = light.color * light.intensity * spec * specTex;
    
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specTex) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.Shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    vec3 diffuse = light.color * light.intensity * diff * albedo * attenuation;
    vec3 specular = light.color * light.intensity * spec * specTex * attenuation;
    
    return (diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, vec3 specTex) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.Shininess);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Spotlight cutoff calculations
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 diffuse = light.color * light.intensity * diff * albedo * attenuation * intensity;
    vec3 specular = light.color * light.intensity * spec * specTex * attenuation * intensity;
    
    return (diffuse + specular);
}

void main() {
    // 1. Sample surface maps
    vec3 albedo = texture(u_Material.Albedo, v_TexCoords).rgb;
    vec3 specTex = texture(u_Material.Specular, v_TexCoords).rgb;
    
    vec3 norm = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    
    // 2. Accumulate lighting contributions
    vec3 lightingResult = CalculateDirLight(u_DirLight, norm, viewDir, albedo, specTex);
    
    for (uint i = 0; i < u_PointLightCount; ++i) {
        lightingResult += CalculatePointLight(u_PointLights[i], norm, v_FragPos, viewDir, albedo, specTex);
    }
    
    for (uint i = 0; i < u_SpotLightCount; ++i) {
        lightingResult += CalculateSpotLight(u_SpotLights[i], norm, v_FragPos, viewDir, albedo, specTex);
    }
    
    // 3. HDR Tone Mapping & Gamma Correction
    vec3 mapped = lightingResult / (lightingResult + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    color = vec4(mapped, 1.0);
}
