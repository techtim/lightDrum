#version 330

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 textureMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 1) in vec4 color;
layout(location = 3) in vec2 text_coord;

out vec3 positionVarying;
out vec3 normalVarying;
out vec4 colorVarying;
out vec2 texCoordVarying;

void main()
{
    colorVarying = color;
    normalVarying = normal;
    texCoordVarying = text_coord;
    // texCoordVarying = (textureMatrix * vec4(text_coord.x, text_coord.y, 0, 1)).xy;

    vec4 pos = modelMatrix * vec4(position,1);
    positionVarying = pos.xyz;

    gl_Position = projectionMatrix * viewMatrix * pos;
}
