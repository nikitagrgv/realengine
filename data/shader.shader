/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec4 ioColor;
#inout vec2 ioUV;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

uniform float uTime;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    float t = sin(uTime) / 2.0f + 0.5f;
    ioColor = vec4(aColor * t, 1.0f);
    ioUV = aUV;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = ioColor + texture(uTexture, ioUV);
}