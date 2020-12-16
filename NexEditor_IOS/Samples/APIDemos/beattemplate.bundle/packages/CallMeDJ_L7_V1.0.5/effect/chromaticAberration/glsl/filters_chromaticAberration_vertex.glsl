/******************************************************************************
* File Name   :	filters_chromaticAberration_vertex.glsl
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
uniform highp mat4 u_tex_matrix;
uniform highp mat4 u_mvp_matrix;
varying vec2 v_v2TexCoord;

void main()
{
	gl_Position = vec4(a_position.xyz, 1) * u_mvp_matrix;
	v_v2TexCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
}