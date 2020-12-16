
function main()
    video_src = kmInitVideoSrc()
	src_tex_id = system.video_left.id

	aspect_ratio = kmGetOutputWidth() / kmGetOutputHeight()
	virtual_width = aspect_ratio * 720
	virtual_height = 720

	src_w = kmGetSrcWidth(system.video_left.id) * virtual_width / kmGetOutputWidth()
	src_h = kmGetSrcHeight(system.video_left.id) * virtual_height / kmGetOutputHeight()
	tex_w = kmGetTexWidth(system.video_left.id) * virtual_width / kmGetOutputWidth()
	tex_h = kmGetTexHeight(system.video_left.id) * virtual_height / kmGetOutputHeight()

    RBFx.setV2SrcResolution(src_w, src_h)
	RBFx.setV2Resolution(tex_w, tex_h)
  
	RBFx.setSTexture0(src_tex_id)
	RBFx.setAlpha(effect.alpha)
	RBFx.setColorMatrix(system.video_left.colorconv)
	RBFx.setTexMatrix(system.video_left.texmat)
	RBFx.setMvpMatrix(effect.matrix)
    RBFx.setDirAlpha(range.diralpha * 0.1)          -- 0.1 ~ 3.0 (default : 0.5)
    RBFx.setUserInputStrength(range.strength * 0.1) -- 1.0 ~ 5.0 (default : 2.0)
    RBFx.setUserInputPosition(range.centerX*0.01, range.centerY*0.01)
	RBFx.setBOriAlpha(range.bOriAlpha * 1.0)
    RBFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end