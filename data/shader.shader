/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec2 ioUV;
#inout vec3 ioNormal;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0f);
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
    FragColor = uLightColor * texture(uTexture, ioUV);
}