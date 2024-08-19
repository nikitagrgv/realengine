/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec3 ioGlobalPos;
#inout vec2 ioUV;
#inout vec3 ioNormal;

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
    ioGlobalPos = glob_pos.xyz;
    ioUV = aUV;
    ioNormal = aNormal;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uLightColor;
uniform vec3 uLightPos;

void main()
{
    float ambient_power = 0.1;
    vec3 ambient = ambient_power * uLightColor;

    vec3 dir_to_light = normalize(uLightPos - ioGlobalPos);
    vec3 norm = normalize(ioNormal);
    float diff = max(dot(norm, dir_to_light), 0.0);
    vec3 diffuse = diff * uLightColor;

    float l = length(uLightPos - ioGlobalPos);
    diffuse *= 1.0 / (l * l);

    FragColor = vec4((ambient + diffuse), 1) * texture(uTexture, ioUV);
}