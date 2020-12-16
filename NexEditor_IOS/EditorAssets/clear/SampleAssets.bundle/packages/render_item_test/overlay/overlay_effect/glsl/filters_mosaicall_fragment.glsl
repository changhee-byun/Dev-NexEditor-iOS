uniform sampler2D u_sTexture0;
varying highp vec2 v_v2TexCoord0;
       
void main()
{
	highp vec2 uTexel = vec2(0.25 , 0.25);
    highp float uScale = 0.10;
	highp float dx = uScale * uTexel.x;
	highp float dy = uScale * uTexel.y;
	highp vec2 coord = vec2(floor(v_v2TexCoord0.x/dx)*dx + (dx/2.0) , floor(v_v2TexCoord0.y/dy)*dy + (dy/2.0) );
	coord.x = clamp(coord.x, 0.0, 1.0);
	coord.y = clamp(coord.y, 0.0, 1.0);
	gl_FragColor = texture2D(u_sTexture0, coord).rgba;
}