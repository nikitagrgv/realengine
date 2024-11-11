/////////////////////////////////////////////////////////////////////////////////
#inout vec3 ioFragPos;
#inout vec3 ioNormal;
#inout vec2 ioUV;
#inout float ioAo;

/////////////////////////////////////////////////////////////////////////////////
#vertex
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in float aAo;

uniform mat4 uModelViewProj;

void main()
{
    vec4 glob_pos = vec4(aPos, 1.0f);
    gl_Position = uModelViewProj * glob_pos;
    ioFragPos = aPos;
    ioNormal = aNormal;
    ioUV = aUV;
    ioAo = aAo;
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

struct Light {
    vec3 dir;
};
uniform Light uLight;

uniform sampler2D atlas;

void main()
{
    vec3 norm = normalize(ioNormal);

    vec4 albedo_color = texture(atlas, ioUV);

    vec4 ambient = vec4(albedo_color.xyz * 0.3, albedo_color.w);

    vec3 dir_to_light = -uLight.dir;
    float diff = float(dir_to_light.y > 0) * sqrt(abs(dir_to_light.y)) * max(dot(norm, dir_to_light), 0.0);
    vec4 diffuse = 0.7 * diff * albedo_color;

    float ao = mix(0.3, 1.0, ioAo);
    FragColor = (ambient + diffuse) * vec4(ao, ao, ao, 1);
}