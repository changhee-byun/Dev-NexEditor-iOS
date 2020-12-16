/******************************************************************************
* File Name   :	filters_radialblur_fragment.glsl
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
uniform highp vec2 u_v2Resolution;
uniform highp vec2 u_v2SrcResolution;
uniform highp float u_dirAlpha; //0.5;
uniform highp float u_userInputStrength; //default 2
uniform highp float u_bOriAlpha;

varying highp vec2 v_v2TexCoord0;
varying highp vec2 vResolutionRatioFactor;
varying highp vec2 v_userInputPosition;


void main(void)
{
	highp float dirfactor[10];
	dirfactor[0] = -0.08;
	dirfactor[1] = -0.05;
	dirfactor[2] = -0.03;
	dirfactor[3] = -0.02;
	dirfactor[4] = -0.01;
	dirfactor[5] = 0.01;
	dirfactor[6] = 0.02;
	dirfactor[7] = 0.03;
	dirfactor[8] = 0.05;
	dirfactor[9] = 0.08;

	highp float userX = (v_userInputPosition.x * (u_v2SrcResolution.x / u_v2Resolution.x) - v_v2TexCoord0.x) * vResolutionRatioFactor.x;
	highp float userY = (v_userInputPosition.y * (u_v2SrcResolution.y / u_v2Resolution.y) - v_v2TexCoord0.y) * vResolutionRatioFactor.y;

	highp vec2 dir = vec2(userX, userY);

	highp float dist = sqrt(userX*userX + userY*userY);
	dir = dir / dist;


	highp vec4 baseCol = texture2D(u_sTexture0, v_v2TexCoord0);
	highp vec4 sum = baseCol;

	for (int i = 0; i < 10; i++)
		sum += texture2D(u_sTexture0, v_v2TexCoord0 + dir * dirfactor[i] * u_dirAlpha);

	sum *= 1.0 / 11.0;

	highp float t = dist * u_userInputStrength;

	t = clamp(t, 0.0, 1.0);

	highp vec4 color = mix(baseCol, sum, t);

	if(u_bOriAlpha != 1.0)
		color.a = 1.0;

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);
	gl_FragColor = color * u_alpha;

}
