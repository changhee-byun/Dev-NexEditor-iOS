package com.nexstreaming.kminternal.kinemaster.editorwrapper;

import android.util.SparseArray;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.BounceInterpolator;
import android.view.animation.Interpolator;
import android.view.animation.OvershootInterpolator;

import com.nexstreaming.app.common.util.SimplexNoise;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRenderer;
import com.nexstreaming.kminternal.nexvideoeditor.LayerRendererInterface;

public enum LayerExpression {

    // When adding new expression types, DO NOT change existing expression ID values: They are stored in saved projects
    None(0,null),
    Fade(1, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1 - p);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }
    }),
    Pop(2, new Imp() {

        Interpolator interp_in = new OvershootInterpolator();
        Interpolator interp_out = new AccelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp_in.getInterpolation(p0);
            renderer.setAlpha(p);
            renderer.scale(p,p,cx,cy);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp_out.getInterpolation(p0);
            renderer.setAlpha(1-p);
            renderer.scale(1+p,1+p,cx,cy);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }
    }),
    Slide(3, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.translate((1-p)*-100,0);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1-p);
            renderer.translate(p*100,0);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {

        }
    }),
    Spin(4, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.rotate(360*2*(1-p),cx,cy);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1-p);
            renderer.rotate(360 * 2 * p, cx, cy);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {

        }
    }),
    Drop(5, new Imp() {

        Interpolator interp_in = new BounceInterpolator();
        Interpolator interp_out = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp_in.getInterpolation(p0);
            renderer.translate(0, (1 - p) * -1200);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp_out.getInterpolation(p0);
            renderer.setAlpha(1 - p);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {

        }
    }),
    Scale(6, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(p);
            renderer.scale(p,p,cx,cy);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1-p);
            renderer.scale(1-p,1-p,cx,cy);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }
    }),
    Floating(7, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1 - p);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }

        @Override
        void prepOverall(LayerRendererInterface renderer, float _cx, float _cy, float p, int localTime, int totalTime) {
            float cx = _cx / 500 + totalTime*10;
            float cy = _cy / 500 + totalTime*10;
            double dx = 10*SimplexNoise.noise(cx,cy,(float)localTime/500);
            double dy = 30*SimplexNoise.noise(cx+7534.5431,cy+123.432,(float)localTime/500);
            double dx2 = 4*SimplexNoise.noise(cx+543,cy+823.25,(float)localTime/300+234);
            double dy2 = 10*SimplexNoise.noise(cx+734.5431,cy+13.432,(float)localTime/300+567);
            renderer.translate((float)(dx+dx2),(float)(dy+dy2));
        }
    }),
    Drifting(8, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1 - p);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }

        @Override
        void prepOverall(LayerRendererInterface renderer, float _cx, float _cy, float p, int localTime, int totalTime) {
            float cx = _cx / 500 + totalTime*10;
            float cy = _cy / 500 + totalTime*10;
            double dx = 40*SimplexNoise.noise(cx,cy,(float)localTime/1000);
            double dy = 40*SimplexNoise.noise(cx+7534.5431,cy+123.432,(float)localTime/1000);
            double dx2 = 10*SimplexNoise.noise(cx+543,cy+823.25,(float)localTime/500+234);
            double dy2 = 10*SimplexNoise.noise(cx+734.5431,cy+13.432,(float)localTime/500+567);
            renderer.translate((float)(dx+dx2),(float)(dy+dy2));
        }
    }),
    Squishing(9, new Imp() {

        Interpolator interp = new AccelerateDecelerateInterpolator();

        @Override
        public void prepInPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(p);
        }

        @Override
        public void prepOutPart(LayerRendererInterface renderer, float cx, float cy, float p0) {
            float p = interp.getInterpolation(p0);
            renderer.setAlpha(1 - p);
        }

        @Override
        public void prepMidPart(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration) {
        }

        @Override
        void prepOverall(LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int totalTime) {
            float sx = (float) (Math.sin((float)localTime/400f)*0.2f + 1f);
            float sy = (float) (Math.cos((float)localTime/400f)*0.2f + 1f);
            renderer.scale(sx,sy,cx,cy);
        }
    }),

    ;
    private final int id;
    private final Imp imp;
    private LayerExpression( int id, Imp imp ) {
        this.id = id;
        this.imp = imp;
    }
    public int getId() {
        return id;
    }
    private static final SparseArray<LayerExpression> idToItemMap = new SparseArray<LayerExpression>() {{
        for( LayerExpression e: values() ) {
            put(e.getId(),e);
        }
    }};
    public static LayerExpression fromId( int id ) {
        return idToItemMap.get(id);
    }

    public void rendererSetup( LayerRendererInterface renderer, float cx, float cy, int localTime, int itemDuration, int inTime, int outTime ) {

        if( imp==null )
            return;

        int adjInTime;
        int adjOutTime;

        if( inTime+outTime > itemDuration ) {
            float p = (float)inTime / (float)(inTime+outTime);
            adjInTime = (int)(p * itemDuration);
            adjOutTime = itemDuration - adjInTime;
        } else {
            adjInTime = inTime;
            adjOutTime = outTime;
        }

        imp.prepOverall(renderer, cx, cy, (float) localTime / (float) itemDuration, localTime, itemDuration);

        if( localTime <= adjInTime ) {
            imp.prepInPart(renderer,cx,cy,(float)localTime/(float)adjInTime);
        } else if ( localTime >= itemDuration-adjOutTime ) {
            imp.prepOutPart(renderer, cx, cy, (float) (localTime - (itemDuration - adjOutTime)) / (float) adjOutTime);
        } else {
            imp.prepMidPart(renderer, cx, cy, (float) (localTime - adjInTime) / (float) (itemDuration - adjInTime - adjOutTime), localTime - adjInTime, itemDuration - adjInTime - adjOutTime);
        }
    }

    private static abstract class Imp {
        abstract void prepInPart( LayerRendererInterface renderer, float cx, float cy, float p );
        abstract void prepOutPart( LayerRendererInterface renderer, float cx, float cy, float p );
        abstract void prepMidPart( LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int midDuration );
        void prepOverall( LayerRendererInterface renderer, float cx, float cy, float p, int localTime, int totalTime ) {

        }
    }

}
