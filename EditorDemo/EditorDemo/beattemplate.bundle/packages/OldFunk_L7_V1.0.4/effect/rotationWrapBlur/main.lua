
function main()
    video_src = kmInitVideoSrc()
	src_tex_id = system.video_left.id

	aspect_ratio = kmGetOutputWidth() / kmGetOutputHeight()
	virtual_width = aspect_ratio * 720
	virtual_height = 720
	BPFx.setV2Resolution(kmGetTexWidth(src_tex_id) * virtual_width / kmGetOutputWidth(), kmGetTexHeight(src_tex_id) * virtual_height / kmGetOutputHeight())

	BPFx.setSTexture0(src_tex_id)
	BPFx.setAlpha(effect.alpha)
	BPFx.setColorMatrix(system.video_left.colorconv)
	BPFx.setTexMatrix(system.video_left.texmat)
	BPFx.setMvpMatrix(effect.matrix)
    BPFx.setPosX(range.posx*0.1);
    BPFx.setUserInputStrength(range.strength * 1.0)
    BPFx.setUserInputPosition(range.centerX*0.01, range.centerY*0.01)
    BPFx.setStep(range.step*0.01)
    BPFx.setAngle(range.angle*1.0)
    BPFx.setEllipse(range.ellipse*1.0)
	BPFx.setBOriAlpha(range.bOriAlpha * 1.0)
    BPFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end