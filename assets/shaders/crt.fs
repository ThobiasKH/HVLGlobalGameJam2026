#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;

uniform float curvature;
uniform float scanlineIntensity;
uniform float vignette;
uniform float ditherStrength;

uniform float chromAberration;
uniform float jitterStrength;
uniform float jitterSpeed;

uniform vec2 resolution;

void main() {
    vec2 uv = fragTexCoord;

    // --- Subpixel jitter (screen wobble) ---
    float jitter = sin(time * jitterSpeed) * jitterStrength;
    uv.x += jitter / resolution.x;
    uv.y += cos(time * jitterSpeed * 0.7) * jitterStrength / resolution.y;

    // --- CRT curvature ---
    uv = uv * 2.0 - 1.0;
    float r = dot(uv, uv);
    uv *= 1.0 + curvature * r * 2.0;
    uv = uv * 0.5 + 0.5;

    // --- Chromatic aberration (RGB split) ---
    float ca = chromAberration / resolution.x;

    vec4 colR = texture(texture0, uv + vec2( ca, 0.0));
    vec4 colG = texture(texture0, uv);
    vec4 colB = texture(texture0, uv - vec2( ca, 0.0));

    vec4 color = vec4(colR.r, colG.g, colB.b, 1.0);

    // --- Scanlines ---
    float scanline = sin(uv.y * resolution.y * 3.14159);
    color.rgb -= scanline * scanlineIntensity;

    // --- Pixel-locked dithering ---
    vec2 pixel = floor(uv * resolution);
    float dither = fract(sin(dot(pixel, vec2(12.9898, 78.233))) * 43758.5453);
    color.rgb += (dither - 0.5) * ditherStrength;

    // --- Vignette ---
    float dist = distance(uv, vec2(0.5));
    color.rgb *= smoothstep(0.85, vignette, dist);

    finalColor = color;
}
