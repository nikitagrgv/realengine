/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec3 ioFragPosGlobal;
#inout vec2 ioUV;
#inout vec3 ioNormalGlobal;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uViewProj;

void main()
{
    vec4 glob_pos = uModel * vec4(aPos, 1.0f);
    gl_Position = uViewProj * glob_pos;
    ioFragPosGlobal = glob_pos.xyz;
    ioUV = aUV;
    ioNormalGlobal = mat3(transpose(inverse(uModel))) * aNormal;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uCameraPos;

struct Light {
    vec3 color;
    vec3 pos;
    float ambientPower;
    float diffusePower;
    float specularPower;
};
uniform Light uLight;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material uMaterial;

void main()
{
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    vec3 dir_to_light = vec3(0.0);
    vec3 norm = vec3(0.0);

    #ifdef USE_AMBIENT
    ambient = uLight.ambientPower * uLight.color;
    #endif

    #ifdef USE_DIFFUSE
    dir_to_light = normalize(uLight.pos - ioFragPosGlobal);
    norm = normalize(ioNormalGlobal);
    float diff = max(dot(norm, dir_to_light), 0.0);
    diffuse = uLight.diffusePower * diff * uLight.color;
    #endif

    #ifdef USE_SPECULAR
    vec3 dir_to_view = normalize(uCameraPos - ioFragPosGlobal);
    vec3 reflect_dir = reflect(-dir_to_light, norm);
    float spec = pow(max(dot(dir_to_view, reflect_dir), 0.0), uMaterial.shininess);
    specular = uLight.specularPower * spec * uLight.color;
    #endif

    FragColor = vec4(ambient + diffuse + specular, 1) * texture(uTexture, ioUV);
}