uniform highp float u_alpha;
uniform highp mat4 u_color_matrix;
uniform highp vec2 u_real;
uniform highp vec2 u_texture_size;
varying highp vec2 v_v2TexCoord0;
varying highp vec2 v_blurTexCoords[14];
uniform sampler2D u_sTexture0;



void main(){

	highp float s = step(0.0, v_v2TexCoord0.y) * step(0.0, v_v2TexCoord0.x) * step(v_v2TexCoord0.y, u_real.y) * step(v_v2TexCoord0.x, u_real.x); // s == 1.0 if inside, 0.0 if outside (edge, value)
	if(s <= 0.0)
		discard;

	highp vec4 color;
	highp vec2 border = u_real - 1.0 / u_texture_size;
	color =  texture2D(u_sTexture0, clamp(v_v2TexCoord0, vec2(0,0), border)) * 0.159576912161;
	
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[0], vec2(0,0), border))*0.0044299121055113265;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[1], vec2(0,0), border))*0.00895781211794;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[2], vec2(0,0), border))*0.0215963866053;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[3], vec2(0,0), border))*0.0443683338718;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[4], vec2(0,0), border))*0.0776744219933;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[5], vec2(0,0), border))*0.115876621105;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[6], vec2(0,0), border))*0.147308056121;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[7], vec2(0,0), border))*0.147308056121;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[8], vec2(0,0), border))*0.115876621105;
	color += texture2D(u_sTexture0, clamp(v_blurTexCoords[9], vec2(0,0), border))*0.0776744219933;
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[10], vec2(0,0), border))*0.0443683338718;
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[11], vec2(0,0), border))*0.0215963866053;
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[12], vec2(0,0), border))*0.00895781211794;
	color +=  texture2D(u_sTexture0, clamp(v_blurTexCoords[13], vec2(0,0), border))*0.0044299121055113265;

	color = color * u_color_matrix;
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color.rgba * u_alpha;
}