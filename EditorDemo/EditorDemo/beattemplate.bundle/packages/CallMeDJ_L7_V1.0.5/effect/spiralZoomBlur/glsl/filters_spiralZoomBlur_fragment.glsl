uniform sampler2D u_sTexture0;uniform highp float u_alpha;uniform highp mat4 u_color_matrix;uniform highp vec2 u_v2Resolution;uniform highp vec2 u_v2SrcResolution;uniform highp vec2 u_userInputPosition;uniform highp float u_userInputRadius;uniform highp float u_userInputStrength;uniform highp float u_enableZoomOut;uniform highp float u_qualityLevl;varying highp vec2 v_v2TexCoord0;highp float random(highp vec2 scale)
{
	return fract(  sin( dot(v_v2TexCoord0,scale) )*43758.5453 );
}void main(){	highp vec2 textureCoordinateToUse = v_v2TexCoord0;	highp float userX = u_userInputPosition.x * (u_v2SrcResolution.x / u_v2Resolution.x);	highp float userY = u_userInputPosition.y * (u_v2SrcResolution.y / u_v2Resolution.y);	highp vec2 userXY = vec2(userX,userY);	highp float dist = distance(userXY, v_v2TexCoord0);	highp float radius = u_userInputRadius;	if (dist < radius)	{		textureCoordinateToUse -= userXY;		highp float percent = (radius - dist) / radius;		highp float theta = percent * percent * u_userInputStrength*36.0;		highp float s = sin(theta);		highp float c = cos(theta);		textureCoordinateToUse = vec2(dot(textureCoordinateToUse, vec2(c, -s)), dot(textureCoordinateToUse, vec2(s, c)));		textureCoordinateToUse += userXY;	}
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
	final.rgb /= max(final.a,0.00001);	final = final * u_color_matrix;	final = clamp(final, 0.0, 1.0);	gl_FragColor =  final * u_alpha;}