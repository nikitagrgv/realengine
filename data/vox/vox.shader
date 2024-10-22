/////////////////////////////////////////////////////////////////////////////////
#inout vec2 ioUV;
#inout vec2 ioNormal;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uModelViewProj;

void main()
{
    vec4 glob_pos = vec4(aPos, 1.0f);
    gl_Position = uModelViewProj * glob_pos;
    ioUV = aUV;
    ioNormal = aNormal;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform sampler2D atlas;

void main()
{
    FragColor = texture(atlas, ioUV);
}