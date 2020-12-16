
varying highp vec2 v_texCoord;
uniform sampler2D u_textureSampler;
uniform highp vec2 u_velocity;
uniform highp float u_alpha;

void main(void)
{
	highp vec2 texcoord = v_texCoord;
	highp vec4 color = texture2D(u_textureSampler, texcoord);
	texcoord += u_velocity;
	for(int i = 1; i < 50; ++i, texcoord += u_velocity){

		highp float f = step(0.0, texcoord.y) * step(0.0, texcoord.x) * step(texcoord.y, 1.0) * step(texcoord.x, 1.0);
		color += (f * texture2D(u_textureSampler, texcoord));
	}

	color /= 50.0;
	gl_FragColor = color * u_alpha;
}