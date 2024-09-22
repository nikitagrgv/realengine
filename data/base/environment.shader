/////////////////////////////////////////////////////////////////////////////////
#inout vec3 ioTexCoords;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uViewProj;

void main()
{
    ioTexCoords = aPos;
    gl_Position = uViewProj * vec4(aPos, 1.0f);
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

uniform samplerCube uSkybox;

void main()
{
    FragColor = texture(uSkybox, ioTexCoords);
}