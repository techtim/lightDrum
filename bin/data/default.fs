#version 430

// uniform float usingTexture;
// uniform float usingColors;
// uniform vec4 globalColor;
uniform vec3 cameraPosition;

in vec3 positionVarying;
in vec3 normalVarying;
in vec4 colorVarying;
in vec2 texCoordVarying;

#include "common_shaders/depth.h"

layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = colorVarying;
	gl_FragDepth = GetDepth(cameraPosition, positionVarying);
}
