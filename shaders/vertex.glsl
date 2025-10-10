#version 330 core
layout (location = 0) in vec4 vertex; // <pos>, <texCoords>

uniform mat4 model;
uniform mat4 projection;

out vec2 TexCoords;
out vec4 ParticleColor;

uniform vec4 particleColor; // 粒子颜色

void main()
{
    TexCoords = vertex.zw;
    ParticleColor = particleColor;
    vec4 pos = model * vec4(vertex.xy, 0.0, 1.0);
    gl_Position = projection * pos;
}