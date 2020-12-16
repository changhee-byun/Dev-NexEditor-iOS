
function main()
    video_src = kmInitVideoSrc()
	src_tex_id = system.video_left.id
	CCFx.setSTexture0(src_tex_id)
    CCFx.setTime(system.play_percentage *1.0 )
    CCFx.setStrength(range.strength *0.1)
    CCFx.setQuality(range.quality *1.0)
	CCFx.setBOriAlpha(range.bOriAlpha * 1.0)
	CCFx.setDirection(range.direction*1.0)

	aspect_ratio = kmGetOutputWidth() / kmGetOutputHeight()
	virtual_width = aspect_ratio * 720
	virtual_height = 720
	CCFx.setV2Resolution(kmGetTexWidth(src_tex_id) * virtual_width / kmGetOutputWidth(), kmGetTexHeight(src_tex_id) * virtual_height / kmGetOutputHeight())


	CCFx.setAlpha(effect.alpha)
	CCFx.setColorMatrix(system.video_left.colorconv)
	CCFx.setTexMatrix(system.video_left.texmat)
	CCFx.setMvpMatrix(effect.matrix)
	CCFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end