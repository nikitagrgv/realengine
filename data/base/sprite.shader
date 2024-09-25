/////////////////////////////////////////////////////////////////////////////////
#inout vec2 ioUV;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat3 uTransform;

void main()
{
    ioUV = aUV;

    vec2 pos = (uTransform * vec3(aPos, 1.0)).xy;
    gl_Position = vec4(pos, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, ioUV);
}