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
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;

void main()
{
    float ambient_power = 0.1;
    vec3 ambient = ambient_power * uLightColor;

    vec3 dir_to_light = normalize(uLightPos - ioFragPosGlobal);
    vec3 norm = normalize(ioNormalGlobal);
    float diff = max(dot(norm, dir_to_light), 0.0);
    vec3 diffuse = diff * uLightColor;

    vec3 dir_to_view = normalize(uCameraPos - ioFragPosGlobal);
    vec3 reflect_dir = reflect(-dir_to_light, norm);
    float spec = pow(max(dot(dir_to_view, reflect_dir), 0.0), 32);
    vec3 specular = spec * uLightColor * 5.0f;

    FragColor = vec4(ambient + diffuse + specular, 1) * texture(uTexture, ioUV);
}