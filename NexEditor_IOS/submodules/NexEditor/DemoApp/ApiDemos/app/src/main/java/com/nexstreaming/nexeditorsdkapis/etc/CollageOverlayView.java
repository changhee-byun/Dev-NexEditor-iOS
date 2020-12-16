package com.nexstreaming.nexeditorsdkapis.etc;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PointF;
import android.graphics.Rect;
import android.util.Log;
import android.view.View;
import android.widget.AbsoluteLayout;
import android.widget.Button;
import android.widget.ImageView;

import com.nexstreaming.nexeditorsdk.nexCollageInfoDraw;
import com.nexstreaming.nexeditorsdk.nexCollageInfo;
import com.nexstreaming.nexeditorsdk.nexCollageInfoTitle;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;

/**
 * Created by songkyebeom on 11/07/2017.
 */

public class CollageOverlayView extends View {

    private List<nexCollageInfo> infos;
    private nexCollageInfo drawInfo;
    private nexCollageInfo titleInfo;
    private final Object lock = new Object();

    Paint drawPaint;
    Paint titlePaint;
    Paint selPaint;
    public CollageOverlayView(Context context) {
        super(context);

        drawPaint = new Paint();
        drawPaint.setStyle(Paint.Style.STROKE);
        drawPaint.setStrokeWidth(5);
        drawPaint.setColor(Color.argb(255, 255, 0, 0));

        titlePaint = new Paint();
        titlePaint.setStyle(Paint.Style.STROKE);
        titlePaint.setStrokeWidth(5);
        titlePaint.setColor(Color.argb(255, 0, 255, 0));

        selPaint = new Paint();
        selPaint.setStyle(Paint.Style.STROKE);
        selPaint.setStrokeWidth(5);
        selPaint.setColor(Color.argb(255, 0, 0, 255));

        setWillNotDraw(false);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        synchronized (lock) {
            if( infos != null ) {
                int viewWidth = this.getMeasuredWidth();
                int viewHeight = this.getMeasuredHeight();

                for(nexCollageInfo collageInfo : infos) {
                    int width = (int) (collageInfo.getRectangle().width() * viewWidth);
                    int height = (int) (collageInfo.getRectangle().height() * viewHeight);
                    int x = (int) (collageInfo.getRectangle().left * viewWidth);
                    int y = (int) (collageInfo.getRectangle().top * viewHeight);

                    Path path = new Path();
                    Iterator<PointF> positions = collageInfo.getPositions().iterator();
                    if( positions.hasNext() ) {
                        PointF pos = positions.next();

                        path.moveTo((int)(pos.x*viewWidth + 0.5), (int)(pos.y*viewHeight+0.5));
                        while(positions.hasNext()) {
                            pos = positions.next();
                            path.lineTo((int)(pos.x*viewWidth + 0.5), (int)(pos.y*viewHeight+0.5));
                        }
                        path.close();
                        if( collageInfo instanceof nexCollageInfoDraw ) {
                            if( collageInfo.equals(drawInfo) ) {
                                canvas.drawPath(path, selPaint);
                            }
                            else {
                                canvas.drawPath(path, drawPaint);
                            }
                        }
                        else if( collageInfo instanceof nexCollageInfoTitle) {
                            if( collageInfo.equals(titleInfo) ) {
                                canvas.drawPath(path, selPaint);
                            }
                            else {
                                canvas.drawPath(path, titlePaint);
                            }
                        }
                    }
                }
            }
        }
    }

    public void setCollageInfo(List<nexCollageInfo> infos) {
        synchronized (lock) {
            this.infos = infos;
        }
    }

    public boolean hasCollageInfo() {
        if( this.infos == null ) return false;
        return true;
    }

    public void setSelCollageInfo(nexCollageInfo draw, nexCollageInfo title) {
        synchronized (lock) {
            this.drawInfo = draw;
            this.titleInfo = title;
        }
    }

}
