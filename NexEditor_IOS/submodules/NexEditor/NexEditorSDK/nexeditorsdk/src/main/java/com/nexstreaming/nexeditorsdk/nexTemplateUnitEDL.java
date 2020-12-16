/******************************************************************************
 * File Name        : nexTemplate.java
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

package com.nexstreaming.nexeditorsdk;

import android.content.Context;
import android.util.Log;

import java.util.List;

final class nexTemplateUnitEDL {
    private static String TAG="nexTemplateUnitEDL";

    public nexTemplateUnitEDL parent = null;
    public nexTemplateUnitEDL prev = null;
    public nexTemplateUnitEDL next = null;
    public nexTemplateUnitEDL child = null;

    public int type = 0; // 0: master, 1:sub
    public int clip_type = 0; // 0:image, 1:video, 2: system
    public int clip_start = 0;
    public int clip_end = 0;

    public int master_idx = 0;
    public int sub_idx = 0;

    public int duration = 0;
    public int transition_duration = 0;
    public int start = 0;
    public int end = 0;

    public nexClip clip = null;

    public boolean empty = true;
    public String alternative = null;

    public int useVideoMemSize = 0;

    public nexTemplateEffect template_effect = null;
    public nexTemplateDrawInfo template_drawinfo = null;

    public nexTemplateUnitEDL dup() {
        nexTemplateUnitEDL u = new nexTemplateUnitEDL();

        u.type = this.type; // 0: master, 1:sub
        u.clip_start = this.clip_start;
        u.clip_end = this.clip_end;

        u.master_idx = this.master_idx;
        u.sub_idx = this.sub_idx;

        u.duration = this.duration;
        u.start = this.start;
        u.end = this.end;

        u.empty = this.empty;
        u.alternative = this.alternative;

        return u;
    }

    public void addLastNext(nexTemplateUnitEDL unit) {
        if( next == null)
        {
            unit.prev = this;
            next = unit;
            return;
        }

        nexTemplateUnitEDL c = next;
        while( c.next != null )
            c = c.next;

        unit.prev = c;
        c.next = unit;
    }

    public void addChild(nexTemplateUnitEDL unit) {
        unit.parent = this;

        if( child == null)
        {
            child = unit;
            return;
        }

        child.addLastNext(unit);
    }

    public void removeChild(nexTemplateUnitEDL unit) {

        if( child == unit )
        {
            return;
        }

        nexTemplateUnitEDL c = child;
        while( c != null && c.equals(unit) == false )
            c = c.next;

        if( c == null ) return;

        while( c.next != null )
            c = c.next;

        unit.prev = c;
        c.next = unit;
    }

    public nexTemplateUnitEDL getLast() {
        nexTemplateUnitEDL u = next;
        if( u == null ) return null;

        while( u.next != null )
        {
            u = u.next;
        }
        return u;
    }

    public int getChildCount() {
        int count = 0;
        nexTemplateUnitEDL c = child;
        while( c != null ) {
            count++;
            c = c.next;
        }

        return count;
    }

    public nexTemplateUnitEDL getMaxDurationChildUnit() {
        nexTemplateUnitEDL cur, last;
        cur = last = child;
        while(cur != null ) {
            if( cur.duration > last.duration ) {
                last = cur;
            }
            cur = cur.next;
        }
        return last;
    }

    public nexTemplateUnitEDL getEmptyChildUnit() {
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty ) {
                return cur;
            }
            cur = cur.next;
        }
        return null;
    }

    public nexTemplateUnitEDL getFirstEmptyChildUnit() {
        nexTemplateUnitEDL p = prev;
        if( p != null ) {
            nexTemplateUnitEDL sub = p.getEmptyChildUnit();
            if( sub != null )
                return p.getFirstEmptyChildUnit();
        }
        return getEmptyChildUnit();
    }

    public nexTemplateUnitEDL findEmptyChildUnit(int clipTime) {
        nexTemplateUnitEDL p = prev;
        if( p != null && start > clipTime ) {
            nexTemplateUnitEDL sub = p.getEmptyChildUnit();
            if( sub != null )
                return p.getFirstEmptyChildUnit();
        }
        return getEmptyChildUnit();
    }

    public boolean isAllChildEmpty() {
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty == false && clip_type != 2 ) {
                return false;
            }
            cur = cur.next;
        }
        return true;
    }

    public int getTotalEmptyChildUnit() {
        int count = 0;
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty ) {
                count++;
            }
            cur = cur.next;
        }
        return count;
    }

    public int getTotalUsedChildUnit() {
        int count = 0;
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty == false ) {
                count++;
            }
            cur = cur.next;
        }
        return count;
    }

    public boolean hasVideoSourceChild() {
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty == false && cur.clip_type == 1) {
                return true;
            }
            cur = cur.next;
        }
        return false;
    }

    public boolean isAvailableVideo2Top(nexClip clip) {

        if( child == null ) return false;

        int curSize = clip.getWidth()*clip.getHeight();
        int childSize = 0;
        int imageCount = 0;
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.clip == clip ) return false;

            if( cur.clip_type != 0 ) {
                childSize = childSize + (cur.clip.getWidth() * cur.clip.getHeight());
            }
            else {
                imageCount++;
            }
            cur = cur.next;
        }

        if( (nexTemplate.mVideoMemorySize - childSize) < curSize || imageCount <= 0)
            return false;
        return true;
    }

    public boolean hasEmptyChild() {
        nexTemplateUnitEDL cur = child;
        while(cur != null ) {
            if( cur.empty ) {
                return true;
            }
            cur = cur.next;
        }
        return false;
    }

    public int getTotalEmptyUnit() {
        int count = 0;
        nexTemplateUnitEDL cur = next;
        while(cur != null ) {
            count += cur.getTotalEmptyChildUnit();
            cur = cur.next;
        }
        return count;
    }

    public boolean applyTemplateUnit2Project(nexProject project, Context context, int startTime, float template_ratio) {
        start = startTime;
        end = startTime + duration;

        List<nexDrawInfo> te_top = template_effect.makeTopDrawInfos(startTime, 0, null);

        nexClip tmpClip = null;
        nexTemplateUnitEDL sub = child;
        while( sub != null ) {

            if( sub.template_drawinfo != null ) {
                if( sub.clip_type == 2 ) {

                    if( sub.clip.getAttachmentState() == false ) {
                        project.add(sub.clip);
                        sub.clip.clearDrawInfos();
                        sub.clip.mStartTime = startTime;
                        sub.clip.mEndTime = startTime + sub.clip.getTotalTime();

                        sub.template_drawinfo.setDrawInfo2Clip(sub.clip, template_effect.id, duration, startTime, template_ratio, null, false);
                    }

                    // sub.template_drawinfo.applyResouceClip(project, context, template_effect, startTime, template_ratio);
                    sub = sub.next;
                    continue;
                }

                if( sub.clip != null )
                    tmpClip = sub.clip;

                if( tmpClip != null ) {
                    if (tmpClip.getAttachmentState() == false) {
                        project.add(tmpClip);
                        if (tmpClip.getClipType() == nexClip.kCLIP_TYPE_IMAGE) {
                            tmpClip.mStartTime = Integer.MAX_VALUE;
                            tmpClip.mEndTime = Integer.MIN_VALUE;
                        } else {
                            tmpClip.mStartTime = startTime;
                            tmpClip.mEndTime = startTime + tmpClip.getProjectDuration();
                        }
                    }

                    sub.template_drawinfo.setDrawInfo2Clip(tmpClip, template_effect.id, duration, startTime, template_ratio, null, false);
                    sub.template_drawinfo.applySoundEffect(project, startTime, duration);
                }

            }
            sub = sub.next;
        }

        startTime = setTopDrawInfo2Project(project, te_top);

        if( next != null )
        {
            return next.applyTemplateUnit2Project(project, context, startTime, template_ratio);
        }

        return true;
    }

    int setTopDrawInfo2Project(nexProject project, List<nexDrawInfo> tops) {

        int start = 0;
        for (nexDrawInfo in : tops) {
            project.getTopDrawInfo().add(in);
            if (in.getIsTransition() == 1)
                start = in.getStartTime();
            else {
                start = in.getEndTime();
            }
        }
        return start;
    }

    public void calcTime(int startTime) {
        start = startTime;
        end = startTime + duration;

        if( child != null )
        {
            nexTemplateUnitEDL sub = child;
            while(sub != null ) {
                sub.start = start + (int)(duration * sub.template_drawinfo.start);
                sub.end = start + (int)(duration * sub.template_drawinfo.end);
                sub = sub.next;

                if( sub.clip != null ) {
                    if( sub.clip.getClipType() == nexClip.kCLIP_TYPE_IMAGE ) {
                        sub.clip_start = start;
                    }
                    else {

                    }
               }
            }
        }

        if( next != null )
            next.calcTime(end);
    }

    public void printInfo() {
        if( type == 0 ) {
            Log.d(TAG, String.format("unitedl: -----------------master-----------------"));
            Log.d(TAG, String.format("unitedl: start: %d", start));
            Log.d(TAG, String.format("unitedl: end: %d", end));
            Log.d(TAG, String.format("unitedl: duration: %d", duration));
            if( template_effect != null )
                Log.d(TAG, String.format("unitedl: effect: %s", template_effect.effect_id));
            Log.d(TAG, String.format("unitedl: ---------------------------------------"));
        }
        else {
            Log.d(TAG, String.format("unitedl: -----------------sub-----------------"));
            Log.d(TAG, String.format("unitedl: empty: %b", empty));
            Log.d(TAG, String.format("unitedl: clip_type: %d", clip_type));
            Log.d(TAG, String.format("unitedl: start: %d", start));
            Log.d(TAG, String.format("unitedl: end: %d", end));
            if( clip != null ) {
                Log.d(TAG, String.format("unitedl: clip path: %s", clip.getPath()));
                Log.d(TAG, String.format("unitedl: clip dur: %d", clip.getProjectDuration()));
                Log.d(TAG, String.format("unitedl: clip_start: %d", clip_start));
                Log.d(TAG, String.format("unitedl: clip_end: %d", clip_end));
            }
            Log.d(TAG, String.format("unitedl: -------------------------------------"));
        }

        if( child != null )
            child.printInfo();

        if( next != null )
            next.printInfo();
    }
}
