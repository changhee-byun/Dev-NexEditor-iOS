uniform sampler2D u_sTexture0;
uniform highp vec2 u_v2Resolution;
varying highp vec2 v_v2TexCoord0;

highp float BlendExclusionf(highp float base, highp float blend)
{
	return (base + blend - 2.0 * base * blend); 
}

highp vec3 BlendExclusion(highp vec3 base, highp vec3 blend)
{
	return vec3(BlendExclusionf(base.r, blend.r), BlendExclusionf(base.g, blend.g), BlendExclusionf(base.b, blend.b));
}

highp vec3 BlendExclusionbgr(highp vec3 base, highp vec3 blend)
{
	return vec3(BlendExclusionf(base.b, blend.b), BlendExclusionf(base.g, blend.g), BlendExclusionf(base.r, blend.r));
}

void main()
{
	highp vec2 offset = vec2(1.0 / u_v2Resolution);
	highp vec4 ori =  texture2D(u_sTexture0, v_v2TexCoord0).rgba;

	highp vec2 tc2 = v_v2TexCoord0 + vec2(0.0, -2.0*offset.y);

	highp vec2 tc6 = v_v2TexCoord0 + vec2(-offset.x, -offset.y);
	highp vec2 tc7 = v_v2TexCoord0 + vec2(0.0, -offset.y);
	highp vec2 tc8 = v_v2TexCoord0 + vec2(+offset.x, -offset.y);

	highp vec2 tc10 = v_v2TexCoord0 + vec2(-2.0*offset.x, 0.0);
	highp vec2 tc11 = v_v2TexCoord0 + vec2(-offset.x, 0.0);
	highp vec2 tc12 = v_v2TexCoord0 + vec2(0.0, 0.0);
	highp vec2 tc13 = v_v2TexCoord0 + vec2(+offset.x, 0.0);
	highp vec2 tc14 = v_v2TexCoord0 + vec2(+2.0*offset.x, 0.0);

	highp vec2 tc16 = v_v2TexCoord0 + vec2(-offset.x, +offset.y);
	highp vec2 tc17 = v_v2TexCoord0 + vec2(0.0, +offset.y);
	highp vec2 tc18 = v_v2TexCoord0 + vec2(+offset.x, +offset.y);

	highp vec2 tc22 = v_v2TexCoord0 + vec2(0.0, +2.0*offset.y);


	highp vec3 col2 = (texture2D(u_sTexture0, tc2).rgb)/13.0;

	highp vec3 col6 = (texture2D(u_sTexture0, tc6).rgb)/13.0;
	highp vec3 col7 = (texture2D(u_sTexture0, tc7).rgb)/13.0;
	highp vec3 col8 = (texture2D(u_sTexture0, tc8).rgb)/13.0;

	highp vec3 col10 = (texture2D(u_sTexture0, tc10).rgb)/13.0;
	highp vec3 col11 = (texture2D(u_sTexture0, tc11).rgb)/13.0;
	highp vec3 col13 = (texture2D(u_sTexture0, tc13).rgb)/13.0;
	highp vec3 col14 = (texture2D(u_sTexture0, tc14).rgb)/13.0;

	highp vec3 col16 = (texture2D(u_sTexture0, tc16).rgb)/13.0;
	highp vec3 col17 = (texture2D(u_sTexture0, tc17).rgb)/13.0;
	highp vec3 col18 = (texture2D(u_sTexture0, tc18).rgb)/13.0;

	highp vec3 col22 = (texture2D(u_sTexture0, tc22).rgb)/13.0;

	highp vec3 blur = (ori.rgb/13.0) + col2 + col6 + col7 + col8 + col10 + col11 + col13 + col14 + col16 + col17 + col18 + col22;

	highp vec3 unsharp = ori.rgb*2.0 - blur.rgb;

	gl_FragColor = vec4(BlendExclusion(ori.rgb , unsharp.rgb*0.5), ori.a);

}