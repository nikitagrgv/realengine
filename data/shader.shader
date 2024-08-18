/////////////////////////////////////////////////////////////////////////////////
#version 330 core

#inout vec2 ioUV;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0f);
    ioUV = aUV;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, ioUV);
}