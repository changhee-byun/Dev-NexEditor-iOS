package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.util.Log;

import com.google.gson_nex.Gson;
import com.nexstreaming.kminternal.json.TemplateMetaData;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Created by jeongwook.yoon on 2018-01-25.
 */

public class nexBeatTemplateManager {
    private static nexBeatTemplateManager sSingleton = null;
    private static Context mAppContext;
    private static final String TAG="nexMusicTempMan";
    private List<nexBeatTemplateManager.BeatTemplate> musicTemplates = new ArrayList<nexBeatTemplateManager.BeatTemplate>();
    private List<nexBeatTemplateManager.BeatTemplate> externalView_musicTemplates = null;

    public static class BeatTemplate extends nexAssetPackageManager.ItemEnt{
        private String bgmId;
        private boolean parsed = false;
        private int maxSourceCount;
        private int maxExtendCount = -1;
        private int maxRecommendCount;
        private int internalSourceCount = -1;
        BeatTemplate(nexAssetPackageManager.Item item){
            super(item);
        }

        public static BeatTemplate promote(nexAssetPackageManager.ItemEnt item){
            if( item.category() != nexAssetPackageManager.Category.beattemplate ){
                return null;
            }
            return new BeatTemplate(item);
        }

        private void parseTemplate(){
            if( !parsed ){
                parsed = true;
                String json = nexTemplateComposer.AssetPackageTemplateJsonToString(id());
                if( json != null ) {
                    Gson gson = new Gson();
                    TemplateMetaData.Music meta = gson.fromJson(json, TemplateMetaData.Music.class);
                    if( meta != null ){
                        bgmId = meta.string_audio_id;
                        int count = -1;
                        int recommendCount = 1;
                        int extendsCount = 0;
                        int internalSource = 0;
                        for(TemplateMetaData.EffectEntry ent : meta.list_effectEntries){
                            count++;
                            if( ent.b_source_change || ent.int_priority > 0 ){
                                if( ent.int_priority > 0  ){
                                    extendsCount++;
                                }else {
                                    if( ent.internal_clip_id != null ){
                                        internalSource++;
                                    }else {
                                        recommendCount++;
                                    }
                                }
                            }
                        }
                        maxSourceCount = count;
                        maxRecommendCount = recommendCount;
                        maxExtendCount = extendsCount;
                        internalSourceCount = internalSource;
                    }
                }
            }
        }

        public String getBGMId(){
            parseTemplate();
            return bgmId;
        }

        public int getInternalSourceCount(){
            if( internalSourceCount == -1 ){
                parseTemplate();
            }
            return internalSourceCount;
        }

        public int getMaxExtendCount(){
            if( maxExtendCount < 0 ){
                parseTemplate();
            }
            return maxExtendCount;
        }

        public int getMaxRecommendCount(){
            if( maxRecommendCount == 0 ){
                parseTemplate();
            }
            return maxRecommendCount;
        }

        public int getMaxSourceCount(){
            if( maxSourceCount == 0 ){
                if( id().contains(".sc.") ) {
                    String tmpID = id();
                    int idx = tmpID.indexOf(".sc.");
                    String sc = tmpID.substring(idx+4, tmpID.length());

                    if( sc != null && sc.length() > 0 ) {
                        maxSourceCount = Integer.parseInt(sc);
                        return maxSourceCount;
                    }
                }else{
                    parseTemplate();
                }
            }
            return maxSourceCount;
        }

    }

    private nexBeatTemplateManager(Context appContext ){
        mAppContext = appContext;
    }

    public static nexBeatTemplateManager getBeatTemplateManager(Context appContext ) {

        if( sSingleton!=null && !sSingleton.mAppContext.getPackageName().equals(appContext.getPackageName()) ) {
            sSingleton = null;
        }
        if( sSingleton==null ) {
            sSingleton = new nexBeatTemplateManager(appContext);
        }
        return sSingleton;
    }
    private Object m_musicTemplateLock = new Object();
    private void resolve( boolean ignoreExpired){
        synchronized (m_musicTemplateLock) {
            musicTemplates.clear();
            List<nexAssetPackageManager.Item> audios = nexAssetPackageManager.getAssetPackageManager(mAppContext).getInstalledAssetItems(nexAssetPackageManager.Category.beattemplate);
            for (nexAssetPackageManager.Item info : audios) {
                if ( info.hidden() || ( ignoreExpired && nexAssetPackageManager.getAssetPackageManager(mAppContext).checkExpireAsset(info.packageInfo()) ) ) {
                    continue;
                }
                BeatTemplate t = new BeatTemplate(info);
                musicTemplates.add(t);
            }
        }
    }

    public void loadTemplate(){
        resolve( false);
    }

    public void loadTemplate( boolean ignoreExpired ){
        resolve( ignoreExpired);
    }

    public nexBeatTemplateManager.BeatTemplate getBeatTemplate(String Id){
        synchronized (m_musicTemplateLock) {
            for (nexBeatTemplateManager.BeatTemplate t : musicTemplates) {
                if (t.id().compareTo(Id) == 0) {
                    return t;
                }
            }
            return null;
        }
    }

    public List<nexBeatTemplateManager.BeatTemplate> getBeatTemplates() {
        synchronized (m_musicTemplateLock) {

            if (externalView_musicTemplates == null) {
                externalView_musicTemplates = Collections.unmodifiableList(musicTemplates);
            }
            return externalView_musicTemplates;
        }
    }

    private static class NewEntry{
        public int int_stime;
        public int int_etime;
        public String color_filter_id;
        public String clip_effect_id;
        public boolean b_source_change;
        public String internal_clip_id;
        public int clip_effect_start_time;
        public int clip_effect_end_time;
    }

    public enum Level{
        Recommend,
        Extends,
        Max
    }

    static boolean  internalApplyTemplateToProjectById(nexProject project , String musicTemplateId ){
        String jsonString = nexTemplateComposer.AssetPackageTemplateJsonToString(musicTemplateId);
        Level level = Level.Max;
        if (jsonString != null){
            int minDuration = 10800000;
            int start = 0;

            Gson gson = new Gson();
            TemplateMetaData.Music meta = gson.fromJson(jsonString, TemplateMetaData.Music.class);

            int maxCount = -1;
            int recommendCount = 1;
            int extendsCount = 0;
            int internalCount = 0;
            for( TemplateMetaData.EffectEntry entry : meta.list_effectEntries ){
                if( start > 0 ) {
                    if (minDuration > (entry.int_time - start)) {
                        minDuration = entry.int_time-start;
                    }
                }
                start = entry.int_time;
                maxCount++;
                if( entry.b_source_change || entry.int_priority > 0 ){
                    if( meta.default_effect != null ) {
                        if (entry.color_filter_id == null) {
                            entry.color_filter_id = meta.default_effect.sc_color_effect_id;
                        }

                        if (entry.clip_effect_id == null) {
                            entry.clip_effect_id = meta.default_effect.sc_clip_effect_id;
                        }
                    }
                    if( entry.int_priority > 0 ){
                        extendsCount++;
                    }else{
                        if( entry.internal_clip_id != null ){
                            internalCount++;
                        }else {
                            recommendCount++;
                        }
                    }
                }else {
                    if( meta.default_effect != null ) {
                        if (entry.color_filter_id == null) {
                            entry.color_filter_id = meta.default_effect.color_filter_id;
                        }

                        if (entry.clip_effect_id == null) {
                            entry.clip_effect_id = meta.default_effect.clip_effect_id;
                        }
                    }

                    if( entry.clip_effect_id != null ) {
                        //if( entry.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                        if (entry.int_effect_in_duration == -1) {
                            entry.int_effect_in_duration = 50;
                        }

                        if (entry.int_effect_out_duration == -1) {
                            entry.int_effect_out_duration = 50;
                        }
                        //}
                    }
                }
            }

            meta.list_effectEntries.get(0).int_effect_in_duration = 0;
            meta.list_effectEntries.get(0).int_effect_out_duration = 0;
            meta.list_effectEntries.get(0).clip_effect_id = null;

            meta.list_effectEntries.get(meta.list_effectEntries.size()-1).int_effect_in_duration = 0;
            meta.list_effectEntries.get(meta.list_effectEntries.size()-1).int_effect_out_duration = 0;

            int priorityLevel = 0;
            int eventLevel = 0;
            int sourceClipCount = project.getTotalClipCount(true);
            if( sourceClipCount > recommendCount ){
                priorityLevel = sourceClipCount - recommendCount;
                if( priorityLevel > extendsCount ){
                    eventLevel = priorityLevel-extendsCount;
                    priorityLevel = extendsCount;
                }
            }

            if( level == Level.Extends ){
                eventLevel = 0;
            }else if( level == Level.Recommend){
                priorityLevel = 0;
                eventLevel = 0;
            }

            Log.d(TAG,"sourceClipCount="+sourceClipCount+", recommend="+recommendCount+", extends="+extendsCount+" ,max="+maxCount+", priorityLevel="+priorityLevel+", eventLevel="+eventLevel+", internal="+internalCount);

            ArrayList<nexBeatTemplateManager.NewEntry> resolveEntry = new ArrayList<>();
            ArrayList<TemplateMetaData.EffectEntry> newEffects = new ArrayList<>();

            for(TemplateMetaData.EffectEntry ent :  meta.list_effectEntries){
                if( ent.int_priority > 0 ){
                    if( ent.int_priority <= priorityLevel ){
                        ent.b_source_change = true;
                        Log.d(TAG,"t="+ent.int_time+", priority="+ ent.int_priority );
                    }else{
                        continue;
                    }
                }

                if( !ent.b_source_change && eventLevel > 0 ){
                    if( ent.internal_clip_id == null ) {
                        if( ent.int_time > 0 ) {
                            ent.b_source_change = true;
                            eventLevel--;
                            Log.d(TAG, "t=" + ent.int_time + ", event=" + eventLevel);
                        }
                    }
                }
                newEffects.add(ent);
            }

            int startTime2 = 0;
            int newClipCount = newEffects.size()-1;
            for(int i2 = 0 ; i2 < newClipCount ; i2++){
                nexBeatTemplateManager.NewEntry ent2 = new nexBeatTemplateManager.NewEntry();
                ent2.int_stime = startTime2;
                ent2.color_filter_id = newEffects.get(i2).color_filter_id;
                ent2.internal_clip_id = newEffects.get(i2).internal_clip_id;
                if( newEffects.get(i2+1).b_source_change ) {
                    ent2.int_etime = newEffects.get(i2 + 1).int_time;
                }else{
                    ent2.int_etime = newEffects.get(i2 + 1).int_time - newEffects.get(i2+1).int_effect_in_duration;
                }
                startTime2 = ent2.int_etime;

                int dur = ent2.int_etime - ent2.int_stime;
                if( dur < 100 ){
                    Log.w(TAG,"W["+i2+"]("+ent2.int_stime+") lower duration ("+dur+")");
                }

                if( newEffects.get(i2).b_source_change ){
                    ent2.b_source_change = true;
                }else{

                    ent2.clip_effect_id = newEffects.get(i2).clip_effect_id;
                    ent2.clip_effect_start_time = 0;
                    ent2.clip_effect_end_time = newEffects.get(i2).int_effect_in_duration + newEffects.get(i2).int_effect_out_duration;

                    if(  ent2.clip_effect_end_time < 100 ){
                        if( i2 != 0 ) {
                            Log.w(TAG, "W[" + i2 + "](" + ent2.int_stime + ") lower effect duration (" + ent2.clip_effect_end_time + ")");
                        }
                    }

                    if(  dur < ent2.clip_effect_end_time ){
                        Log.w(TAG,"W["+i2+"]("+ent2.int_stime+") higher effect duration ("+dur+" < "+ ent2.clip_effect_end_time +")");
                        ent2.clip_effect_end_time = dur;
                    }
                }
                if( ent2.int_stime < ent2.int_etime)
                    resolveEntry.add(ent2);
            }

            for( nexBeatTemplateManager.NewEntry nent : resolveEntry){
                Log.d(TAG,"st="+nent.int_stime+",et="+nent.int_etime+",sc="+nent.b_source_change+",ei="+nent.clip_effect_id+", cf="+nent.color_filter_id+", et="+nent.clip_effect_end_time+", is="+nent.internal_clip_id  );
            }

            nexProject source = nexProject.clone(project);
            project.allClear(true);
            int count = source.getTotalClipCount(true);
            int index = 0;
            nexColorEffect [] colorEffects = nexColorEffect.getInternalPresetList();
            int colorEffectIndex = 0;

            newClipCount = resolveEntry.size();
            for( int i = 0 ; i < newClipCount ; i++){

                nexBeatTemplateManager.NewEntry rent = resolveEntry.get(i);

                if( rent.b_source_change ){
                    if( rent.internal_clip_id == null ) {
                        index++;
                        if (index >= count) {
                            index = 0;
                        }
                    }
                }

                if( rent.internal_clip_id != null ){
                    project.add(new nexClip(rent.internal_clip_id, "@assetItem:"+rent.internal_clip_id));
                    project.getLastPrimaryClip().setAssetResource(true);
                }else {
                    project.add(nexClip.dup(source.getClip(index, true)));
                    project.getLastPrimaryClip().setRotateDegree(source.getClip(index, true).getRotateDegree());
                }

                if (project.getLastPrimaryClip().getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    project.getLastPrimaryClip().setImageClipDuration(rent.int_etime - rent.int_stime);
                    project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);
                    if (rent.color_filter_id != null) {
                        if (rent.color_filter_id.compareToIgnoreCase("rand") == 0) {
                            project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                            colorEffectIndex++;
                            if (colorEffectIndex >= colorEffects.length) {
                                colorEffectIndex = 0;
                            }
                        } else {
                            for (nexColorEffect ce : nexColorEffect.getPresetList()) {
                                if (ce.getPresetName().compareToIgnoreCase(rent.color_filter_id) == 0) {
                                    project.getLastPrimaryClip().setColorEffect(ce);
                                    break;
                                }
                            }
                        }
                    }

                    if (rent.clip_effect_id != null) {
                        if( rent.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                            project.getLastPrimaryClip().getClipEffect().setEffect(rent.clip_effect_id);
                            project.getLastPrimaryClip().getClipEffect().setEffectShowTime(rent.clip_effect_start_time, rent.clip_effect_end_time);
                        }
                    }
                }else if( project.getLastPrimaryClip().getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                    int startTime = source.getClip(index, true).getVideoClipEdit().getStartTrimTime();
                    int endTime = source.getClip(index, true).getVideoClipEdit().getEndTrimTime();
                    boolean loop = true;
                    boolean change = false;
                    int duration = rent.int_etime - rent.int_stime;

                    if( (startTime + duration) > endTime ){
                        continue;
                    }

                    while(loop){
                        project.getLastPrimaryClip().getVideoClipEdit().setTrim(startTime, startTime + duration);
                        startTime += duration;

                        project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);

                        if( change ){
                            project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                            colorEffectIndex++;
                            if (colorEffectIndex >= colorEffects.length) {
                                colorEffectIndex = 0;
                            }
                        }else {

                            if (rent.color_filter_id != null) {
                                if (rent.color_filter_id.compareToIgnoreCase("rand") == 0) {
                                    project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                                    colorEffectIndex++;
                                    if (colorEffectIndex >= colorEffects.length) {
                                        colorEffectIndex = 0;
                                    }
                                } else {
                                    for (nexColorEffect ce : nexColorEffect.getPresetList()) {
                                        if (ce.getPresetName().compareToIgnoreCase(rent.color_filter_id) == 0) {
                                            project.getLastPrimaryClip().setColorEffect(ce);
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if (rent.clip_effect_id != null) {
                            if( rent.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                                project.getLastPrimaryClip().getClipEffect().setEffect(rent.clip_effect_id);
                                project.getLastPrimaryClip().getClipEffect().setEffectShowTime(rent.clip_effect_start_time, rent.clip_effect_end_time);
                            }
                        }

                        i++;
                        if( i >= newClipCount ){
                            break;
                        }

                        if( rent.b_source_change) {
                            change = true;
                        }else{
                            change = false;
                        }

                        rent = resolveEntry.get(i);
                        duration = rent.int_etime - rent.int_stime;

                        if( (startTime + duration) > endTime){
                            break;
                        }
                        project.add(nexClip.dup(source.getClip(index, true)));
                        project.getLastPrimaryClip().setRotateDegree(source.getClip(index, true).getRotateDegree());
                    }
                    index++;
                    if( index >= count ){
                        index = 0;
                    }
                    continue;
                }
            }

            project.setBackgroundMusicPath(meta.string_audio_id);
            project.setProjectAudioFadeInTime(0);
            project.setProjectAudioFadeOutTime(0);

        }else{
            return false;
        }
        return true;
    }

    public boolean applyTemplateToProjectById(nexProject project , String musicTemplateId ) {
        BeatTemplate b = getBeatTemplate(musicTemplateId);
        if( b == null ){
            return false;
        }

        if( nexAssetPackageManager.checkExpireAsset(b.packageInfo()) ){
            //throw new ExpiredTimeException();
            return false;
        }
        return internalApplyTemplateToProjectById( project , musicTemplateId );
    }

    public boolean applyTemplateToProjectById2(nexProject project , String musicTemplateId ){
        String jsonString = nexTemplateComposer.AssetPackageTemplateJsonToString(musicTemplateId);
        Level level = Level.Max;
        if (jsonString != null){
            int minDuration = 10800000;
            int start = 0;

            Gson gson = new Gson();
            TemplateMetaData.Music meta = gson.fromJson(jsonString, TemplateMetaData.Music.class);

            int maxCount = -1;
            int recommendCount = 1;
            int extendsCount = 0;
            int internalCount = 0;
            for( TemplateMetaData.EffectEntry entry : meta.list_effectEntries ){
                if( start > 0 ) {
                    if (minDuration > (entry.int_time - start)) {
                        minDuration = entry.int_time-start;
                    }
                }
                start = entry.int_time;
                maxCount++;
                if( entry.b_source_change || entry.int_priority > 0 ){
                    if( meta.default_effect != null ) {
                        if (entry.color_filter_id == null) {
                            entry.color_filter_id = meta.default_effect.sc_color_effect_id;
                        }

                        if (entry.clip_effect_id == null) {
                            entry.clip_effect_id = meta.default_effect.sc_clip_effect_id;
                        }
                    }
                    if( entry.int_priority > 0 ){
                        extendsCount++;
                    }else{
                        if( entry.internal_clip_id != null ){
                            internalCount++;
                        }else {
                            recommendCount++;
                        }
                    }
                }else {
                    if( meta.default_effect != null ) {
                        if (entry.color_filter_id == null) {
                            entry.color_filter_id = meta.default_effect.color_filter_id;
                        }

                        if (entry.clip_effect_id == null) {
                            entry.clip_effect_id = meta.default_effect.clip_effect_id;
                        }
                    }

                    if( entry.clip_effect_id != null ) {
                        if( entry.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                            if (entry.int_effect_in_duration == 0) {
                                entry.int_effect_in_duration = 50;
                            }

                            if (entry.int_effect_out_duration == 0) {
                                entry.int_effect_out_duration = 50;
                            }
                        }
                    }
                }
            }

            int sourceClipCount = project.getTotalClipCount(true);
            int priorityLevel = 0;
            int eventLevel = 0;

            if( sourceClipCount > recommendCount ){
                priorityLevel = sourceClipCount - recommendCount;
                if( priorityLevel > extendsCount ){
                    eventLevel = priorityLevel-extendsCount;
                    priorityLevel = extendsCount;
                }
            }

            if( level == Level.Extends ){
                eventLevel = 0;
            }else if( level == Level.Recommend){
                priorityLevel = 0;
                eventLevel = 0;
            }

            Log.d(TAG,"sourceClipCount="+sourceClipCount+", recommend="+recommendCount+", extends="+extendsCount+" ,max="+maxCount+", priorityLevel="+priorityLevel+", eventLevel="+eventLevel+", internal="+internalCount);

            int newClipCount = meta.list_effectEntries.size();
            ArrayList<nexBeatTemplateManager.NewEntry> resolveEntry = new ArrayList<>();

            int lastTime = 0;
            for(int ii = 1 ; ii < newClipCount-1  ; ii++ ){
                nexBeatTemplateManager.NewEntry ent = new nexBeatTemplateManager.NewEntry();
                ent.b_source_change = meta.list_effectEntries.get(ii).b_source_change;

                if( meta.list_effectEntries.get(ii).int_priority > 0 ){
                    if( priorityLevel == 0 )
                        continue;

                    if( meta.list_effectEntries.get(ii).int_priority <= priorityLevel ){
                        ent.b_source_change = true;
                        Log.d(TAG,"num="+ii+", priority="+ meta.list_effectEntries.get(ii).int_priority );
                    }
                }

                if( !ent.b_source_change && eventLevel > 0 ){
                    if( ent.internal_clip_id == null ) {
                        ent.b_source_change = true;
                        eventLevel--;
                        Log.d(TAG, "num=" + ii + ", event=" + eventLevel);
                    }
                }

                ent.clip_effect_id = meta.list_effectEntries.get(ii).clip_effect_id;
                ent.color_filter_id = meta.list_effectEntries.get(ii).color_filter_id;
                ent.internal_clip_id = meta.list_effectEntries.get(ii).internal_clip_id;

                if( !ent.b_source_change ){
                    nexBeatTemplateManager.NewEntry gap = new nexBeatTemplateManager.NewEntry();
                    gap.int_stime = lastTime;
                    gap.clip_effect_id = "none";
                    gap.color_filter_id = "none";
                    gap.internal_clip_id = ent.internal_clip_id;
                    ent.int_stime = meta.list_effectEntries.get(ii).int_time - meta.list_effectEntries.get(ii).int_effect_in_duration;
                    ent.clip_effect_id = meta.list_effectEntries.get(ii).clip_effect_id;
                    gap.int_etime = ent.int_stime;
                    ent.int_etime = ent.int_stime+ (meta.list_effectEntries.get(ii).int_effect_in_duration+ meta.list_effectEntries.get(ii).int_effect_out_duration);
                    lastTime = ent.int_etime;
                    if( gap.int_stime < gap.int_etime ) {
                        resolveEntry.add(gap);
                    }

                }else{
                    ent.int_stime = lastTime;
                    ent.int_etime = meta.list_effectEntries.get(ii).int_time;
                    if( ent.int_stime >= ent.int_etime )
                        continue;
                    lastTime = ent.int_etime;
                }
                resolveEntry.add(ent);
            }

            nexBeatTemplateManager.NewEntry lent = new nexBeatTemplateManager.NewEntry();
            lent.b_source_change = false;
            lent.int_stime = lastTime;
            lent.int_etime = meta.list_effectEntries.get(newClipCount-1).int_time;
            lent.clip_effect_id = meta.list_effectEntries.get(newClipCount-1).clip_effect_id;
            lent.color_filter_id = meta.list_effectEntries.get(newClipCount-1).color_filter_id;
            resolveEntry.add(lent);
            Log.d(TAG,"s= ,gap duration="+minDuration);
            for( nexBeatTemplateManager.NewEntry nent : resolveEntry){
                Log.d(TAG,"s="+nent.int_stime+",e="+nent.int_etime+",c="+nent.b_source_change+",ee="+nent.clip_effect_id+", sc="+nent.internal_clip_id );
            }

            nexProject source = nexProject.clone(project);
            project.allClear(true);
            int count = source.getTotalClipCount(true);
            int index = 0;
            nexColorEffect [] colorEffects = nexColorEffect.getInternalPresetList();
            int colorEffectIndex = 0;

            //for( NewEntry rent : resolveEntry){
            newClipCount = resolveEntry.size();
            for( int i = 0 ; i < newClipCount ; i++){

                nexBeatTemplateManager.NewEntry rent = resolveEntry.get(i);
                if( rent.internal_clip_id != null ){
                    project.add(new nexClip(rent.internal_clip_id));
                }else {
                    project.add(nexClip.dup(source.getClip(index, true)));
                }

                if (project.getLastPrimaryClip().getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                    project.getLastPrimaryClip().setImageClipDuration(rent.int_etime - rent.int_stime);
                    //project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.PANORAMA);
                    project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);
                    if (rent.color_filter_id != null) {
                        if (rent.color_filter_id.compareToIgnoreCase("rand") == 0) {
                            project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                            colorEffectIndex++;
                            if (colorEffectIndex >= colorEffects.length) {
                                colorEffectIndex = 0;
                            }
                        } else {
                            for (nexColorEffect ce : nexColorEffect.getPresetList()) {
                                if (ce.getPresetName().compareToIgnoreCase(rent.color_filter_id) == 0) {
                                    project.getLastPrimaryClip().setColorEffect(ce);
                                    break;
                                }
                            }
                        }
                    }

                    if (rent.clip_effect_id != null) {
                        if( rent.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                            project.getLastPrimaryClip().getClipEffect().setEffect(rent.clip_effect_id);
                        }
                    }
                }else if( project.getLastPrimaryClip().getClipType() == nexClip.kCLIP_TYPE_VIDEO ){
                    int startTime = source.getClip(index, true).getVideoClipEdit().getStartTrimTime();
                    int endTime = source.getClip(index, true).getVideoClipEdit().getEndTrimTime();
                    boolean loop = true;
                    boolean change = false;
                    int duration = rent.int_etime - rent.int_stime;

                    if( (startTime + duration) > endTime ){
                        continue;
                    }

                    while(loop){
                        project.getLastPrimaryClip().getVideoClipEdit().setTrim(startTime, startTime + duration);
                        startTime += duration;

                        project.getLastPrimaryClip().getCrop().randomizeStartEndPosition(false, nexCrop.CropMode.FILL);

                        if( change ){
                            project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                            colorEffectIndex++;
                            if (colorEffectIndex >= colorEffects.length) {
                                colorEffectIndex = 0;
                            }
                        }else {

                            if (rent.color_filter_id != null) {
                                if (rent.color_filter_id.compareToIgnoreCase("rand") == 0) {
                                    project.getLastPrimaryClip().setColorEffect(colorEffects[colorEffectIndex]);
                                    colorEffectIndex++;
                                    if (colorEffectIndex >= colorEffects.length) {
                                        colorEffectIndex = 0;
                                    }
                                } else {
                                    for (nexColorEffect ce : nexColorEffect.getPresetList()) {
                                        if (ce.getPresetName().compareToIgnoreCase(rent.color_filter_id) == 0) {
                                            project.getLastPrimaryClip().setColorEffect(ce);
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if (rent.clip_effect_id != null) {
                            if( rent.clip_effect_id.compareToIgnoreCase("none") != 0 ) {
                                project.getLastPrimaryClip().getClipEffect().setEffect(rent.clip_effect_id);
                            }
                        }

                        i++;
                        if( i >= newClipCount ){
                            break;
                        }

                        if( rent.b_source_change) {
                            change = true;
                        }else{
                            change = false;
                        }

                        rent = resolveEntry.get(i);
                        duration = rent.int_etime - rent.int_stime;

                        if( (startTime + duration) > endTime){
                            break;
                        }
                        project.add(nexClip.dup(source.getClip(index, true)));
                    }
                    index++;
                    if( index >= count ){
                        index = 0;
                    }
                    continue;
                }
                if( rent.b_source_change ){
                    if( rent.internal_clip_id == null ) {
                        index++;
                        if (index >= count) {
                            index = 0;
                        }
                    }
                }
            }

            project.setBackgroundMusicPath(meta.string_audio_id);
            project.setProjectAudioFadeInTime(0);
            project.setProjectAudioFadeOutTime(0);

        }else{
            return false;
        }

        return true;
    }

    boolean updateBeatTemplate(boolean installed , nexAssetPackageManager.Item item){
        synchronized (m_musicTemplateLock) {
            boolean update = false;
            Log.d(TAG,"updateMusicTemplate("+installed+","+item.packageInfo().assetIdx()+")");
            if(installed){
                if (!item.hidden()) {
                    BeatTemplate t = new BeatTemplate(item);
                    musicTemplates.add(t);
                }
            }else{
                for (BeatTemplate m : musicTemplates) {
                    if( m.id().compareTo(item.id()) == 0 ){
                        musicTemplates.remove(m);
                        break;
                    }
                }
            }
            return update;
        }
    }
}
