/******************************************************************************
* File Name   :	filters_spectrumGlitch_fragment.glsl
* Description :
*******************************************************************************
* Copyright (c) 2002-2019 KineMaster Corp. All rights reserved.
* http://www.kinemaster.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/
uniform sampler2D u_sTexture0;
uniform highp float u_alpha;
uniform highp float u_time;
uniform highp float u_strength;
uniform highp float u_quality;
uniform highp float u_direction;//0:all, 1: right, 2:left
uniform highp float u_bOriAlpha;
uniform highp vec2 u_v2Resolution;
uniform highp mat4 u_color_matrix;

varying highp vec2 v_v2TexCoord;

highp float satfloat(highp float t) {
	return clamp(t, 0.0, 1.0);
}


//remaps inteval [a;b] to [0;1]
highp float remap(highp float t, highp float a, highp float b) {
	return satfloat((t - a) / (b - a));
}

highp float linterp(highp float t) {
	return satfloat(1.0 - abs(2.0*t - 1.0));
}

highp vec3 spectrum_offset(highp float t) {
	highp vec3 ret;
	highp float lo = step(t, 0.5);
	highp float hi = 1.0 - lo;
	highp float w = linterp(remap(t, 1.0 / 6.0, 5.0 / 6.0));
	highp float neg_w = 1.0 - w;
	ret = vec3(lo, 1.0, hi) * vec3(neg_w, w, neg_w);
	return pow(ret, vec3(1.0 / 2.2));
}

highp float rand(highp vec2 n) {
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

highp float srand(highp vec2 n) {
	return rand(n) * 2.0 - 1.0;
}

highp float mytrunc(highp float x, highp float num_levels)
{
	return floor(x*num_levels) / num_levels;
}
highp vec2 mytrunc(highp vec2 x, highp float num_levels)
{
	return floor(x*num_levels) / num_levels;
}

highp vec4 spGlitch_fn(highp vec2 Coord)
{
	highp vec4 retColor;
	highp vec2 uv = Coord.xy / u_v2Resolution.xy;

	highp float time = mod(u_time*100.0, 32.0) / 110.0;

	highp float GLITCH = u_strength;

	highp float gnm = satfloat(GLITCH);
	highp float rnd0 = rand(mytrunc(vec2(time, time), 6.0));
	highp float r0 = satfloat((1.0 - gnm)*0.7 + rnd0);
	highp float rnd1 = rand(vec2(mytrunc(uv.x, 10.0*r0), time)); //horz

	highp float r1 = 0.5 - 0.5 * gnm + rnd1;
	r1 = 1.0 - max(0.0, ((r1<1.0) ? r1 : 0.9999999)); //note: weird ass bug on old drivers
	highp float rnd2 = rand(vec2(mytrunc(uv.y, 40.0*r1), time)); //vert
	highp float r2 = satfloat(rnd2);

	highp float rnd3 = rand(vec2(mytrunc(uv.y, 10.0*r0), time));
	//highp float r3 = (1.0 - satfloat(rnd3 + 0.8)) - 0.1;

	highp float pxrnd = rand(uv + time);

	highp float ofs = 0.05 * r2 * GLITCH * (rnd0 > 0.5 ? 1.0 : -1.0);
	if(u_direction == 1.0)
		ofs = abs(ofs);
	else if(u_direction == 2.0)
		ofs = -abs(ofs);
	ofs += 0.5 * pxrnd * ofs;

	//uv.y += 0.1 * r3 * GLITCH;

	highp float RCP_NUM_SAMPLES_F = 1.0 / u_quality;

	highp vec4 sum = vec4(0.0);
	highp vec3 wsum = vec3(0.0);

	highp float i;
	for ( i= 0.0; i<u_quality; i+=1.0)
	{
		highp float t = i * RCP_NUM_SAMPLES_F;
		uv.x = satfloat(uv.x + ofs * t);
		uv.y = satfloat(uv.y);
		highp vec4 samplecol = texture2D(u_sTexture0, uv);
		highp vec3 s = spectrum_offset(t);
		samplecol.rgb = samplecol.rgb * s;
		sum += samplecol;
		wsum += s;
	}
	sum.rgb /= wsum;

	if(u_bOriAlpha == 1.0)
		retColor.a = texture2D(u_sTexture0, v_v2TexCoord).a;
	else
		retColor.a = 1.0;

	retColor.rgb = sum.rgb;

	return retColor;
}


void main()
{
	highp vec4 color;
	
	color = spGlitch_fn(v_v2TexCoord*u_v2Resolution);
	
	color = color * u_color_matrix;

	gl_FragColor =  color * u_alpha;
}
