attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform highp mat4 u_mvp_matrix;
varying highp vec2 v_texCoord;

void main() {
	
	v_texCoord = a_texCoord;
	gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
}