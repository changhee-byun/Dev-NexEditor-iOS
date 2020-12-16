
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

			BHFx.setMvpMatrix(id_matrix)
			BHFx.setAlpha(1.0)
			BHFx.setColorMatrix(id_matrix)
			BHFx.setTextureSize(tex_w / block_size, tex_h / block_size)
			BHFx.setReal(src_w/tex_w, src_h/tex_h)
			BHFx.setSTexture0(first_pass_tex)
			BHFx.setTexMatrix(id_matrix)
			BHFx.drawRect(2, 2)

			third_pass_tex = kmSetRenderToClearTexture(tex_w , tex_h,4)
			BVFx.setAlpha(1.0)
			BVFx.setColorMatrix(id_matrix)
			BVFx.setMvpMatrix(id_matrix)
			BVFx.setTextureSize(tex_w / block_size, tex_h / block_size)
			BVFx.setReal(src_w/tex_w, src_h/tex_h)
			BVFx.setSTexture0(second_pass_tex)
			BVFx.setTexMatrix(id_matrix)
			BVFx.drawRect(2, 2)
      
      
			kmSetRenderToDefault()      
      
			BLFx.setSTexture0(third_pass_tex)
			src_tex_id = system.video_left.id
			BLFx.setSTexture1(src_tex_id)
			BLFx.setAlpha(effect.alpha)
			BLFx.setColorMatrix(system.video_left.colorconv)
			BLFx.setBOriAlpha(range.bOriAlpha * 1.0)

			bSize = math.floor(range.f_block_size*1.0)
			if bSize == 0.0 then
				BLFx.setStrengthZeroFlag(1.0)
			else
				BLFx.setStrengthZeroFlag(0.0)
			end

			BLFx.setTexMatrix(system.video_left.texmat)
			BLFx.setMvpMatrix(effect.matrix)
            BLFx.drawScreen(video_src)
            kmReleaseTexture(video_src)
end