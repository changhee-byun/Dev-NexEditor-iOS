package com.nexstreaming.kminternal.kinemaster.codeccolorformat;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Vector;

import android.content.Context;
import android.content.SharedPreferences;
import android.media.MediaFormat;
import android.os.AsyncTask;
import android.preference.PreferenceManager;
import android.util.Log;

import com.nexstreaming.app.common.task.ResultTask;
import com.nexstreaming.app.common.task.Task;
import com.nexstreaming.app.common.task.Task.TaskError;
import com.nexstreaming.kminternal.kinemaster.config.LL;

public class ColorFormatChecker {
	
	private static final String LOG_TAG = "ColorFormatChecker";
	private static ResultTask<ColorFormat> sColorFormatTask = null;
	private static final String PREF_KEY = "km_detected_codec_color_format";
	public static final String EDITOR_PROP_EXPORT_COLOR_FORMAT = "setExportColorFormat";
		
	public static ResultTask<ColorFormat> checkColorFormat(Context ctx) {
		
		if( sColorFormatTask==null ) {
			sColorFormatTask = new ResultTask<ColorFormat>();
			
			final SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(ctx);
			if(pref.contains(PREF_KEY)) {
				String colorFormat = pref.getString(PREF_KEY, null);
				for( ColorFormat fmt: ColorFormat.values() ) {
					if(fmt.name().equals(colorFormat)) {
						sColorFormatTask.setResult(fmt);
						return sColorFormatTask;
					}
				}
			}
			
			new AsyncTask<Void, Void, ColorFormat>() {
				
				TaskError mError = null;

				@Override
				protected ColorFormat doInBackground(Void... params) {
					try {
						ColorFormatChecker checker = new ColorFormatChecker();
						return checker.checkColorFormatInternal();
					} catch( final Exception e ) {
						mError = Task.makeTaskError("Error getting color format", e);
						return null;
					}
				}
				
				@Override
				protected void onPostExecute(ColorFormat result) {
					if( result!=null ) {
						pref.edit().putString(PREF_KEY, result.name()).commit();
						sColorFormatTask.sendResult(result);
					} else {
						sColorFormatTask.sendFailure(mError);
					}
					super.onPostExecute(result);
				}
			}.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, ((Void)null));

		}
		return sColorFormatTask;
	}
	
	public enum ColorFormat{
		UNKNOWN, NV12, NV21
	}
	
	private WrapMediaEncoder mWrapMediaEncoder;
	private WrapMediaDecoder mWrapMediaDecoder;
	private boolean mEncodeReceivedEOS = false;
	private boolean mDecodeReceivedEOS = false;
	
	private MediaFormat mEncoderFormat;
	
	private Vector<byte[]> mEncodedFrameVec;
	private Vector<byte[]> mDecodedFrameVec;
	
	private ColorFormatChecker() {
	}
	
	private ColorFormat checkColorFormatInternal() throws IOException
	{
		ColorFormat iFormat = ColorFormat.UNKNOWN;
		mEncodedFrameVec = new Vector<byte[]>(); 
		mDecodedFrameVec = new Vector<byte[]>();
		
		mWrapMediaEncoder = new WrapMediaEncoder(1280,720);
		mWrapMediaEncoder.setOnWrapMediaListener(new WrapMediaListener(){
			public boolean frameReceived(int iType, long usTime)
			{
				return true;
			}
			public boolean frameReceived(ByteBuffer buffer, int iType, long usTime)
			{
				if( iType == WrapMediaCodec.INFO_FRAME_TYPE_CONFIG )
				{
					// ByteBuffer tmp = buffer.duplicate();
					if(mEncoderFormat == null )
						mEncoderFormat = MediaFormat.createVideoFormat("video/avc", 1280, 720);
					// mEncoderFormat.setByteBuffer("csd-0", tmp);
					if( LL.D ) Log.d(LOG_TAG, String.format("Encoder Frame config Received(Type:%d)", iType));
				}
				else if( iType == WrapMediaCodec.INFO_FRAME_TYPE_VIDEO )
				{
					if( LL.D ) Log.d(LOG_TAG, String.format("Encoder Frame buffer Received(Type:%d)", iType));
				}
				// new DumpArray(buffer);
				return true;
			}
			public boolean frameReceived(byte [] outData, int iType, int iLength, long usTime)
			{
				mEncodedFrameVec.add(outData);
				if( LL.D ) Log.d(LOG_TAG, String.format("Encoder One Frame Received(Type:%d, Length:%d, Time:%d)", iType, iLength, usTime));
				return true;
			}
			public boolean configReceived(byte [] sps, byte [] pps)
			{
				if(mEncoderFormat == null )
					mEncoderFormat = MediaFormat.createVideoFormat("video/avc", 1280, 720);
				
				ByteBuffer bufferSPS = ByteBuffer.wrap(sps);
				ByteBuffer bufferPPS = ByteBuffer.wrap(pps);
				
				mEncoderFormat.setByteBuffer("csd-0", bufferSPS);
				mEncoderFormat.setByteBuffer("csd-1", bufferPPS);
				if( LL.D ) Log.d(LOG_TAG, String.format("Encoder Frame config Received"));
				return true;
			}
			public boolean flagReceived(int iFlag)
			{
				if( LL.D ) Log.d(LOG_TAG, "Receive flag from Encoder : " + WrapMediaCodec.flagToString(iFlag));
				if( iFlag == WrapMediaCodec.BUFFER_FLAG_END_OF_STREAM )
				{
					mEncodeReceivedEOS = true;
				}
				return true;				
			}
		});
		
		byte[] inputSrc = new byte[1280*720*3/2];
		
		Arrays.fill(inputSrc, 0, 1280*720, (byte)0x2D);
		for(int i = 0; i < 1280*720 / 2; i += 2)
		{
			inputSrc[1280*720 + i] = (byte)0xD4;
			inputSrc[1280*720 + i + 1] = (byte)0x81;
		}
		
		
		long usTime = 0;
		for(int i = 0; i<5; i++)
		{
			mWrapMediaEncoder.offerEncoder(inputSrc, usTime);
			usTime += 30000;
		}			
		
		while(mEncodeReceivedEOS == false)
			mWrapMediaEncoder.offerEncoder(null, usTime);
		
		try {
			mWrapMediaEncoder.close();
			mWrapMediaEncoder = null;
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		if( mEncoderFormat != null )
		{
			mWrapMediaDecoder = new WrapMediaDecoder(mEncoderFormat, null);
			
			mWrapMediaDecoder.setOnWrapMediaListener(new WrapMediaListener(){
				public boolean frameReceived(int iType, long usTime)
				{
					return true;
				}
				public boolean frameReceived(ByteBuffer buffer, int iType, long usTime)
				{
					return true;
				}
				public boolean frameReceived(byte [] outData, int iType, int iLength, long usTime)
				{
					if( mDecodedFrameVec.size() <= 0 )
					{
						mDecodedFrameVec.add(outData);
					}
					if( LL.D ) Log.d(LOG_TAG, String.format("Decoder One Frame Received(Type:%d, Length:%d, Time:%d)", iType, iLength, usTime));
					return true;
				}
				public boolean configReceived(byte [] sps, byte [] pps)
				{
					return true;
				}
				public boolean flagReceived(int iFlag)
				{
					if( LL.D ) Log.d(LOG_TAG, "Receive flag from Decoder : " + WrapMediaCodec.flagToString(iFlag));
					if( iFlag == WrapMediaCodec.BUFFER_FLAG_END_OF_STREAM )
					{
						mDecodeReceivedEOS = true;
						if( LL.D ) Log.d(LOG_TAG, "Decoder EOS received");
					}
					return true;				
					
				}
			});
			
			usTime = 0;
			while( mEncodedFrameVec.size() > 0 )
			{
				byte [] buffer = mEncodedFrameVec.get(0);
				if( buffer != null )
				{
					mWrapMediaDecoder.offerDecoder(buffer, usTime);
					mEncodedFrameVec.remove(0);
					usTime += 30000;
				}
			}				
			
			while(mDecodeReceivedEOS == false)
			{
				mWrapMediaDecoder.offerDecoder((ByteBuffer)null, usTime);
			}
			
			try {
				mWrapMediaDecoder.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			if( mDecodedFrameVec.size() > 0 )
			{
				int iWidth = mWrapMediaDecoder.getWidth();
				int iHeight = mWrapMediaDecoder.getHeight();
				byte [] dstBuffer = mDecodedFrameVec.get(0);
				if( dstBuffer != null )
				{
					byte iSrcU1 = inputSrc[1280*720];
					byte iSrcV1 = inputSrc[1280*720+1];
					byte iSrcU2 = inputSrc[1280*720+2];
					byte iSrcV2 = inputSrc[1280*720+3];
					
					byte iDstU1 = dstBuffer[iWidth*iHeight];
					byte iDstV1 = dstBuffer[iWidth*iHeight+1];
					byte iDstU2 = dstBuffer[iWidth*iHeight+2];
					byte iDstV2 = dstBuffer[iWidth*iHeight+3];
					
					int val = Math.abs(iSrcU1 - iDstU1);
					val += Math.abs(iSrcV1 - iDstV1);
					val += Math.abs(iSrcU2 - iDstU2);
					val += Math.abs(iSrcV2 - iDstV2);
					
					val = val / 4;
					
					if( val > 10 ) {
						iFormat = ColorFormat.NV21;
					} else {
						iFormat = ColorFormat.NV12;
					}
				}
			}
		}
		return iFormat;
	}
}
