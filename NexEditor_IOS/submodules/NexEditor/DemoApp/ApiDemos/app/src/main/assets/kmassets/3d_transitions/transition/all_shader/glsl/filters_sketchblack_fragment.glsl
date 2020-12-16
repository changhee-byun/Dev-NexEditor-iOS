uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;
uniform highp vec2 u_v2Resolution;
varying highp vec2 v_v2TexCoord0;
void main()
{
	highp vec4 luminance = vec4(0.3, 0.59, 0.11, 0.0);
	highp vec4 mask = texture2D(u_sTexture1, v_v2TexCoord0);
	highp vec2 n = 1.0 / u_v2Resolution.xy;
	highp vec4 CC = texture2D(u_sTexture0, v_v2TexCoord0);
	highp vec4 RD = texture2D(u_sTexture0, v_v2TexCoord0 + vec2(n.x, -n.y));
	highp vec4 RC = texture2D(u_sTexture0, v_v2TexCoord0 + vec2(n.x, 0.0));
	highp vec4 RU = texture2D(u_sTexture0, v_v2TexCoord0 + n);
	highp vec4 LD = texture2D(u_sTexture0, v_v2TexCoord0 - n);
	highp vec4 LC = texture2D(u_sTexture0, v_v2TexCoord0 - vec2(n.x, 0.0));
	highp vec4 LU = texture2D(u_sTexture0, v_v2TexCoord0 - vec2(n.x, -n.y));
	highp vec4 CD = texture2D(u_sTexture0, v_v2TexCoord0 - vec2(0.0, n.y));
	highp vec4 CU = texture2D(u_sTexture0, v_v2TexCoord0 + vec2(0.0, n.y));
	highp float result = 2.0 * abs(length(vec2(-abs(dot(luminance, RD - LD))
													+4.0 * abs(dot(luminance, RC - LC))
													-abs(dot(luminance, RU - LU)),
													-abs(dot(luminance, LD - LU))
													+4.0 * abs(dot(luminance, CD - CU))
													-abs(dot(luminance, RD - RU))
													)
									) - .5);
	if(result < 0.9)
	{
		result = 0.0;
	}
	highp float gray = 0.299 * CC.r + 0.587 * CC.g + 0.114 * CC.b;
	highp vec4 gray_rgb = vec4(gray);
	highp vec4 res = vec4(result)*0.5+gray_rgb*0.5;
	gl_FragColor = vec4(mask.rgb*0.3+res.rgb*0.7, CC.a);
}