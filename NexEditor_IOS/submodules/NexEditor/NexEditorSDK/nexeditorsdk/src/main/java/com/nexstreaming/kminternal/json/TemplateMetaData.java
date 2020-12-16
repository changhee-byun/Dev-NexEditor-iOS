package com.nexstreaming.kminternal.json;

import java.util.ArrayList;

/**
 * Created by jeongwook.yoon on 2018-01-15.
 */

public class TemplateMetaData {
    private TemplateMetaData(){} // Prevent instantiation

    public static class DefaultEffects{
        public String color_filter_id;
        public String clip_effect_id;
        public String sc_color_effect_id;
        public String sc_clip_effect_id;
    }


    public static class EffectEntry{
        public int int_priority;
        public int int_time;
        public int int_effect_in_duration = -1;
        public int int_effect_out_duration = -1;
        public String color_filter_id;
        public String clip_effect_id;
        public boolean b_source_change;
        public String internal_clip_id;
    }

    public static class Music{
        public String string_title;
        public String string_audio_id;
        public int int_version;
        public DefaultEffects default_effect = null;
        public ArrayList<EffectEntry> list_effectEntries;
    }
}
