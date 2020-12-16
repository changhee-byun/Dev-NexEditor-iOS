
function main()
    video_src = kmInitVideoSrc()
	src_tex_id = system.video_left.id
	CCFx.setSTexture0(src_tex_id)

	aspect_ratio = kmGetOutputWidth() / kmGetOutputHeight()
	virtual_width = aspect_ratio * 720
	virtual_height = 720
	CCFx.setV2Resolution(kmGetTexWidth(src_tex_id) * virtual_width / kmGetOutputWidth(), kmGetTexHeight(src_tex_id) * virtual_height / kmGetOutputHeight())

	CCFx.setBOriAlpha(range.bOriAlpha * 1.0)
	CCFx.setAlpha(effect.alpha)
	CCFx.setZoom(range.zoom*0.1)
	CCFx.setBOriMix(range.oriMix*1.0)
	CCFx.setMixMode(range.mixMode * 1.0)

	CCFx.setColorMatrix(system.video_left.colorconv)
	CCFx.setTexMatrix(system.video_left.texmat)
	CCFx.setMvpMatrix(effect.matrix)
    CCFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end