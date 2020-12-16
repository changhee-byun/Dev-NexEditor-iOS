/******************************************************************************
* File Name   :	filters_blurLast_fragment.glsl
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
uniform highp mat4 u_color_matrix;
uniform sampler2D u_sTexture0;
uniform sampler2D u_sTexture1;

uniform highp float u_bOriAlpha;
uniform highp float u_strengthZeroFlag;

varying highp vec2 v_v2TexCoord;


void main() {
	highp vec4 color = texture2D(u_sTexture0, v_v2TexCoord);

	if(u_strengthZeroFlag == 1.0)
		color = texture2D(u_sTexture1, v_v2TexCoord);

	if (u_bOriAlpha == 0.0)
	{
		//highp float alpha = texture2D(u_sTexture1, v_v2TexCoord).a;
		//color = mix(vec4(0.0), color, alpha);
		color.a = 1.0;
	}

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color * u_alpha;
}
