#version 430 core

layout (location = 0) in vec2 a_vert_coord;

void main()
{
    gl_Position = a_vert_coord;
}
