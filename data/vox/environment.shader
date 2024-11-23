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

vec3 render(in vec3 light, in vec3 ro, in vec3 rd, in float resolution) {
    vec3 col;

    if (rd.y < 0.0) {


    } else {
        // Sky with haze
        col = vec3(0.3, 0.55, 0.8) * (1.0 - 0.8 * rd.y) * 0.9;

        // Sun
        float sundot = clamp(dot(rd, light), 0.0, 1.0);
        col += 0.25 * vec3(1.0, 0.7, 0.4) * pow(sundot, 8.0);
        col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 64.0);

        // Clouds
        col = mix(col, vec3(1.0, 0.95, 1.0), 0.5 *
        smoothstep(0.5, 0.8, fbm((ro.xz + rd.xz * (250000.0 - ro.y) / rd.y) * 0.000008)));
    }

    // Horizon/atmospheric perspective
    col = mix(col, vec3(0.7, 0.75, 0.8), pow(1.0 - max(abs(rd.y), 0.0), 8.0));

    return col;
}

vec2 iMouse = vec2(0, 0);
float iTime = 0.0;
vec2 iResolution = vec2(1, 1);

void main()
{
    vec3 dir = normalize(ioTexCoords);
    vec2 fragCoord = dir.xy / 2 + vec2(0.5,0.5);

    const float verticalFieldOfView = 50.0 * 3.1415927 / 180.0;

    vec3 cameraOrigin = vec3(-iTime, sin(iTime) + 2.1, 0.0);
    float yaw = 2.0 * iMouse.x/iResolution.x;
    float pitch = -2.0 * (iMouse.y / iResolution.y - 0.5);

    if (iMouse.x == 0.0) {
        // Shadertoy starting position
        yaw = 1.0; pitch = 0.0;
    }

    vec3 ro = cameraOrigin;
    vec3 rd = normalize(vec3(fragCoord.xy - iResolution.xy / 2.0, iResolution.y * 0.5 / -tan(verticalFieldOfView * 0.5)));

    mat3 yawMatrix = mat3(cos(yaw), 0.0, -sin(yaw),
    0.0, 1.0, 0.0,
    sin(yaw), 0.0, cos(yaw));

    mat3 pitchMatrix = mat3(1.0, 0.0, 0.0,
    0.0, cos(pitch), sin(pitch),
    0.0, -sin(pitch), cos(pitch));

    rd = yawMatrix * pitchMatrix * rd;

    vec3 light = normalize(vec3(-0.8, 0.3, -0.3));

    vec3 col = render(light, ro, rd, iResolution.y);

    // Gamma encode
    col = pow(col, vec3(0.4545));

    FragColor = vec4(col, 1.0);
}
