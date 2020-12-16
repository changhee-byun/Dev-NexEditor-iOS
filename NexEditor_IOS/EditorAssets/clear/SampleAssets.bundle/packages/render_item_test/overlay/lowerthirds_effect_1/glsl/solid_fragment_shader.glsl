
varying highp vec2 v_texCoord;
uniform highp vec4 u_user_color;
uniform highp float u_alpha;

void main(void)
{
	highp vec4 color = u_user_color + vec4(v_texCoord.x, v_texCoord.y, 0.0, 0.0) * 0.0001;
	gl_FragColor = color * u_alpha;
}