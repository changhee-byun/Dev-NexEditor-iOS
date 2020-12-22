/******************************************************************************
* File Name   :	filters_zoomMirror_fragment.glsl
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

uniform highp float u_bOriAlpha;
uniform highp float u_zoom;
uniform highp float u_bOriMix;
uniform highp float u_mixMode;

uniform highp vec2 u_v2Resolution;
uniform highp mat4 u_color_matrix;

varying highp vec2 v_v2TexCoord;


highp vec4 zoomMirror_fn(highp vec2 fragCoord)
{
	highp vec4 retCol;
	highp float hap = -0.5;

	highp vec2 uv = fragCoord / u_v2Resolution;
	uv.xy += hap;

	highp vec2 uv2 = uv;
	uv = (uv.xy - hap) + uv2 * u_zoom;

	highp float modVal;
	highp float modOdd;
	
	if (uv.x > 1.0)
	{
		modVal = floor(uv.x);
		modOdd = mod(modVal, 2.0);
		if(modOdd == 1.0) //if odd
			uv.x = 1.0 - fract(uv.x);
		else //if even
			uv.x = fract(uv.x);
	}

	if (uv.x < 0.0)
	{
		highp float absVal = -uv.x+1.0;
		modVal = floor(absVal);
		modOdd = mod(modVal, 2.0);
		if (modOdd == 0.0) //if odd
			uv.x = 1.0 - fract(absVal - 1.0);
		else //if even
			uv.x = fract(absVal - 1.0);
	}

	if (uv.y > 1.0)
	{
		modVal = floor(uv.y);
		modOdd = mod(modVal, 2.0);
		if (modOdd == 1.0) //if odd
			uv.y = 1.0 - fract(uv.y);
		else //if even
			uv.y = fract(uv.y);
	}

	if (uv.y < 0.0)
	{
		highp float absVal = -uv.y + 1.0;
		modVal = floor(absVal);
		modOdd = mod(modVal, 2.0);
		if (modOdd == 0.0) //if odd
			uv.y = 1.0 - fract(absVal - 1.0);
		else //if even
			uv.y = fract(absVal - 1.0);
	}
		
	retCol = texture2D(u_sTexture0, uv);

	return retCol;
}


void main(void) {
	highp vec4 color;
	highp vec4 zoomCol;
	highp vec4 oriCol;

	{
		zoomCol = zoomMirror_fn(v_v2TexCoord*u_v2Resolution);
		if (u_bOriMix == 1.0)
		{
			oriCol = texture2D(u_sTexture0, v_v2TexCoord);
			if(u_mixMode == 0.0)
				color = min(oriCol, zoomCol);
			else
				color = max(oriCol, zoomCol);
		}
		else
			color = zoomCol;
	}

	if (u_bOriAlpha == 0.0)
		color.a = 1.0;

	color = color * u_color_matrix;

	gl_FragColor = color * u_alpha;

}







