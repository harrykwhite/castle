#version 430 core

in vec2 v_tex_coord;

out vec4 o_frag_color;

uniform vec4 u_blend;
uniform sampler2D u_tex;

void main()
{
    vec4 tex_color = texture(u_tex, v_tex_coord);
    o_frag_color = tex_color * u_blend;
}
