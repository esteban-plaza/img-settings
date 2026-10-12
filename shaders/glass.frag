#version 440

// Liquid Glass surface: samples a blurred copy of what is behind the panel,
// bends it near the rounded edge like a thick lens, adds a touch of chromatic
// dispersion, a tint, and a specular rim.

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 itemSize;    // panel size in px
    vec2 texSize;     // blurred backdrop size in px (panel + 2 * pad)
    float pad;        // extra backdrop captured around the panel
    float radius;     // corner radius
    float refraction; // max edge displacement in px (<= pad)
    float edgeWidth;  // width of the lens bevel in px
    float rimStrength;
    vec4 tint;        // rgb + mix amount
};

layout(binding = 1) uniform sampler2D source;

float sdRoundRect(vec2 p, vec2 halfSize, float r)
{
    vec2 q = abs(p) - halfSize + vec2(r);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

vec3 backdrop(vec2 local)
{
    return texture(source, (local + vec2(pad)) / texSize).rgb;
}

void main()
{
    vec2 halfSize = itemSize * 0.5;
    vec2 local = qt_TexCoord0 * itemSize;
    vec2 p = local - halfSize;
    float r = min(radius, min(halfSize.x, halfSize.y));
    float d = sdRoundRect(p, halfSize, r); // < 0 inside

    vec2 e = vec2(0.5, 0.0);
    vec2 grad = vec2(sdRoundRect(p + e.xy, halfSize, r) - sdRoundRect(p - e.xy, halfSize, r),
                     sdRoundRect(p + e.yx, halfSize, r) - sdRoundRect(p - e.yx, halfSize, r));
    vec2 n = grad / max(length(grad), 1e-4); // outward normal

    // Circular bevel profile: 1 at the rim, 0 once past the bevel.
    float depth = clamp(-d / edgeWidth, 0.0, 1.0);
    float t = 1.0 - depth;
    float bend = 1.0 - sqrt(max(1.0 - t * t, 0.0));

    // Pull in light from just outside the edge, as a thick lens would.
    vec2 offset = n * bend * refraction;
    vec3 col = vec3(backdrop(local + offset * 1.12).r,
                    backdrop(local + offset).g,
                    backdrop(local + offset * 0.88).b);

    col = mix(col, tint.rgb, tint.a);

    // Specular rim, brightest where the edge faces the light (top-left)
    // with a softer bounce on the opposite side.
    vec2 light = normalize(vec2(-0.55, -0.85));
    float facing = dot(n, light);
    float band = 1.0 - smoothstep(0.0, 1.6, abs(d + 0.9));
    float rim = band * (0.22 + 0.78 * pow(abs(facing), 1.6) * (facing > 0.0 ? 1.0 : 0.55));
    float glow = pow(t, 3.0) * 0.10;
    col += vec3(rim + glow) * rimStrength;
    col += vec3(0.035) * rimStrength * (1.0 - qt_TexCoord0.y);

    float alpha = clamp(0.5 - d, 0.0, 1.0);
    fragColor = vec4(col * alpha, alpha) * qt_Opacity;
}
