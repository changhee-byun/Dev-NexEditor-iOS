uniform sampler2D u_sTexture0;
varying highp vec2 v_v2TexCoord0;

void main() 
{
	highp vec4 color = (texture2D(u_sTexture0, v_v2TexCoord0)).bgra;
	highp float gray = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
	gl_FragColor = vec4(gray, gray, gray, color.a);
}