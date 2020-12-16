
function main()
    video_src = kmInitVideoSrc()
	src_tex_id = system.video_left.id
	BPFx.setSTexture0(src_tex_id)
	BPFx.setAlpha(effect.alpha)
	BPFx.setColorMatrix(system.video_left.colorconv)
	BPFx.setTexMatrix(system.video_left.texmat)
	BPFx.setMvpMatrix(effect.matrix)
    BPFx.setStrength(range.strength*0.3 + 0.5)
    BPFx.setQualityLevel(range.qualityLevel * 1.0 + 6.0);
	BPFx.setBOriAlpha(range.bOriAlpha * 1.0)
	BPFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end