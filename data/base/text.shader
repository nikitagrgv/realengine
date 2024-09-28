/////////////////////////////////////////////////////////////////////////////////
#inout vec2 ioUV;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

void main()
{
    ioUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, ioUV);
}