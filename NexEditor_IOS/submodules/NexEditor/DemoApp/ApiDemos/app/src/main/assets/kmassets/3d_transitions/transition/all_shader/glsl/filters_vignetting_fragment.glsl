uniform sampler2D u_sTexture0;
uniform highp vec2 u_v2Resolution;
uniform highp vec2 u_v2SrcResolution;
varying highp vec2 v_v2TexCoord0;
const highp float RADIUS = 0.85;
const highp float SOFTNESS = 0.6;
const highp float uUserInputStrength = 0.5;

void main()
{
	highp vec4 ori = texture2D(u_sTexture0, v_v2TexCoord0);
	highp vec2 position = (vec2(0.5) * (u_v2SrcResolution / u_v2Resolution)) - (v_v2TexCoord0.xy);
	highp float len = length(position);
	highp float modi_radius = (1.0 - uUserInputStrength);
	highp float vignette = smoothstep(modi_radius, modi_radius-SOFTNESS, len);

	gl_FragColor = vec4(mix(ori.rgb, ori.rgb * vignette, 0.9),ori.a);
}