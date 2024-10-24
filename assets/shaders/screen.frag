#version 430 core

in vec2 v_tex_coord;
out vec4 o_frag_color;

uniform sampler2D u_tex;

void main()
{
    o_frag_color = texture(u_tex, v_tex_coord);
}
