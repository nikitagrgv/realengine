/////////////////////////////////////////////////////////////////////////////////
#inout vec3 ioFragPosGlobal;
#inout vec2 ioUV;
#inout vec3 ioNormalGlobal;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

// TODO# together
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
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
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
    vec3 emission = texture(uMaterial.emissionMap, ioUV).rgb;

    vec3 dir_to_light = normalize(uLight.pos - ioFragPosGlobal);
    vec3 norm = normalize(ioNormalGlobal);
    vec3 albedo_tex_color = vec3(texture(uMaterial.diffuseMap, ioUV));
    vec3 specular_tex_color = vec3(texture(uMaterial.specularMap, ioUV));

    #ifdef USE_AMBIENT
    ambient = uLight.ambientPower * uLight.color * uMaterial.ambient * albedo_tex_color;
    #endif

    #ifdef USE_DIFFUSE
    float diff = max(dot(norm, dir_to_light), 0.0);
    diffuse = uLight.diffusePower * diff * uLight.color * uMaterial.diffuse * albedo_tex_color;
    #endif

    #ifdef USE_SPECULAR
    vec3 dir_to_view = normalize(uCameraPos - ioFragPosGlobal);
    vec3 reflect_dir = reflect(-dir_to_light, norm);
    float spec = pow(max(dot(dir_to_view, reflect_dir), 0.0), uMaterial.shininess);
    specular = uLight.specularPower * spec * uLight.color * uMaterial.specular * specular_tex_color;
    #endif

    FragColor = vec4(ambient + diffuse + specular + emission, 1);
}