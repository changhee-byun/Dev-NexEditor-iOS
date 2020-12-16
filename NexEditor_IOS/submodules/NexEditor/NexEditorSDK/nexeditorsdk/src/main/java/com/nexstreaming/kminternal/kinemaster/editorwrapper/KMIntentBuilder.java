package com.nexstreaming.kminternal.kinemaster.editorwrapper;

import android.content.Intent;
import com.google.gson_nex.Gson;

import java.util.ArrayList;

@SuppressWarnings("unused")
public class KMIntentBuilder {

    private KMIntentData intentData = new KMIntentData();

    public VideoClipBuilder startVideoClip() {
        KMIntentData.VisualClip visualClip = new KMIntentData.VisualClip();
        intentData.project.visualClips.add(visualClip);
        return new VideoClipBuilder(visualClip,this);
    }

    public ImageClipBuilder startImageClip() {
        KMIntentData.VisualClip visualClip = new KMIntentData.VisualClip();
        intentData.project.visualClips.add(visualClip);
        return new ImageClipBuilder(visualClip,this);
    }

    public SolidClipBuilder startSolidClip() {
        KMIntentData.VisualClip visualClip = new KMIntentData.VisualClip();
        intentData.project.visualClips.add(visualClip);
        return new SolidClipBuilder(visualClip,this);
    }

    public AudioClipBuilder startAudioClip() {
        KMIntentData.AudioClip audioClip = new KMIntentData.AudioClip();
        intentData.project.audioClips.add(audioClip);
        return new AudioClipBuilder(audioClip,this);
    }

    public TextLayerBuilder startTextLayer() {
        KMIntentData.Layer layer = new KMIntentData.Layer();
        layer.layerType = KMIntentData.LayerType.Text;
        intentData.project.layers.add(layer);
        return new TextLayerBuilder(layer,this);
    }

    public StickerLayerBuilder startStickerLayer() {
        KMIntentData.Layer layer = new KMIntentData.Layer();
        layer.layerType = KMIntentData.LayerType.Sticker;
        intentData.project.layers.add(layer);
        return new StickerLayerBuilder(layer,this);
    }

    public ImageLayerBuilder startImageLayer() {
        KMIntentData.Layer layer = new KMIntentData.Layer();
        layer.layerType = KMIntentData.LayerType.Image;
        intentData.project.layers.add(layer);
        return new ImageLayerBuilder(layer,this);
    }

    public VideoLayerBuilder startVideoLayer() {
        KMIntentData.Layer layer = new KMIntentData.Layer();
        layer.layerType = KMIntentData.LayerType.Video;
        intentData.project.layers.add(layer);
        return new VideoLayerBuilder(layer,this);
    }

    public Intent build() {
        Gson gson = new Gson();

//        Intent intent = new Intent();
        Intent intent = new Intent("com.kinemaster.intent.NEW_PROJECT");
        intent.setPackage("com.nexstreaming.app.kinemasterfree");
        intent.putExtra("com.kinemaster.intent.projectData",gson.toJson(intentData));
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK|Intent.FLAG_ACTIVITY_NEW_TASK);
        return intent;
    }

    public static class AudioClipBuilder {
        protected final KMIntentData.AudioClip audioClip;
        protected final KMIntentBuilder intentBuilder;
        AudioClipBuilder(KMIntentData.AudioClip audioClip, KMIntentBuilder intentBuilder) {
            this.audioClip = audioClip;
            this.intentBuilder = intentBuilder;
        }

        public AudioClipBuilder path( String path ) {
            audioClip.path = path;
            return this;
        }

        public AudioClipBuilder mediaTitle( String mediaTitle ) {
            audioClip.mediaTitle = mediaTitle;
            return this;
        }

        public AudioClipBuilder startTrim( int startTrim ) {
            audioClip.startTrim = startTrim;
            return this;
        }

        public AudioClipBuilder endTrim( int endTrim ) {
            audioClip.endTrim = endTrim;
            return this;
        }

        public AudioClipBuilder startTime( int startTime ) {
            audioClip.startTime = startTime;
            return this;
        }

        public AudioClipBuilder endTime( int endTime ) {
            audioClip.endTime = endTime;
            return this;
        }

        public AudioClipBuilder loop( boolean loop ) {
            audioClip.loop = loop;
            return this;
        }

        public AudioClipBuilder background( boolean background ) {
            audioClip.background = background;
            return this;
        }

        public AudioClipBuilder extendToEnd( boolean extendToEnd ) {
            audioClip.extendToEnd = extendToEnd;
            return this;
        }

        public AudioClipBuilder mute( boolean mute ) {
            audioClip.mute = mute;
            return this;
        }

        public AudioClipBuilder pinned( boolean pinned ) {
            audioClip.pinned = pinned;
            return this;
        }

        public AudioClipBuilder volume( int volume ) {
            audioClip.volume = volume;
            return this;
        }

        public KMIntentBuilder endAudioClip() {
            return intentBuilder;
        }

    }

    private static class LayerBuilder<T> {
        protected final KMIntentData.Layer layer;
        protected final KMIntentBuilder intentBuilder;
        @SuppressWarnings("unchecked")
        protected final T self = (T) this;
        LayerBuilder(KMIntentData.Layer layer, KMIntentBuilder intentBuilder) {
            this.layer = layer;
            this.intentBuilder = intentBuilder;
        }

        public T startTime( int startTime ) {
            layer.startTime = startTime;
            return self;
        }

        public T endTime( int endTime ) {
            layer.endTime = endTime;
            return self;
        }

        public T pinned( boolean pinned ) {
            layer.pinned = pinned;
            return self;
        }

        public T animationIn( String animationIn ) {
            layer.animationIn = animationIn;
            return self;
        }

        public T animationOut( String animationOut ) {
            layer.animationOut = animationOut;
            return self;
        }

        public T animationOverall( String animationOverall ) {
            layer.animationOverall = animationOverall;
            return self;
        }

        public T animationInDuration( int animationInDuration) {
            layer.animationInDuration = animationInDuration;
            return self;
        }

        public T animationOutDuration( int animationOutDuration) {
            layer.animationOutDuration = animationOutDuration;
            return self;
        }

        public KeyframeBuilder<T> startKeyframe() {
            if( layer.keyFrames==null ) {
                layer.keyFrames = new ArrayList<>();
            }
            KMIntentData.KeyFrame keyFrame = new KMIntentData.KeyFrame();
            layer.keyFrames.add(keyFrame);
            return new KeyframeBuilder<>(keyFrame,this);
        }

    }

    public static class KeyframeBuilder<T> {
        protected final KMIntentData.KeyFrame keyFrame;
        protected final LayerBuilder<T> layerBuilder;
        KeyframeBuilder(KMIntentData.KeyFrame keyFrame, LayerBuilder<T> layerBuilder) {
            this.keyFrame = keyFrame;
            this.layerBuilder = layerBuilder;
        }

        public KeyframeBuilder<T> time( float time) {
            keyFrame.time = time;
            return this;
        }

        public KeyframeBuilder<T> angle( float angle) {
            keyFrame.angle = angle;
            return this;
        }

        public KeyframeBuilder<T> alpha( float alpha) {
            keyFrame.alpha = alpha;
            return this;
        }

        public KeyframeBuilder<T> scale( float scale) {
            keyFrame.scale = scale;
            return this;
        }

        public KeyframeBuilder<T> position( float x, float y) {
            keyFrame.x = x;
            keyFrame.y = y;
            return this;
        }

        public T endKeyframe() {
            return (T)layerBuilder;
        }
    }

    public static class TextLayerBuilder extends LayerBuilder<TextLayerBuilder> {

        TextLayerBuilder(KMIntentData.Layer layer, KMIntentBuilder intentBuilder) {
            super(layer, intentBuilder);
            layer.textLayerAttributes = new KMIntentData.TextLayerAttributes();
        }

        public TextLayerBuilder text(String text) {
            layer.textLayerAttributes.text = text;
            return this;
        }

        public TextLayerBuilder textSize(float textSize) {
            layer.textLayerAttributes.textSize = textSize;
            return this;
        }

        public TextLayerBuilder textColor(int textColor) {
            layer.textLayerAttributes.textColor = textColor;
            return this;
        }

        public TextLayerBuilder shadowColor(int shadowColor) {
            layer.textLayerAttributes.shadowColor = shadowColor;
            return this;
        }

        public TextLayerBuilder glowColor(int glowColor) {
            layer.textLayerAttributes.glowColor = glowColor;
            return this;
        }

        public TextLayerBuilder outlineColor(int outlineColor) {
            layer.textLayerAttributes.outlineColor = outlineColor;
            return this;
        }

        public TextLayerBuilder backgroundColor(int backgroundColor) {
            layer.textLayerAttributes.backgroundColor = backgroundColor;
            return this;
        }

        public TextLayerBuilder align(int align) {
            layer.textLayerAttributes.align = align;
            return this;
        }

        public TextLayerBuilder fontId(String fontId) {
            layer.textLayerAttributes.fontId = fontId;
            return this;
        }

        public KMIntentBuilder endTextLayer() {
            return intentBuilder;
        }
    }

    public static class StickerLayerBuilder extends LayerBuilder<StickerLayerBuilder> {

        StickerLayerBuilder(KMIntentData.Layer layer, KMIntentBuilder intentBuilder) {
            super(layer, intentBuilder);
        }

        public StickerLayerBuilder stickerId(String stickerId) {
            layer.stickerLayerAttributes.stickerId = stickerId;
            return this;
        }

        public KMIntentBuilder endStackerLayer() {
            return intentBuilder;
        }
    }

    public static class ImageLayerBuilder extends LayerBuilder<ImageLayerBuilder> {

        ImageLayerBuilder(KMIntentData.Layer layer, KMIntentBuilder intentBuilder) {
            super(layer, intentBuilder);
        }

        public ImageLayerBuilder path(String path) {
            layer.imageLayerAttributes.path = path;
            return this;
        }

        public ImageLayerBuilder colorFilterId(String colorFilterId) {
            layer.imageLayerAttributes.colorFilterId = colorFilterId;
            return this;
        }

        public ImageLayerBuilder brightness(int brightness) {
            layer.imageLayerAttributes.brightness = brightness;
            return this;
        }

        public ImageLayerBuilder contrast(int contrast) {
            layer.imageLayerAttributes.contrast = contrast;
            return this;
        }

        public ImageLayerBuilder saturation(int saturation) {
            layer.imageLayerAttributes.saturation = saturation;
            return this;
        }

        public KMIntentBuilder endImageLayer() {
            return intentBuilder;
        }
    }


    public static class VideoLayerBuilder extends LayerBuilder<VideoLayerBuilder> {

        VideoLayerBuilder(KMIntentData.Layer layer, KMIntentBuilder intentBuilder) {
            super(layer, intentBuilder);
        }

        public VideoLayerBuilder path(String path) {
            layer.videoLayerAttributes.path = path;
            return this;
        }

        public VideoLayerBuilder colorFilterId(String colorFilterId) {
            layer.videoLayerAttributes.colorFilterId = colorFilterId;
            return this;
        }

        public VideoLayerBuilder brightness(int brightness) {
            layer.videoLayerAttributes.brightness = brightness;
            return this;
        }

        public VideoLayerBuilder contrast(int contrast) {
            layer.videoLayerAttributes.contrast = contrast;
            return this;
        }

        public VideoLayerBuilder saturation(int saturation) {
            layer.videoLayerAttributes.saturation = saturation;
            return this;
        }

        public VideoLayerBuilder startTrim(int startTrim) {
            layer.videoLayerAttributes.startTrim = startTrim;
            return this;
        }

        public VideoLayerBuilder volume(int volume) {
            layer.videoLayerAttributes.volume = volume;
            return this;
        }

        public VideoLayerBuilder mute(boolean mute) {
            layer.videoLayerAttributes.mute = mute;
            return this;
        }

        public KMIntentBuilder endImageLayer() {
            return intentBuilder;
        }
    }


    private static class VisualClipBuilder<T> {

        protected final KMIntentData.VisualClip visualClip;
        protected final KMIntentBuilder intentBuilder;
        @SuppressWarnings("unchecked")
        protected final T self = (T) this;
        VisualClipBuilder(KMIntentData.VisualClip visualClip, KMIntentBuilder intentBuilder) {
            this.visualClip = visualClip;
            this.intentBuilder = intentBuilder;
        }

        public T duration(int duration) {
            visualClip.duration = duration;
            return self;
        }
        public T transitionDuration(int transitionDuration) {
            visualClip.transitionDuration = transitionDuration;
            return self;
        }
        public T clipEffectId(String clipEffectId) {
            visualClip.clipEffectId = clipEffectId;
            return self;
        }
        public T clipEffectParameter(String key, String value){
            visualClip.clipEffectParameters.put(key,value);
            return self;
        }
        public T transitionEffectId(String transitionEffectId){
            visualClip.transitionEffectId = transitionEffectId;
            return self;
        }
        public T transitionEffectParameter(String key, String value){
            visualClip.transitionEffectParameters.put(key,value);
            return self;
        }
        public T volume(int volume){
            visualClip.volume = volume;
            return self;
        }
        public T mute(boolean mute){
            visualClip.mute = mute;
            return self;
        }
        public T rotation(int degree){
            visualClip.rotation = degree;
            return self;
        }
        public T brightness(int brightness){
            visualClip.brightness = brightness;
            return self;
        }
        public T contrast(int contrast){
            visualClip.contrast = contrast;
            return self;
        }
        public T saturation(int saturation){
            visualClip.saturation = saturation;
            return self;
        }
        public T colorFilterId(String colorFilterId){
            visualClip.colorFilterId = colorFilterId;
            return self;
        }
        public T vignette(boolean set){
            visualClip.vignette = set;
            return self;
        }

    }

    public static class VideoClipBuilder extends VisualClipBuilder<VideoClipBuilder> {

        VideoClipBuilder(KMIntentData.VisualClip visualClip, KMIntentBuilder intentBuilder) {
            super(visualClip, intentBuilder);
        }

        public VideoClipBuilder path(String path) {
            visualClip.path = path;
            return this;
        }

        public VideoClipBuilder startTrim(int startTrim) {
            visualClip.startTrim = startTrim;
            return this;
        }

        public VideoClipBuilder rotation(int degree){
            visualClip.rotation = degree;
            return this;
        }

        public VideoClipBuilder fliph(boolean fliph) {
            visualClip.fliph = fliph;
            return this;
        }

        public VideoClipBuilder flipv(boolean flipv) {
            visualClip.flipv = flipv;
            return this;
        }

        public VideoClipBuilder playbackSpeed(int playbackSpeed) {
            visualClip.playbackSpeed = playbackSpeed;
            return this;
        }

        public VideoClipBuilder crop(float startLeft, float startTop, float startRight, float startBottom, float endLeft, float endTop, float endRight, float endBottom ) {
            visualClip.cropStartLeft = startLeft;
            visualClip.cropStartTop = startTop;
            visualClip.cropStartRight = startRight;
            visualClip.cropStartBottom = startBottom;
            visualClip.cropEndLeft = endLeft;
            visualClip.cropEndTop = endTop;
            visualClip.cropEndRight = endRight;
            visualClip.cropEndBottom = endBottom;
            return this;
        }

        public KMIntentBuilder endVideoClip() {
            return intentBuilder;
        }
    }

    public static class ImageClipBuilder extends VisualClipBuilder<ImageClipBuilder> {

        ImageClipBuilder(KMIntentData.VisualClip visualClip, KMIntentBuilder intentBuilder) {
            super(visualClip, intentBuilder);
        }

        public ImageClipBuilder path(String path) {
            visualClip.path = path;
            return this;
        }

        public ImageClipBuilder crop(float startLeft, float startTop, float startRight, float startBottom, float endLeft, float endTop, float endRight, float endBottom ) {
            visualClip.cropStartLeft = startLeft;
            visualClip.cropStartTop = startTop;
            visualClip.cropStartRight = startRight;
            visualClip.cropStartBottom = startBottom;
            visualClip.cropEndLeft = endLeft;
            visualClip.cropEndTop = endTop;
            visualClip.cropEndRight = endRight;
            visualClip.cropEndBottom = endBottom;
            return this;
        }

        public KMIntentBuilder endImageClip() {
            return intentBuilder;
        }
    }

    public static class SolidClipBuilder extends VisualClipBuilder<SolidClipBuilder> {

        SolidClipBuilder(KMIntentData.VisualClip visualClip, KMIntentBuilder intentBuilder) {
            super(visualClip, intentBuilder);
        }

        public SolidClipBuilder solidColor( int color ) {
            String s = Long.toHexString(((long)color & 0x00FFFFFFFFL)|0x1000000000L);
            visualClip.path = "@solid:" + s.substring(s.length()-8) + ".jpg";
            return this;
        }

        public KMIntentBuilder endSolidClip() {
            return intentBuilder;
        }
    }



}
