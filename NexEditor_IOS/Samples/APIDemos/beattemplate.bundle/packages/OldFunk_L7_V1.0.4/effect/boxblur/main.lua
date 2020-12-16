
function main()
	video_src = kmInitVideoSrc()
	block_size = math.max(range.f_block_size, 1)
			
	aspect_ratio = kmGetOutputWidth() / kmGetOutputHeight()
	virtual_width = aspect_ratio * 720
	virtual_height = 720

	src_w = kmGetSrcWidth(system.video_left.id) * virtual_width / kmGetOutputWidth()
	src_h = kmGetSrcHeight(system.video_left.id) * virtual_height / kmGetOutputHeight()
	tex_w = kmGetTexWidth(system.video_left.id) * virtual_width / kmGetOutputWidth()
	tex_h = kmGetTexHeight(system.video_left.id) * virtual_height / kmGetOutputHeight()


	first_pass_tex = kmSetRenderToClearTexture(tex_w / block_size, tex_h / block_size,4)
	id_matrix = kmNewMatrix()
	kmLoadIdentity(id_matrix)
	kmDrawTexBox(system.video_left.id, id_matrix, -1, 1, 1, -1, 0, 0)
	second_pass_tex = kmSetRenderToClearTexture(tex_w / block_size, tex_h / block_size,4)

	BEHFx.setMvpMatrix(id_matrix)
	BEHFx.setAlpha(1.0)
	BEHFx.setColorMatrix(id_matrix)
	BEHFx.setTextureSize(tex_w / block_size, tex_h / block_size)
	BEHFx.setBorder(src_w/tex_w, src_h/tex_h)
	BEHFx.setSTexture0(first_pass_tex)
	--BEHFx.setBOriAlpha(range.bOriAlpha * 1.0)
    BEHFx.setRhombusOrSquare(range.rhombusOrSquare*1.0) --0:rhombus 1:square
	BEHFx.drawRect(2, 2)

	third_pass_tex = kmSetRenderToClearTexture(tex_w , tex_h,4)
	BEVFx.setAlpha(1.0)
	BEVFx.setColorMatrix(id_matrix)
	BEVFx.setMvpMatrix(id_matrix)
	BEVFx.setTextureSize(tex_w / block_size, tex_h / block_size)
	BEVFx.setBorder(src_w/tex_w, src_h/tex_h)
	BEVFx.setSTexture0(second_pass_tex)
	--BEVFx.setBOriAlpha(range.bOriAlpha * 1.0)
    BEVFx.setRhombusOrSquare(range.rhombusOrSquare*1.0) --0:rhombus 1:square
	BEVFx.setTexMatrix(id_matrix)
	--BEVFx.drawBound(effect.left, effect.bottom, effect.right, effect.top)
    BEVFx.drawRect(2, 2)
      
    kmSetRenderToDefault()      
      
    BEFx.setSTexture0(third_pass_tex)
    src_tex_id = system.video_left.id
	BEFx.setSTexture1(src_tex_id)
	BEFx.setBOriAlpha(range.bOriAlpha * 1.0)

	bSize = math.floor(range.f_block_size*1.0)
	if bSize == 0.0 then
		BEFx.setStrengthZeroFlag(1.0)
	else
		BEFx.setStrengthZeroFlag(0.0)
	end
    BEFx.setAlpha(effect.alpha)
	BEFx.setColorMatrix(system.video_left.colorconv)
    BEFx.setTexMatrix(system.video_left.texmat)
	BEFx.setMvpMatrix(effect.matrix)
    BEFx.drawScreen(video_src)
    kmReleaseTexture(video_src)
end