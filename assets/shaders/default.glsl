#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 0) out vec3 v_FragPos;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

void main() {
    v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoords = a_TexCoords;

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 v_FragPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoords;

uniform vec3 u_LightPos = vec3(5.0, 5.0, 5.0);
uniform vec3 u_LightColor = vec3(1.0, 0.95, 0.9);
uniform vec3 u_ViewPos = vec3(0.0, 0.0, 5.0);

void main() {
    // Elegant procedural dark grid background / surface pattern
    vec3 baseColor = vec3(0.2, 0.4, 0.8);
    float gridX = step(0.05, fract(v_TexCoords.x * 5.0));
    float gridY = step(0.05, fract(v_TexCoords.y * 5.0));
    float grid = gridX * gridY;
    baseColor = mix(vec3(0.08, 0.1, 0.15), baseColor, grid);

    // Normal calculation
    vec3 norm = normalize(v_Normal);
    vec3 lightDir = normalize(u_LightPos - v_FragPos);

    // Diffuse Blinn-Phong
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    // Specular Blinn-Phong
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec * u_LightColor;

    // Ambient
    vec3 ambient = vec3(0.15) * baseColor;

    vec3 finalColor = (ambient + diffuse * baseColor + specular);
    
    // Apply modern tone mapping and gamma correction
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0/2.2));

    color = vec4(finalColor, 1.0);
}
