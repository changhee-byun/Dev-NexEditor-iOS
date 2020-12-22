attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform highp mat4 u_tex_matrix;
uniform highp mat4 u_mvp_matrix;
varying vec2 v_v2TexCoord0;

void main()
{
	gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
	v_v2TexCoord0 = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
}