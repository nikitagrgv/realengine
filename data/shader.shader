/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec4 ioColor;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform float uTime;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    float t = sin(uTime) / 2.0f + 0.5f;
    ioColor = vec4(aColor * t, 1.0f);
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

void main()
{
    FragColor = ioColor;
}