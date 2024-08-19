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
uniform vec4 uLightColor;
uniform vec3 uLightPos;

void main()
{
    vec3 dir_to_light = normalize(uLightPos - ioGlobalPos);
    float dot = max(dot(ioNormal, dir_to_light), 0.0);
    FragColor = dot * uLightColor * texture(uTexture, ioUV);
}