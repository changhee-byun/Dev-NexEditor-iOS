package com.nexstreaming.kminternal.kinemaster.mediainfo;

import java.io.File;
import java.io.IOException;

import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.TextView;

import com.nexstreaming.kminternal.kinemaster.config.EditorGlobal;
import com.nexstreaming.kminternal.nexvideoeditor.NexEditor;
import com.nexstreaming.kminternal.nexvideoeditor.NexVisualClipChecker;

public class MediaInfoDialog {

	private MediaInfoDialog() {}
	
	public static void showDialog(final Context ctx, String path) {
		if( path==null ) {
			showDialog(ctx, (File)null);
		} else {
			showDialog(ctx, new File(path));
		}
	}
	
	public static void showDialog(final Context ctx, File path) {
	
		StringBuilder sb = new StringBuilder();
		
		if( path==null ) {
			sb.append("Null Path");
		} else {
			sb.append("Name: ");
			sb.append(path.getName());
			sb.append('\n');
			
			sb.append("Path: ");
			sb.append(path.getAbsolutePath());
			sb.append('\n');
			
			if( !path.exists() ) {
				sb.append("(File does not exist)\n");
			} else if (EditorGlobal.isImage(path.getAbsolutePath())) {
				sb.append("Type: IMAGE\n");
				BitmapFactory.Options opts = new BitmapFactory.Options();
				opts.inJustDecodeBounds = true;
				BitmapFactory.decodeFile(path.getAbsolutePath(), opts);
				sb.append("Size: ");
				sb.append(opts.outWidth);
				sb.append('x');
				sb.append(opts.outHeight);
				sb.append('\n');
				
				sb.append("MIME: ");
				sb.append(opts.outMimeType==null?"?":opts.outMimeType);
				sb.append('\n');
				
				ExifInterface exif = null;
				try {
					exif = new ExifInterface(path.getAbsolutePath());
				} catch( IOException e ) {
					exif = null;
				}
				if( exif!=null ) {
					String[] tags = {
							ExifInterface.TAG_APERTURE,	
							ExifInterface.TAG_DATETIME,	
							ExifInterface.TAG_EXPOSURE_TIME,	
							ExifInterface.TAG_FLASH,	
							ExifInterface.TAG_FOCAL_LENGTH,	
							ExifInterface.TAG_GPS_ALTITUDE,	
							ExifInterface.TAG_GPS_ALTITUDE_REF,	
							ExifInterface.TAG_GPS_DATESTAMP,	
							ExifInterface.TAG_GPS_LATITUDE,	
							ExifInterface.TAG_GPS_LATITUDE_REF,	
							ExifInterface.TAG_GPS_LONGITUDE,	
							ExifInterface.TAG_GPS_LONGITUDE_REF,	
							ExifInterface.TAG_GPS_PROCESSING_METHOD,	
							ExifInterface.TAG_GPS_TIMESTAMP,	
							ExifInterface.TAG_IMAGE_LENGTH,	
							ExifInterface.TAG_IMAGE_WIDTH,	
							ExifInterface.TAG_ISO,	
							ExifInterface.TAG_MAKE,	
							ExifInterface.TAG_MODEL,	
							ExifInterface.TAG_ORIENTATION,	
							ExifInterface.TAG_WHITE_BALANCE	
					};
					sb.append("EXIF: \n");
					for( String tag: tags ) {
						String value = exif.getAttribute(tag);
						if( value!=null ) {
							sb.append("   ");
							sb.append(tag);
							sb.append('=');
							sb.append(value);
							sb.append('\n');
						}
					}
				}
			} else if (EditorGlobal.isAudio(path.getAbsolutePath())) {
				sb.append("Type: AUDIO\n");
				MediaInfo info = MediaInfo.getInfo(path.getAbsolutePath(),true);
				if( info==null ) {
					sb.append("(Media info not available)\n");
				} else {
					sb.append("File size: ");
					sb.append(info.getFileSize());
					sb.append('\n');

					sb.append("Duration: ");
					sb.append(info.getDuration());
					sb.append('\n');

					sb.append("Audio Duration: ");
					sb.append(info.getAudioDuration());
					sb.append('\n');

					sb.append("Has audio: ");
					sb.append(info.hasAudio()?"yes":"no");
					sb.append('\n');

					sb.append("Has video: ");
					sb.append(info.hasVideo()?"yes":"no");
					sb.append('\n');

					sb.append("Excluded: ");
					sb.append(info.isExcluded()?"yes":"no");
					sb.append('\n');
					
				}
			} else if (EditorGlobal.isVideo(path.getAbsolutePath())) {
				sb.append("Type: VIDEO\n");
				MediaInfo info = MediaInfo.getInfo(path.getAbsolutePath(),true);
				if( info==null ) {
					sb.append("(Media info not available)\n");
				} else {
					sb.append("File size: ");
					sb.append(info.getFileSize());
					sb.append('\n');
					
					sb.append("Duration: ");
					sb.append(info.getDuration());
					sb.append('\n');

					sb.append("Audio Duration: ");
					sb.append(info.getAudioDuration());
					sb.append('\n');

					sb.append("Video Duration: ");
					sb.append(info.getVideoDuration());
					sb.append('\n');

					sb.append("FPS: ");
					sb.append(info.getFPS());
					sb.append('\n');

					sb.append("Seek point count: ");
					sb.append(info.getSeekPointCount());
					sb.append('\n');

					sb.append("H264 Level: ");
					sb.append(info.getVideoH264Level());
					sb.append('\n');

					sb.append("H264 Profile: ");
					sb.append(info.getVideoH264Profile());
					sb.append('\n');

					sb.append("Size: ");
					sb.append(info.getVideoWidth());
					sb.append('x');
					sb.append(info.getVideoHeight());
					sb.append('\n');

					sb.append("Has audio: ");
					sb.append(info.hasAudio()?"yes":"no");
					sb.append('\n');

					sb.append("Has video: ");
					sb.append(info.hasVideo()?"yes":"no");
					sb.append('\n');

					sb.append("Excluded: ");
					sb.append(info.isExcluded()?"yes":"no");
					sb.append('\n');

//					sb.append("Support Type: " + info.getMediaSupportType() + "\n" );
					
					
					NexEditor editor = EditorGlobal.getEditor();
					if( editor != null ) {
						NexVisualClipChecker checker = editor.getVisualClipChecker();
						if( checker!=null ) {
                            String supportType;
							int iSupport = checker.checkSupportedClip(
									info.getVideoH264Profile(), 
									info.getVideoH264Level(), 
									info.getVideoWidth(),
									info.getVideoHeight(),
									info.getFPS(),
									info.getVideoBitrate(),
									info.getAudioSamplingRate(),
									info.getAudioChannels());
							switch( iSupport ) {
								case 0: // Supported
									supportType = "Supported";
									break;
								case 1: // Not supported but supported after resolution transcoding.
									supportType = "NeedTranscodeRes";
									break;
								case 2: // Not supported but supported after FPS transcoding.
									supportType = "NeedTranscodeFPS";
									break;
								case 3: // Not supported (profile not supported)
									supportType = "NotSupported_VideoProfile";
									break;
								case 4: // Not supported (resolution over HW codec max resolution)
									supportType = "NotSupported_ResolutionTooHigh";
									break;
								default:
									supportType = "Unknown";
									break;
							}
							sb.append("VCC Support Type: " + supportType + " (" + iSupport + ")\n" );
						}
					}
					
				}
			} else {
				sb.append("Type: UNKNOWN\n");
			}
		}
		
		final String diag_info = sb.toString();
		TextView tv = new TextView(ctx);
		tv.setText(diag_info);
		tv.setTextIsSelectable(true);
		tv.setMovementMethod(new ScrollingMovementMethod());
		tv.setScrollBarStyle(View.SCROLLBARS_INSIDE_INSET);
		new AlertDialog.Builder(ctx).setView(tv).setTitle("Diagnostic Information").setCancelable(true).setPositiveButton("OK", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dlg, int arg1) {
				dlg.dismiss();
			}
		}).setNeutralButton("Copy to Clipboard", new DialogInterface.OnClickListener() {
			
			@Override
			public void onClick(DialogInterface dialog, int which) {
				ClipboardManager clipboard = (ClipboardManager)
				        ctx.getSystemService(Context.CLIPBOARD_SERVICE);
				ClipData clip = ClipData.newPlainText("diagnostic info", diag_info);
				clipboard.setPrimaryClip(clip);
			}
		}).show();

	}

}
