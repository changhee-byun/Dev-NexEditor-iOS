/******************************************************************************
* File Name   :	filters_motionblur_fragment.glsl
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
uniform highp vec2 u_border;
uniform highp vec2 u_texture_size;
varying highp vec2 v_v2TexCoord0;
varying highp vec2 v_blurTexCoords[15];
uniform sampler2D u_sTexture0;

void main(){
	highp vec4 color;
	highp vec2 border = u_border - 1.0 / u_texture_size;
	
	color = texture2D(u_sTexture0, clamp(v_blurTexCoords[0], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[1], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[2], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[3], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[4], vec2(0,0), border));
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[5], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[6], vec2(0,0), border));
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[7], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[8], vec2(0,0), border));
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[9], vec2(0, 0), border));
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[10], vec2(0,0), border));
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[11], vec2(0,0), border)) ;
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[12], vec2(0,0), border));
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[13], vec2(0,0), border)) ;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[14], vec2(0, 0), border)) ;

	color /= 15.0;

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color * u_alpha;
}