package com.nexstreaming.kminternal.kinemaster.editorwrapper;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class KMIntentData {

    public int kmIntentFormatVersion = 0;
    public Project project = new Project();

    public KMIntentData() {

    }

    public static class Project {
        public String name;
        public int compositionWidth = 1920;
        public int compositionHeight = 1080;
        public int videoFadeInTime = 0;
        public int videoFadeOutTime = 0;
        public int audioFadeInTime = 0;
        public int audioFadeOutTime = 0;
        public List<VisualClip> visualClips = new ArrayList<>();
        public List<AudioClip> audioClips = new ArrayList<>();
        public List<Layer> layers = new ArrayList<>();

        public Project() {

        }
    }

    public static class VisualClip {
        public String path;
        public int startTrim;
        public int duration;
        public String clipEffectId;
        public Map<String,String> clipEffectParameters;
        public String transitionEffectId;
        public Map<String,String> transitionEffectParameters;
        public int transitionDuration = 0;

        public int volume = 100;
        public boolean mute = false;
        public int playbackSpeed = 100;

        public int brightness = 0;
        public int contrast = 0;
        public int saturation = 0;
        public String colorFilterId;

        public int rotation = 0;
        public boolean fliph = false;
        public boolean flipv = false;

        public float cropStartLeft;
        public float cropStartTop;
        public float cropStartRight;
        public float cropStartBottom;

        public float cropEndLeft;
        public float cropEndTop;
        public float cropEndRight;
        public float cropEndBottom;

        public boolean vignette;
    }

    public static class AudioClip {
        public String path;
        public String mediaTitle;
        public int startTrim;
        public int endTrim;
        public int startTime;
        public int endTime;
        public boolean loop;
        public boolean background;
        public boolean extendToEnd;

        public int volume;
        public boolean mute;

        public boolean pinned;
    }

    public static class Layer {
        public LayerType layerType;
        public int startTime;
        public int endTime;
        public boolean pinned;
        public VideoLayerAttributes videoLayerAttributes;
        public ImageLayerAttributes imageLayerAttributes;
        public TextLayerAttributes textLayerAttributes;
        public StickerLayerAttributes stickerLayerAttributes;
        public List<KeyFrame> keyFrames;
        public String animationIn;
        public String animationOut;
        public String animationOverall;
        public int animationInDuration;
        public int animationOutDuration;
    }

    public static class VideoLayerAttributes {
        public String path;

        public int brightness;
        public int contrast;
        public int saturation;
        public String colorFilterId;
        public int startTrim;

        public int volume;
        public boolean mute;
    }

    public static class ImageLayerAttributes {
        public String path;

        public int brightness;
        public int contrast;
        public int saturation;
        public String colorFilterId;
    }

    public static class TextLayerAttributes {
        public String text;
        public float textSize;
        public String fontId;
        public int textColor;
        public int shadowColor;
        public int glowColor;
        public int outlineColor;
        public int backgroundColor;
        public int align;
    }

    public static class StickerLayerAttributes {
        public String stickerId;
    }

    public static class KeyFrame {
        public float time = 0;
        public float scale = 1;
        public float x;
        public float y;
        public float angle = 0;
        public float alpha = 1;
    }

    public enum LayerType {
        Image,
        Video,
        Text,
        Sticker
    }

}
