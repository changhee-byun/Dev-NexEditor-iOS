/******************************************************************************
* File Name   :	filters_motionblurlast_fragment.glsl
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
uniform highp float u_alpha;
uniform highp float u_strengthZeroFlag;
uniform highp float u_bOriAlpha;
uniform highp mat4 u_color_matrix;
uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;

varying highp vec2 v_v2TexCoord0;


void main() {
	highp vec4 color;
	if (u_strengthZeroFlag == 1.0)
		color = texture2D(u_sTexture1, v_v2TexCoord0);
	else
		color = texture2D(u_sTexture0, v_v2TexCoord0);

	/*if (u_bOriAlpha == 1.0)
	{
		highp float alpha = texture2D(u_sTexture1, v_v2TexCoord0).a;
		if (alpha < 1.0)
			color = vec4(0.0);
	}
	else*/
	if (u_bOriAlpha == 0.0)
		color.a = 1.0;

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color * u_alpha;

}

