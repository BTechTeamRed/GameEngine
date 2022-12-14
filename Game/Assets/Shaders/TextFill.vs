#version 330 core
layout (location = 0) in vec3 pos;
layout(location = 1) in vec2 vertexUV;
out vec2 TexCoords;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(pos.xy, 0.0, 1.0);
    TexCoords = vertexUV;
}  