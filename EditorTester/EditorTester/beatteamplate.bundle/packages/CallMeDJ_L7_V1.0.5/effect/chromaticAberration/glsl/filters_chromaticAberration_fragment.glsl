/******************************************************************************
* File Name   :	filters_chromaticAberration_fragment.glsl
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
uniform highp mat4 u_color_matrix;
uniform highp float u_strength;
uniform highp float u_qualityLevel;
uniform highp float u_bOriAlpha;
varying highp vec2 v_v2TexCoord;


highp vec2 getbarrelDist(highp vec2 uv, highp float adj) {
	highp vec2 rr = uv - 0.5;
	highp float dist = dot(rr, rr);
	return uv + rr * dist * adj;
}


highp float linterp(highp  float t ) {
	highp float val = 1.0 - abs( 2.0*t - 1.0 );
	return clamp( val, 0.0, 1.0 );
}

highp float remap(highp float t) {
	highp float a = 1.0/6.0;
	highp float b = 5.0/6.0;
	highp float val = (t - a)/(b - a) ;
	return clamp( val, 0.0, 1.0 );
}

highp vec4 spectrumOffset(highp float t ) {
	highp vec4 ret;
	highp float lo = step(t,0.5);
	highp float hi = 1.0-lo;
	highp float w = linterp(remap(t));
	ret = vec4(lo,1.0,hi, 1.0) * vec4(1.0-w, w, 1.0-w, 1.0);

	return pow( ret, vec4(1.0/2.2, 1.0 / 2.2, 1.0 / 2.2, 1.0) );
}


void main()
{	
	highp vec4 sumT = vec4(0.0);
	highp vec4 sumoff = vec4(0.0);	

	highp float damp = 1.0 / u_qualityLevel;
	highp float i;
	if (u_strength == 0.5)
	{
		sumT = texture2D(u_sTexture0, v_v2TexCoord);
		sumoff = vec4(1.0);
	}
	else
	{
		for (i = 0.0; i < u_qualityLevel; i += 1.0)
		{
			highp float t = i * damp;
			highp vec4 off = spectrumOffset(t);
			sumoff += off;
			sumT += off * texture2D(u_sTexture0, getbarrelDist(v_v2TexCoord, 0.6 * u_strength*t));
		}
	}
		
	highp vec4 final = sumT / sumoff;
	if(u_bOriAlpha != 1.0)
		final.a = 1.0;
	
	final = final * u_color_matrix;

	gl_FragColor =  final * u_alpha;
}
