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
    gl_Position = uViewProj * vec4(aPos*1.01, 1.0f);
}

/////////////////////////////////////////////////////////////////////////////////
#fragment
out vec4 FragColor;

struct GlobalLight {
    vec3 color;
    vec3 dir;
};

uniform GlobalLight uSunLight;
uniform GlobalLight uMoonLight;

float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec2 x) {
    vec2 i = floor(x), f = fract(x);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);

    float f = 0.5000 * noise(p); p = m2 * p * 2.02;
    f += 0.2500 * noise(p); p = m2 * p * 2.03;
    f += 0.1250 * noise(p); p = m2 * p * 2.01;
    f += 0.0625 * noise(p);
    return f / 0.9375;
}

vec3 render(vec3 light_pos, vec3 pos, vec3 dir) {
    vec3 col;
    vec3 sky_base_col = mix(vec3(0.3, 0.55, 0.8), vec3(1.0, 0.8, 0.5), pow(1.0 - max(abs(light_pos.y), 0.0), 0.4));
    if (dir.y >= 0.0)
    {

        // Sky with haze
        col = sky_base_col * (1.0 - 0.8 * dir.y) * 0.9;

        // Sun
        float sundot = clamp(dot(dir, light_pos), 0.0, 1.0);
        col += 0.25 * vec3(1.0, 0.7, 0.4) * pow(sundot, 8.0);
        col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 64.0);

        // Clouds
        col = mix(col, vec3(1.0, 0.95, 1.0), 0.5 *
        smoothstep(0.5, 0.8, fbm((pos.xz + dir.xz * (250000.0 - pos.y) / dir.y) * 0.000008)));
    }

    // Horizon/atmospheric perspective
    col = mix(col, sky_base_col, pow(1.0 - max(abs(dir.y), 0.0), 8.0));

    return col;
}

void main()
{
    vec3 light_pos = normalize(-uSunLight.dir);
    vec3 pos = vec3(1, 1, 1);
    vec3 dir = normalize(ioTexCoords);
    vec3 col = render(light_pos, pos, dir);

    // Gamma encode
    col = pow(col, vec3(1.1));

    FragColor = vec4(col, 1.0);
}
