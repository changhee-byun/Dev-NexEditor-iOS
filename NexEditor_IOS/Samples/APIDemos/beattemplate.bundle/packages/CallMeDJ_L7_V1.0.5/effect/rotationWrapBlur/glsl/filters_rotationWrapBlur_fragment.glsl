/******************************************************************************
* File Name   :	filters_rotationWrapBlur_fragment.glsl
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
uniform highp float u_posX;
//uniform highp float u_posY;
uniform highp float u_angle;
uniform highp float u_step;
uniform highp float u_userInputStrength;
uniform highp float u_ellipse;
uniform highp float u_bOriAlpha;
varying highp vec2 v_v2TexCoord0;
varying highp vec2 v_userInputPosition;


void main()
{
	highp vec2 textureCoordinateToUse;
	highp vec4 color = vec4(0.0);
	highp float angle = u_angle;
	highp float step = min(abs(u_angle) / 45.0, u_step);

	highp vec2 modded;
	highp vec2 modded1;
	highp float s;
	highp float c;

	highp float i;

	highp vec2 vResolutionRatioFactor;

	if (u_v2Resolution.x > u_v2Resolution.y)
	{
		vResolutionRatioFactor.x = 1.0;
		vResolutionRatioFactor.y = (u_v2Resolution.y / u_v2Resolution.x);
	}
	else if (u_v2Resolution.x < u_v2Resolution.y)
	{
		vResolutionRatioFactor.x = (u_v2Resolution.x / u_v2Resolution.y);
		vResolutionRatioFactor.y = 1.0;
	}
	else
	{
		vResolutionRatioFactor.x = 1.0;
		vResolutionRatioFactor.y = 1.0;
	}
	highp float cnt = 0.0;
	for (i = -u_userInputStrength; i< u_userInputStrength; i += 1.0)
	{
		angle = u_angle;
		angle += step*i;

		s = sin(angle / 180.0 * 3.14);
		c = cos(angle / 180.0 * 3.14);

		{
			
			highp vec2 userXY = vec2(v_userInputPosition.x, v_userInputPosition.y);

			textureCoordinateToUse = v_v2TexCoord0;
			textureCoordinateToUse -= userXY;

			if (u_ellipse == 0.0)
				textureCoordinateToUse *= vResolutionRatioFactor;


			textureCoordinateToUse = vec2(dot(textureCoordinateToUse, vec2(c, s)), dot(textureCoordinateToUse, vec2(-s, c)));

			if (u_ellipse == 0.0)
				textureCoordinateToUse *= (1.0/vResolutionRatioFactor);

			textureCoordinateToUse += userXY;

			if (u_angle >= 0.0)
			{
				modded = mod(textureCoordinateToUse, 1.0);
				if (textureCoordinateToUse.x <= 0.0)
					modded = vec2(1.0 - modded.x, modded.y);
				if (textureCoordinateToUse.y <= 0.0)
					modded = vec2(modded.x, 1.0 - modded.y);

				if (textureCoordinateToUse.x >= 1.0)
					modded = vec2(1.0 - modded.x, modded.y);
				if (textureCoordinateToUse.y >= 1.0)
					modded = vec2(modded.x, 1.0 - modded.y);
			}
			else
			{
				modded = mod(textureCoordinateToUse, 1.0);
				if (textureCoordinateToUse.x >= 1.0)
					modded = vec2(1.0 - modded.x, modded.y);
				if (textureCoordinateToUse.y >= 1.0)
					modded = vec2(modded.x, 1.0 - modded.y);

				if (textureCoordinateToUse.x <= 0.0)
					modded = vec2(1.0 - modded.x, modded.y);
				if (textureCoordinateToUse.y <= 0.0)
					modded = vec2(modded.x, 1.0 - modded.y);
			}
		}

		{
			highp vec2 userXY1 = vec2(2.0, 0.5);
			textureCoordinateToUse = v_v2TexCoord0;
			textureCoordinateToUse -= userXY1;

			if (u_ellipse == 0.0)
				textureCoordinateToUse *= vResolutionRatioFactor;

			textureCoordinateToUse = vec2(dot(textureCoordinateToUse, vec2(c, -s)), dot(textureCoordinateToUse, vec2(s, c)));
			if (u_ellipse == 0.0)
				textureCoordinateToUse *= (1.0/ vResolutionRatioFactor);
			textureCoordinateToUse += userXY1;

			if (u_angle >= 0.0)
			{
				modded1 = mod(textureCoordinateToUse, 1.0);
				if (textureCoordinateToUse.x >= 1.0)
					modded1 = vec2(1.0 - modded1.x, modded1.y);
				if (textureCoordinateToUse.y <= 0.0)
					modded1 = vec2(modded.x, 1.0 - modded1.y);
			}
			else
			{
				modded1 = mod(textureCoordinateToUse, 1.0);
				if (textureCoordinateToUse.x >= 1.0)
					modded1 = vec2(1.0 - modded1.x, modded1.y);
				if (textureCoordinateToUse.y >= 1.0)
					modded1 = vec2(modded.x, 1.0 - modded1.y);
			}

		}
		highp float alpha = smoothstep(u_posX, 1.0, v_v2TexCoord0.x);
		highp vec2 uv = mix(modded, modded1, alpha);
		uv = mod(uv, 1.0);

		color += texture2D(u_sTexture0, uv);

		cnt += 1.0;

	}

	//color /= ((u_userInputStrength*2.0) + 1.0);
	color /= cnt;

	if(u_bOriAlpha == 0.0)
		color.a = 1.0;

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color * u_alpha;
}
