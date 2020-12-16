package com.nexstreaming.kminternal.kinemaster.codeccolorformat;

import java.io.IOException;
import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaFormat;
import android.util.Log;
import android.view.Surface;

import com.nexstreaming.kminternal.kinemaster.config.LL;

public class WrapMediaDecoder extends WrapMediaCodec {
	
	static final String LOG_TAG = "WrapMediaDecoder";
	
	private WrapMediaListener wrapMediaListener;
	private int iFrameType = INFO_FRAME_TYPE_AUDIO;

	private MediaCodec mediaCodec;
	private Surface outSurface;
	
	private MediaFormat outputFormat;
	private int outputWidth;
	private int outputHeight;
	private ByteBuffer[] inputBuffers;
	private ByteBuffer[] outputBuffers;
	
	public WrapMediaDecoder(MediaFormat format, Surface surface) throws IOException{
		
		String mime = format.getString(MediaFormat.KEY_MIME);
		mediaCodec = MediaCodec.createDecoderByType(mime);
		if (mediaCodec == null) {
			if( LL.E ) Log.e(LOG_TAG, "Can't create media codec!");
			return;
		}
		mediaCodec.configure(format, surface, null, 0);
		
		if( mime.startsWith("video/") ) {
			outSurface = surface;
			iFrameType = INFO_FRAME_TYPE_VIDEO;
		}
		
		mediaCodec.start();	
		
		inputBuffers = mediaCodec.getInputBuffers();
		outputBuffers = mediaCodec.getOutputBuffers();
		
		outputWidth = format.getInteger(MediaFormat.KEY_WIDTH);
		outputHeight = format.getInteger(MediaFormat.KEY_HEIGHT);			
	}
	
	public void close() throws IOException {
		mediaCodec.stop();
		mediaCodec.release();
	}
 
	public boolean offerDecoder(byte[] input, long usTime) {
		
		if( mediaCodec == null || inputBuffers == null || outputBuffers == null || wrapMediaListener == null)
		{
			if( LL.E ) Log.e(LOG_TAG, "Media codec did not initailize");
			return false;
		}
		
		int inIndex = mediaCodec.dequeueInputBuffer(-1);
		if( inIndex >= 0 )
		{
			if( input == null || input.length <= 0 )
			{
				mediaCodec.queueInputBuffer(inIndex, 0, 0, usTime, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
				if( LL.D ) Log.d(LOG_TAG, String.format("WrapMediaDecoder EOS pass"));
			}
			else
			{
				ByteBuffer inputBuffer = inputBuffers[inIndex];
				inputBuffer.clear();
				inputBuffer.put(input);
				mediaCodec.queueInputBuffer(inIndex, 0, input.length, usTime, 0);
				if( LL.D ) Log.d(LOG_TAG, String.format("WrapMediaDecoder one frame pass time(%d)", usTime));
				
			}
		}
		
		processOutputbuffer();
			
		return true;
	}
	
	public boolean offerDecoder(ByteBuffer input, long usTime) {
		
		if( mediaCodec == null || inputBuffers == null || outputBuffers == null || wrapMediaListener == null)
		{
			if( LL.E ) Log.e(LOG_TAG, "Media codec did not initailize");
			return false;
		}
		
		int inIndex = mediaCodec.dequeueInputBuffer(10000);
		if( inIndex >= 0 )
		{
			if( input == null )
			{
				mediaCodec.queueInputBuffer(inIndex, 0, 0, usTime, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
				if( LL.D ) Log.d(LOG_TAG, String.format("WrapMediaDecoder EOS pass"));
			}
			else
			{
				ByteBuffer inputBuffer = inputBuffers[inIndex];
				inputBuffer.clear();
				inputBuffer.put(input);
				mediaCodec.queueInputBuffer(inIndex, 0, input.limit(), usTime, 0);
				if( LL.D ) Log.d(LOG_TAG, String.format("WrapMediaDecoder one frame pass"));
			}
		}
		processOutputbuffer();
		
		return true;
	}
	
	public boolean processOutputbuffer()
	{
		BufferInfo info = new BufferInfo();
		int outIndex = mediaCodec.dequeueOutputBuffer(info, 10000);
		if( LL.D ) Log.d(LOG_TAG, String.format("OutputBuffer outIndex(%d)", outIndex));
		switch (outIndex) {
		case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
			if( LL.D ) Log.d(LOG_TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
			outputBuffers = mediaCodec.getOutputBuffers();
			wrapMediaListener.flagReceived(INFO_OUTPUT_BUFFERS_CHANGED);
			break;
			
		case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
			outputFormat = mediaCodec.getOutputFormat();
			if( outputFormat != null )
			{
				outputWidth = outputFormat.getInteger(MediaFormat.KEY_WIDTH);
				outputHeight = outputFormat.getInteger(MediaFormat.KEY_HEIGHT);
			}
			if( LL.D ) Log.d(LOG_TAG, "New format " + outputFormat);
			wrapMediaListener.flagReceived(INFO_OUTPUT_FORMAT_CHANGED);
			break;
			
			
		case MediaCodec.INFO_TRY_AGAIN_LATER:
			if( LL.D ) Log.d(LOG_TAG, "dequeueOutputBuffer timed out!");
			wrapMediaListener.flagReceived(INFO_TRY_AGAIN_LATER);
			break;
			
		default:
			while(outIndex >= 0)
			{
				if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
					if( LL.D ) Log.d(LOG_TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
					wrapMediaListener.flagReceived(BUFFER_FLAG_END_OF_STREAM);
				}
				
				ByteBuffer buffer = outputBuffers[outIndex];
				if( buffer == null )
				{
					mediaCodec.releaseOutputBuffer(outIndex, true);
					if( LL.W ) Log.w(LOG_TAG, "OutputBuffer was not available");
					break;
				}
				
				if( info.size <= 0 )
				{
					mediaCodec.releaseOutputBuffer(outIndex, true);
					if( LL.W ) Log.w(LOG_TAG, "Output was not available!");
					break;
				}
				
				//We can't use this buffer but render it due to the API limit
				if( LL.D ) Log.d(LOG_TAG, "Can't use; render anyway, " + buffer);
				
				boolean bRender = false;
				if( outSurface != null )
				{
					bRender = wrapMediaListener.frameReceived(iFrameType, info.presentationTimeUs);
				}
				else
				{
					byte[] outData = new byte[info.size];
					buffer.get(outData);
					buffer.position(0);
					bRender = wrapMediaListener.frameReceived(outData, iFrameType, outData.length, info.presentationTimeUs);
				}
				
				mediaCodec.releaseOutputBuffer(outIndex, bRender);
				outIndex = mediaCodec.dequeueOutputBuffer(info, 10000);
			}
			break;
		}
		return true;
	}
	
	public void setOnWrapMediaListener(WrapMediaListener listener)
	{
		wrapMediaListener = listener;
	}
	
	public int getWidth()
	{
		return outputWidth;
	}
	
	public int getHeight()
	{
		return outputHeight;
	}
}
