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

uniform vec3 uCameraPos;
uniform float uTime;

float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec2 x) {
    vec2 i = floor(x), f = fract(x);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    const mat2 m2 = mat2(4.8, -5.6, 5.6, 4.8);

    float f = 0.5000 * noise(p); p = m2 * p * 2.02;
    f += 0.2500 * noise(p); p = m2 * p * 2.03;
    f += 0.1250 * noise(p); p = m2 * p * 2.01;
    f += 0.0625 * noise(p);
    return f / 0.9375;
}

mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(
    oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0
    );
}

vec3 rotateVector(vec3 v, vec3 axis, float angle) {
    mat4 m = rotationMatrix(axis, angle);
    return (m * vec4(v, 1.0)).xyz;
}

vec3 render(vec3 light_pos, vec3 pos, vec3 dir) {
    vec3 col;
    vec3 sky_base_col;
    if (light_pos.y >= 0)
    {
        sky_base_col = mix(vec3(0.3, 0.55, 0.8), vec3(1.0, 0.8, 0.5) * 0.5, pow(1.0 - max(abs(light_pos.y), 0.0), 2));
        sky_base_col += mix(vec3(0, 0, 0), vec3(0.4, 0.0, 0.1) * 0.5, pow(1.0 - max(abs(light_pos.y), 0.0), 3));
    }
    else
    {
        sky_base_col = mix(vec3(0.3, 0.55, 0.8) * 0.1, vec3(1.0, 0.8, 0.5) * 0.5, pow(1.0 - max(abs(light_pos.y), 0.0), 15));
        sky_base_col += mix(vec3(0, 0, 0), vec3(0.4, 0.0, 0.1) * 0.5, pow(1.0 - max(abs(light_pos.y), 0.0), 24));
    }

    float bright_multiplier = 1.0;

    const float darker_pos = 0.0;
    if (light_pos.y < darker_pos)
    {
        float v = (light_pos.y - darker_pos) / (1 + darker_pos);
        bright_multiplier = clamp(pow(1 + v, 8), 0.2, 1.0);
    }

    sky_base_col *= bright_multiplier;
    if (dir.y >= 0.0)
    {
        // Sky with haze
        col = sky_base_col * (1.0 - 0.8 * dir.y) * 0.9;

        vec3 rdir = rotateVector(dir, light_pos, 3.1415926/4 + uTime * 0.003);

        // Sun
        {
            const float M = 17;
            const float P = 100;
            float vx = clamp(1 - pow((abs(light_pos.x - rdir.x)) * M, P), 0, 1);
            float vy = clamp(1 - pow((abs(light_pos.y - rdir.y)) * M, P), 0, 1);
            float vz = clamp(1 - pow((abs(light_pos.z - rdir.z)) * M, P), 0, 1);
            float sundot = clamp(vx*vy*vz, 0.0, 1.0);
            col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 1.0);
        }
        {
            const float M = 12;
            const float P = 3;
            float vx = clamp(1 - pow((abs(light_pos.x - rdir.x)) * M, P), 0, 1);
            float vy = clamp(1 - pow((abs(light_pos.y - rdir.y)) * M, P), 0, 1);
            float vz = clamp(1 - pow((abs(light_pos.z - rdir.z)) * M, P), 0, 1);
            float sundot = clamp(vx*vy*vz, 0.0, 1.0);
            col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 1.0);
        }
        float sundot = clamp(dot(dir, light_pos), 0.0, 1.0);
        col += 0.25 * vec3(1.0, 0.7, 0.4) * pow(sundot, 8.0);
        col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 64.0);

        // Clouds
        vec2 displace = vec2(0.5, 0.2) * uTime * 500.0;
        vec2 clouds_pos = pos.xz + displace;
        col = mix(col, vec3(1.0, 0.95, 1.0) * bright_multiplier, 0.5 * smoothstep(0.5, 0.8, fbm((clouds_pos + dir.xz * (50000.0 - pos.y) / dir.y) * 0.000008)));
        col = mix(col, vec3(1.0, 0.95, 1.0) * bright_multiplier, 0.5 * smoothstep(0.5, 0.8, fbm((clouds_pos + dir.xz * (250000.0 - pos.y) / dir.y) * 0.000008)));
    }

    // Horizon/atmospheric perspective
    col = mix(col, sky_base_col, pow(1.0 - max(abs(dir.y), 0.0), 8.0));

    return col;
}

void main()
{
    vec3 light_pos = normalize(-uSunLight.dir);
    vec3 pos = uCameraPos;
    vec3 dir = normalize(ioTexCoords);
    vec3 col = render(light_pos, pos, dir);

    // Gamma encode
    col = pow(col, vec3(1.1));

    FragColor = vec4(col, 1.0);
}
