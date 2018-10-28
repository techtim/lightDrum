#version 330
precision mediump float;
vec4 texture2D(sampler2D tex, vec2 coords) { return texture(tex, coords.xy); }
/// vec4 texture2DCube(sampler2DCube tex, vec2 coords) { return texture(tex, coords); }

uniform vec4 drawOffset;
uniform vec3 iResolution;
uniform float iGlobalTime;
uniform float iTime;
uniform int iFrame;
uniform vec4 color1;
uniform vec4 color2;
uniform float velocity;
uniform float envelope;

out vec4 FragColor;

float tailLength = 0.3;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main(void)
{
    /// xy position of texture to address 0 to 1 space in fragment
    vec2 uv = (gl_FragCoord.xy-drawOffset.xy) / (iResolution.xy);
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    float val = iResolution.x > iResolution.y ? uv.x : uv.y;

    color = mix(color1, color2, envelope);
    float env = map(envelope, 0, 1, -tailLength, 1+tailLength);
    color.w = 1 - abs(env - val) / tailLength;

    FragColor = color;
}