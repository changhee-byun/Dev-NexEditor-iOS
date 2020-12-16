/******************************************************************************
 * File Name        : DemoActivityList.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdkapis;

import android.content.Context;
import android.content.Intent;

import com.nexstreaming.nexeditorsdkapis.audio.AudioBGMTestActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioEditActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioEnvelopActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioPCMActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioSoundSettingActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioSpeedControlActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioTrackActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioVisualizerActivity;
import com.nexstreaming.nexeditorsdkapis.audio.AudioVoiceChangerActivity;
import com.nexstreaming.nexeditorsdkapis.common.Constants;
import com.nexstreaming.nexeditorsdkapis.edit.ClipColorAdjustActivity;
import com.nexstreaming.nexeditorsdkapis.edit.ClipCropTestActivity;
import com.nexstreaming.nexeditorsdkapis.edit.ClipInfoActivity;
import com.nexstreaming.nexeditorsdkapis.edit.ClipMultiTrimActivity;
import com.nexstreaming.nexeditorsdkapis.edit.ProjectEdit2Activity;
import com.nexstreaming.nexeditorsdkapis.edit.ProjectEditActivity;
import com.nexstreaming.nexeditorsdkapis.effect.ColorEffectListActivity;
import com.nexstreaming.nexeditorsdkapis.effect.EffectCaptureActivity;
import com.nexstreaming.nexeditorsdkapis.effect.EffectListActivity;
import com.nexstreaming.nexeditorsdkapis.effect.EffectPreviewActivity;
import com.nexstreaming.nexeditorsdkapis.effect.EffectPreviewListActivity;
import com.nexstreaming.nexeditorsdkapis.etc.AspectRatioTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.AutoTrimSpeedCheckActivity;
import com.nexstreaming.nexeditorsdkapis.etc.AutoTrimTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.CollageTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.EditorSimpleActivity;
import com.nexstreaming.nexeditorsdkapis.etc.ExceptionTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.FaceDetectorActivity;
import com.nexstreaming.nexeditorsdkapis.etc.GifViewActivity;
import com.nexstreaming.nexeditorsdkapis.etc.MediaPlayerActivity;
import com.nexstreaming.nexeditorsdkapis.etc.ReverseTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.Reverse3TestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.SnapShotTestActivity;
import com.nexstreaming.nexeditorsdkapis.etc.Template20TestActivity;
import com.nexstreaming.nexeditorsdkapis.export.TransCoderTestActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.ChromaKeyTestActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayAssetActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayFastViewActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayFilterPreViewActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayKMTextActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayMaskTestActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlaySlideShowActivity;
import com.nexstreaming.nexeditorsdkapis.overlay.OverlayUserImageActivity;
import com.nexstreaming.nexeditorsdkapis.pip.PIPDisplayTestActivity;
import com.nexstreaming.nexeditorsdkapis.pip.PIPPlayTestActivity;
import com.nexstreaming.nexeditorsdkapis.surface.EngineViewTestActivity;
import com.nexstreaming.nexeditorsdkapis.surface.SurfaceViewTestActivity;
import com.nexstreaming.nexeditorsdkapis.surface.ThemeViewTestActivity;
import com.nexstreaming.nexeditorsdkapis.text.TextEffectActivity;
import com.nexstreaming.nexeditorsdkapis.thumbnail.ThumbnailActivity;
import com.nexstreaming.nexeditorsdkapis.thumbnail.ThumbnailCacnelTestActivity;
import com.nexstreaming.nexeditorsdkapis.thumbnail.ThumbnailSeektabActivity;

import java.util.ArrayList;

/**
 * Created by jeongwook.yoon on 2016-11-12.
 */

public class DemoActivityList {

    static final DemoActivity[] DemoActivityList = {
            new DemoActivity(EffectPreviewListActivity.class,new String[]{"image/*", "video/*"},true,"EffectRandomTransitionPreview", Constants.EFFECT_TYPE,Constants.EFFECT_TYPE_RANDOM_TRANSITION),
            new DemoActivity(EffectPreviewActivity.class,"EffectTransitionPreview" ,Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_TRANSITION),
            new DemoActivity(EffectPreviewActivity.class,"EffectClipPreview" ,Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_CLIP),
            new DemoActivity(EffectListActivity.class),
            new DemoActivity(ColorEffectListActivity.class),
            new DemoActivity(ColorEffectListActivity.class,"StickerList",Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_STICKER),
            new DemoActivity(ColorEffectListActivity.class,"FontList",Constants.EFFECT_TYPE, Constants.EFFECT_TYPE_FONT),
            new DemoActivity(ProjectEditActivity.class),
            new DemoActivity(ProjectEdit2Activity.class),
            new DemoActivity(ClipInfoActivity.class,new String[]{"image/*", "video/*"},false),
            new DemoActivity(ClipMultiTrimActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ClipColorAdjustActivity.class,new String[]{"image/*", "video/*"},false),
            new DemoActivity(ClipCropTestActivity.class,new String[]{"image/*", "video/*"},false),
            new DemoActivity(OverlayUserImageActivity.class,new String[]{"video/*"},false),
            new DemoActivity(OverlaySlideShowActivity.class,new String[]{"image/*"},true),
            new DemoActivity(OverlayKMTextActivity.class,new String[]{"video/*"},false),
            new DemoActivity(OverlayFastViewActivity.class),
            new DemoActivity(OverlayMaskTestActivity.class,new String[]{"image/*","video/*"},true),
            new DemoActivity(AudioSoundSettingActivity.class),
            new DemoActivity(AudioSpeedControlActivity.class,new String[]{"video/*"},false),
            new DemoActivity(AudioBGMTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(AudioEnvelopActivity.class,new String[]{"audio/*"},false),
            new DemoActivity(AudioTrackActivity.class),
            new DemoActivity(AudioVoiceChangerActivity.class),
            new DemoActivity(AudioEditActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ThumbnailActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ThumbnailSeektabActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ThumbnailCacnelTestActivity.class),
            new DemoActivity(TransCoderTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(PIPDisplayTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(PIPPlayTestActivity.class),
            new DemoActivity(AutoTrimTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(AutoTrimSpeedCheckActivity.class,new String[]{"video/*"},false),
            new DemoActivity(Template20TestActivity.class,new String[]{"image/*","video/*"},true),
            new DemoActivity(Template20TestActivity.class,new String[]{"image/*"},true, "Template30TestActivity", Constants.TEMPLATE_VERSION, Constants.TEMPLATE_VERSION_3),
            new DemoActivity(CollageTestActivity.class,new String[]{"image/*"},true),
            new DemoActivity(MediaPlayerActivity.class,new String[]{"image/*","video/*"},false),
            new DemoActivity(SnapShotTestActivity.class),
            new DemoActivity(ExceptionTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ReverseTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(Reverse3TestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(AspectRatioTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(AudioVisualizerActivity.class,new String[]{"video/*"},false),
            new DemoActivity(ChromaKeyTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(SurfaceViewTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(EngineViewTestActivity.class,new String[]{"video/*"},false),
            new DemoActivity(OverlayFilterPreViewActivity.class),
            new DemoActivity(EffectCaptureActivity.class,new String[]{"image/*"},false),
            new DemoActivity(ThemeViewTestActivity.class),
            new DemoActivity(EditorSimpleActivity.class),
            new DemoActivity(OverlayAssetActivity.class,new String[]{"image/*","video/*"},false),
            new DemoActivity(AudioPCMActivity.class),
            new DemoActivity(GifViewActivity.class,new String[]{"image/gif"},false),
            new DemoActivity(FaceDetectorActivity.class,new String[]{"image/*"},true),
            new DemoActivity(TextEffectActivity.class)
    };


    static class DemoActivity{
        String label;
        Class  activity;
        String[] callGalleryMimeTypes;
        boolean callGalleryMultiple;
        String putExtraKey;
        int putExtraValue;

        DemoActivity(Class activity, String[] callGalleryMimeTypes, boolean callGalleryMultiple) {
            this.activity = activity;
            this.callGalleryMimeTypes = callGalleryMimeTypes;
            this.callGalleryMultiple = callGalleryMultiple;
        }

        DemoActivity(Class activity) {
            this.activity = activity;
        }

        DemoActivity(Class activity, String label, String putExtraKey, int putExtraValue) {
            this.activity = activity;
            this.label = label;
            this.putExtraKey = putExtraKey;
            this.putExtraValue = putExtraValue;
        }

        DemoActivity(Class activity, String[] callGalleryMimeTypes, boolean callGalleryMultiple,String label, String putExtraKey, int putExtraValue) {
            this.activity = activity;
            this.callGalleryMimeTypes = callGalleryMimeTypes;
            this.callGalleryMultiple = callGalleryMultiple;
            this.label = label;
            this.putExtraKey = putExtraKey;
            this.putExtraValue = putExtraValue;
        }


        public String label(){
            if(label != null)
                return label;
            return activity.getName().substring(activity.getName().lastIndexOf('.') + 1);
        }

        public boolean callGallery(){
            return (callGalleryMimeTypes != null);
        }
    };


    static Intent makeGalleryIntent(int index ){
        if( DemoActivityList[index].callGallery() ){
            Intent intent = new Intent( );
            intent.setType("*/*");
            String[] mimetypes = DemoActivityList[index].callGalleryMimeTypes;
            intent.putExtra(Intent.EXTRA_MIME_TYPES, mimetypes);
            intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, DemoActivityList[index].callGalleryMultiple);
            intent.setAction(Intent.ACTION_GET_CONTENT);
            return intent;
        }
        return null;

    }

    static Intent makeDemoIntent(Context baseContext, int index , ArrayList<String> fileList) {
        Intent intent = new Intent(baseContext, DemoActivityList[index].activity);
        if( DemoActivityList[index].putExtraKey != null ){
            intent.putExtra(DemoActivityList[index].putExtraKey,DemoActivityList[index].putExtraValue);
        }
        if( fileList != null ) {
            intent.putStringArrayListExtra("filelist", fileList);
        }
        intent.putExtra("existsave", false);
        return intent;
    }

    static Intent makeDemoIntent(Context baseContext, int index ) {
        Intent intent = new Intent(baseContext, DemoActivityList[index].activity);
        if( DemoActivityList[index].putExtraKey != null ){
            intent.putExtra(DemoActivityList[index].putExtraKey,DemoActivityList[index].putExtraValue);
        }
        intent.putExtra("existsave", true);
        return intent;
    }


}
