package com.nexstreaming.kminternal.kinemaster.codeccolorformat;

import java.io.IOException;
import java.nio.ByteBuffer;

import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;


public class WrapMediaEncoder extends WrapMediaCodec {
	
	static final String LOG_TAG = "WrapMediaEncoder";
	
	private WrapMediaListener wrapMediaListener;
	
	private MediaCodec mediaCodec;
	
//	private MediaFormat mediaFormat;	
	// private Surface inputSurface;
	
	private byte[] sps;
	private byte[] pps;
	
	private ByteBuffer[] inputBuffers;
	private ByteBuffer[] outputBuffers;
	
	public static abstract class OnEncodedFrameListener {
		public abstract void frameReceived(byte [] outData, int iType, int iLength);
	}
	
	public static abstract class OnParameterSetsListener {
		public abstract void avcParametersSetsEstablished(byte[] sps, byte[] pps);
	}	
	
	public static abstract class OnEncodedFlagListener {
		public abstract boolean flagReceived(int iFlag);
	}	
	
	public WrapMediaEncoder() throws IOException{
		mediaCodec = MediaCodec.createEncoderByType("video/avc");
		MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", 1280, 720);
		mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 125000);
		mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
		mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);
		mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);
		mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
		
		// inputSurface = mediaCodec.createInputSurface();
		mediaCodec.start();		
		inputBuffers = mediaCodec.getInputBuffers();
		outputBuffers = mediaCodec.getOutputBuffers();
	}
	
	public WrapMediaEncoder(int iWidth, int iHeight) throws IOException{
		mediaCodec = MediaCodec.createEncoderByType("video/avc");
		MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", iWidth, iHeight);
		mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 125000);
		mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
		mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar);
		mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);
		mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
		
		// inputSurface = mediaCodec.createInputSurface();
		mediaCodec.start();		
		inputBuffers = mediaCodec.getInputBuffers();
		outputBuffers = mediaCodec.getOutputBuffers();
	}
	
	public WrapMediaEncoder(int iWidth, int iHeight, int iEncodeFormat) throws IOException{
		mediaCodec = MediaCodec.createEncoderByType("video/avc");
		MediaFormat mediaFormat = MediaFormat.createVideoFormat("video/avc", iWidth, iHeight);
		mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 125000);
		mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
		mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, iEncodeFormat);
		mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 5);
		mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
		
		// inputSurface = mediaCodec.createInputSurface();
		mediaCodec.start();		
		inputBuffers = mediaCodec.getInputBuffers();
		outputBuffers = mediaCodec.getOutputBuffers();
	}	

	public void close() throws IOException {
		mediaCodec.stop();
		mediaCodec.release();
	}
 
	public boolean offerEncoder(byte[] input, long usTime) {
		
		if( mediaCodec == null || inputBuffers == null || outputBuffers == null || wrapMediaListener == null)
		{
			if( LL.W ) Log.w(LOG_TAG, "Media codec did not initailize");
			return false;
		}
		
		ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
		ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
		int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);
		if (inputBufferIndex >= 0) {
			ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
			inputBuffer.clear();

			if (input != null) {
				inputBuffer.put(input);
				mediaCodec.queueInputBuffer(inputBufferIndex, 0, input.length,
						usTime, 0);
			} else {
				mediaCodec.queueInputBuffer(inputBufferIndex, 0, 0, usTime,
						MediaCodec.BUFFER_FLAG_END_OF_STREAM);
			}
		}

		BufferInfo info = new BufferInfo();
		int outIndex = mediaCodec.dequeueOutputBuffer(info, 10000);
		switch (outIndex) {
		case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
			if( LL.D ) Log.d(LOG_TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
			outputBuffers = mediaCodec.getOutputBuffers();
			wrapMediaListener
					.flagReceived(INFO_OUTPUT_BUFFERS_CHANGED);
			break;
		case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
			if( LL.D ) Log.d(LOG_TAG, "New format " + mediaCodec.getOutputFormat());
			// mediaFormat = mediaCodec.getOutputFormat();
			wrapMediaListener
					.flagReceived(INFO_OUTPUT_FORMAT_CHANGED);
			break;
		case MediaCodec.INFO_TRY_AGAIN_LATER:
			if( LL.D ) Log.d(LOG_TAG, "dequeueOutputBuffer timed out!");
			wrapMediaListener
					.flagReceived(INFO_TRY_AGAIN_LATER);
			break;
		default:

			while (outIndex >= 0) {
				if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
					if( LL.D ) Log.d(LOG_TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
					wrapMediaListener
							.flagReceived(BUFFER_FLAG_END_OF_STREAM);
				}

				ByteBuffer outputBuffer = outputBuffers[outIndex];
				if (outputBuffer == null) {
					if( LL.W ) Log.w(LOG_TAG, "Output buffer is null!");
					break;
				}

				if (info.size <= 0) {
					if( LL.W ) Log.w(LOG_TAG, "Output was not available!");
					break;
				}

				if( LL.D ) Log.d(LOG_TAG, String.format(
						"Output was available Falg:%d Size:%d", info.flags,
						info.size));
				outputBuffer.position(info.offset);
				outputBuffer.limit(info.offset + info.size);

				if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
					if( LL.D ) Log.d(LOG_TAG, "OutputBuffer BUFFER_FLAG_CODEC_CONFIG");

					byte[] outData = new byte[info.size];
					outputBuffer.get(outData);

					ByteBuffer spsPpsBuffer = ByteBuffer.wrap(outData);
					if (spsPpsBuffer.getInt() == 0x00000001) {
						System.out.println("parsing sps/pps");
					} else {
						System.out.println("something is amiss?");
					}
					int ppsIndex = 0;
					while (!(spsPpsBuffer.get() == 0x00
							&& spsPpsBuffer.get() == 0x00
							&& spsPpsBuffer.get() == 0x00 && spsPpsBuffer.get() == 0x01)) {
					}
					ppsIndex = spsPpsBuffer.position();
					sps = new byte[ppsIndex - 8 + 4];
					sps[0] = 0;
					sps[1] = 0;
					sps[2] = 0;
					sps[3] = 1;
					System.arraycopy(outData, 4, sps, 4, sps.length - 4);

					pps = new byte[outData.length - ppsIndex + 4];
					pps[0] = 0;
					pps[1] = 0;
					pps[2] = 0;
					pps[3] = 1;
					System.arraycopy(outData, ppsIndex, pps, 4, pps.length - 4);
					wrapMediaListener.configReceived(sps, pps);

					mediaCodec.releaseOutputBuffer(outIndex, false);
					outIndex = mediaCodec.dequeueOutputBuffer(info, 10000);
					continue;
				}

				// wrapMediaListener.frameReceived(outputBuffer,
				// OnWrapMediaListener.INFO_FRAME_TYPE_VIDEO,
				// info.presentationTimeUs);

				byte[] outData = new byte[info.size];
				outputBuffer.get(outData);
				wrapMediaListener.frameReceived(outData,
						INFO_FRAME_TYPE_VIDEO,
						outData.length, info.presentationTimeUs);

				mediaCodec.releaseOutputBuffer(outIndex, false);
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
}
