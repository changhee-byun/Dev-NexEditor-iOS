/******************************************************************************
* File Name   :	filters_radialblur_resolfactor_vertex.glsl
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
attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform highp vec2 u_userInputPosition;
uniform highp mat4 u_tex_matrix;
uniform highp mat4 u_mvp_matrix;
uniform highp vec2 u_v2Resolution;
varying vec2 v_v2TexCoord0;
varying highp vec2 vResolutionRatioFactor;
varying vec2 v_userInputPosition;

void main()
{
	if(u_v2Resolution.x > u_v2Resolution.y)
	{
		vResolutionRatioFactor.x = 1.0;
		vResolutionRatioFactor.y = (u_v2Resolution.y / u_v2Resolution.x);
	}		
	else if(u_v2Resolution.x < u_v2Resolution.y)
	{
		vResolutionRatioFactor.x = (u_v2Resolution.x / u_v2Resolution.y);
		vResolutionRatioFactor.y = 1.0;
	}
	else
	{
		vResolutionRatioFactor.x = 1.0;
		vResolutionRatioFactor.y = 1.0;
	}
	gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
	v_v2TexCoord0 = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
	v_userInputPosition = (vec4(u_userInputPosition, 1.0, 1.0) * u_tex_matrix).st;
}
