
varying highp vec2 v_texCoord;
uniform sampler2D u_textureSampler;
uniform highp float u_alpha;

void main(void)
{
	highp vec2 texcoord = v_texCoord;
	highp vec4 color = texture2D(u_textureSampler, texcoord);
	gl_FragColor = color * u_alpha;
}