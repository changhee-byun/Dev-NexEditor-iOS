/******************************************************************************
* File Name   :	filters_boxblurH_vertex.glsl
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

uniform highp float rhombusOrSquare;

varying highp vec2 v_v2TexCoord0;
varying highp vec2 v_blurTexCoords[14];

void main()
{
    highp vec2 conv_texture_size = floor(u_texture_size);
    v_v2TexCoord0 = a_texCoord;
	if (rhombusOrSquare == 0.0)
	{
		v_blurTexCoords[0] = v_v2TexCoord0 + vec2(-7.0 / conv_texture_size.x, 7.0 / conv_texture_size.y);
		v_blurTexCoords[1] = v_v2TexCoord0 + vec2(-6.0 / conv_texture_size.x, 6.0 / conv_texture_size.y);
		v_blurTexCoords[2] = v_v2TexCoord0 + vec2(-5.0 / conv_texture_size.x, 5.0 / conv_texture_size.y);
		v_blurTexCoords[3] = v_v2TexCoord0 + vec2(-4.0 / conv_texture_size.x, 4.0 / conv_texture_size.y);
		v_blurTexCoords[4] = v_v2TexCoord0 + vec2(-3.0 / conv_texture_size.x, 3.0 / conv_texture_size.y);
		v_blurTexCoords[5] = v_v2TexCoord0 + vec2(-2.0 / conv_texture_size.x, 2.0 / conv_texture_size.y);
		v_blurTexCoords[6] = v_v2TexCoord0 + vec2(-1.0 / conv_texture_size.x, 1.0 / conv_texture_size.y);
		v_blurTexCoords[7] = v_v2TexCoord0 + vec2(1.0 / conv_texture_size.x, -1.0 / conv_texture_size.y);
		v_blurTexCoords[8] = v_v2TexCoord0 + vec2(2.0 / conv_texture_size.x, -2.0 / conv_texture_size.y);
		v_blurTexCoords[9] = v_v2TexCoord0 + vec2(3.0 / conv_texture_size.x, -3.0 / conv_texture_size.y);
		v_blurTexCoords[10] = v_v2TexCoord0 + vec2(4.0 / conv_texture_size.x, -4.0 / conv_texture_size.y);
		v_blurTexCoords[11] = v_v2TexCoord0 + vec2(5.0 / conv_texture_size.x, -5.0 / conv_texture_size.y);
		v_blurTexCoords[12] = v_v2TexCoord0 + vec2(6.0 / conv_texture_size.x, -6.0 / conv_texture_size.y);
		v_blurTexCoords[13] = v_v2TexCoord0 + vec2(7.0 / conv_texture_size.x, -7.0 / conv_texture_size.y);
	}
	else
	{
		v_blurTexCoords[0] = v_v2TexCoord0 + vec2(-7.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[1] = v_v2TexCoord0 + vec2(-6.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[2] = v_v2TexCoord0 + vec2(-5.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[3] = v_v2TexCoord0 + vec2(-4.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[4] = v_v2TexCoord0 + vec2(-3.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[5] = v_v2TexCoord0 + vec2(-2.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[6] = v_v2TexCoord0 + vec2(-1.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[7] = v_v2TexCoord0 + vec2(1.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[8] = v_v2TexCoord0 + vec2(2.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[9] = v_v2TexCoord0 + vec2(3.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[10] = v_v2TexCoord0 + vec2(4.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[11] = v_v2TexCoord0 + vec2(5.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[12] = v_v2TexCoord0 + vec2(6.0 / conv_texture_size.x, 0.0);
		v_blurTexCoords[13] = v_v2TexCoord0 + vec2(7.0 / conv_texture_size.x, 0.0);
	}
    gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
}
