uniform sampler2D u_sTexture0;
{
	return fract(  sin( dot(v_v2TexCoord0,scale) )*43758.5453 );
}
	highp vec2 pos = textureCoordinateToUse;
	highp vec4 color = vec4(0.0);
	highp float total = 0.0;
	highp vec2 toCenter = u_userInputPosition - pos;
	highp float offset = random( vec2(12.9898,78.233) );

	highp float alpha;
	if(u_enableZoomOut == 1.0)
		alpha = 8.0*(u_userInputStrength)/0.003 ;
	else //zoom in
		alpha = 8.0*(-u_userInputStrength)/0.009 ;

	highp float strength = (-u_userInputStrength)*alpha;
	
	highp float loopCnt = u_qualityLevl;
	highp float t;
	for(t=0.0; t<=loopCnt; t+= 1.0)
	{
		highp float percent = (t+offset)/loopCnt;
		highp float weight = 4.0*(percent-percent*percent);
		highp vec4 sample = texture2D(u_sTexture0, pos+toCenter*percent*strength);
		sample.rgb *= sample.a;
		color += sample*weight;
		total += weight;
	}

	highp vec4 final;
	final = color/total;
	final.rgb /= max(final.a,0.00001);