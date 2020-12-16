/******************************************************************************
* File Name   :	filters_motionblurH_vertex.glsl
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
uniform highp mat4 u_mvp_matrix;
uniform highp vec2 u_texture_size;

uniform highp float u_angle;
uniform highp float u_startPoint;

varying highp vec2 v_v2TexCoord0;
varying highp vec2 v_blurTexCoords[15];


const highp float M_PI = 3.1415926535897932384626433832795;


void main()
{
    highp vec2 conv_texture_size = floor(u_texture_size);
    v_v2TexCoord0 = a_texCoord;

	highp float alphaX;
	highp float alphaY;
	highp float sign = 1.0;
	if (u_angle == 0.0 || u_angle == 180.0)
	{
		alphaX = 1.0;
		alphaY = 0.0;
	}
	else if (u_angle == 90.0 || u_angle == 270.0)
	{
		alphaX = 0.0;
		alphaY = 1.0;
	}
	else
	{
		if( ((u_angle >45.0) && (u_angle <= 135.0)) || ((u_angle >225.0) && (u_angle <= 315.0)) )
		{
			alphaX = 1.0 / tan(u_angle*M_PI / 180.0);
			alphaY = 1.0;
		}
		else
		{
			alphaX = 1.0;
			alphaY = tan(u_angle*M_PI / 180.0);
		}
	}
	if ((u_angle > 135.0) && (u_angle <=315.0) )
		sign = -1.0;

	v_blurTexCoords[0] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 7.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 7.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[1] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 6.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 6.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[2] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 5.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 5.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[3] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 4.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 4.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[4] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 3.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 3.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[5] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 2.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 2.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[6] =  v_v2TexCoord0 + vec2((u_startPoint*sign - 1.0) / conv_texture_size.x * alphaX, (u_startPoint*sign - 1.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[8] =  v_v2TexCoord0 + vec2((u_startPoint*sign + 1.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 1.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[9] =  v_v2TexCoord0 + vec2((u_startPoint*sign + 2.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 2.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[10] = v_v2TexCoord0 + vec2((u_startPoint*sign + 3.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 3.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[11] = v_v2TexCoord0 + vec2((u_startPoint*sign + 4.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 4.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[12] = v_v2TexCoord0 + vec2((u_startPoint*sign + 5.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 5.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[13] = v_v2TexCoord0 + vec2((u_startPoint*sign + 6.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 6.0) / conv_texture_size.y*alphaY);
	v_blurTexCoords[14] = v_v2TexCoord0 + vec2((u_startPoint*sign + 7.0) / conv_texture_size.x * alphaX, (u_startPoint*sign + 7.0) / conv_texture_size.y*alphaY);

	v_blurTexCoords[7] = v_v2TexCoord0 + vec2((u_startPoint) / conv_texture_size.x * alphaX, (u_startPoint) / conv_texture_size.y*alphaY);

    gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
}
