uniform sampler2D u_sTexture0;
uniform highp vec2 u_block_size;
uniform highp vec2 u_texture_size;
varying highp vec2 v_v2TexCoord0;
       
void main()
{
	highp vec2 uv = v_v2TexCoord0;
	highp float dx = u_block_size.x * (1.0 / u_texture_size.x);
	highp float dy = u_block_size.y * (1.0 / u_texture_size.y);
	highp vec2 coord = clamp(vec2(dx*floor(uv.x / dx) + 1. / u_texture_size.x, dy*floor(uv.y / dy) + 1. / u_texture_size.y), 0.0, 1.0);
	gl_FragColor = texture2D(u_sTexture0, coord);
}