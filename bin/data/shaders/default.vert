#version 330

precision mediump float;
uniform mat4 modelViewProjectionMatrix;
in vec4 position;


void main(){
    gl_Position = modelViewProjectionMatrix * position;
}