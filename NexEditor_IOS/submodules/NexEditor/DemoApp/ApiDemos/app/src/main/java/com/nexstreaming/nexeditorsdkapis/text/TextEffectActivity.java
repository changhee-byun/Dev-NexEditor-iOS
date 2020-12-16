package com.nexstreaming.nexeditorsdkapis.text;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.os.Build;
import android.os.Bundle;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Switch;

import com.nexstreaming.kminternal.nexvideoeditor.NexTextEffect;
import com.nexstreaming.nexeditorsdkapis.R;
import com.nexstreaming.nexeditorsdkapis.dialog.ColorPickerDialog;

public class TextEffectActivity extends Activity {

    enum GlowType {
        innerGlow,
        outerGlow,
        bothGlow //Both
    }
    public class NexTextEffectInfo {

        public class ShadowInfo {
            int color = 0xFF7F7F7F;
            float radius = 2; //dp
            float dx = 0; //dp
            float dy = 0;//dp
            boolean used = false;
        }

        public class GlowInfo{
            int color = 0xFF7F7F7F;
            float radius = 2;
            float dx = 0;
            float dy = 0;
            GlowType glowType = GlowType.innerGlow;
            boolean used = false;
        }

        public class OutlineInfo {
            int color = 0xFF7F7F7F;
            float strokeWidth = 2;
            int strokeColor = 0xFF7F7F7F;
            Paint.Join strokeJoin = Paint.Join.MITER;
            float strokeMiter = 10;
            boolean used = false;
        }

        public class EmbossInfo {
            float[] direction = {1,5,1};
            float ambient = 0.5f;
            float specular = 10;
            float blurRadius = 7.5f;
            boolean used = false;
        }

        public ShadowInfo shadow = new ShadowInfo();
        public GlowInfo glow = new GlowInfo();
        public OutlineInfo outline = new OutlineInfo();
        public EmbossInfo emboss = new EmbossInfo();

        float textSize = 32;//size The scaled pixel size.
        int textColor = Color.WHITE;
        int backgroundColor = Color.TRANSPARENT;
        int letterSpace = 0; // -50 ~ 50 and letter spacing can be supported from Lollipop
        int lineSpace = 0; // -50 ~ 50

    }


    private NexTextEffect textImage;
    private ImageView imageView;


    // for  pinching zoom in/out
    final static float STEP = 200;
    float mRatio = 15.0f;
    int mBaseDist;
    float mBaseRatio;
    int windowwidth;
    int windowheight;

    // default value;
    private NexTextEffectInfo effectInfo = new NexTextEffectInfo();


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_text_effect);

        final LinearLayout layout = (LinearLayout)findViewById(R.id.root_layout);
        ViewTreeObserver vto = layout.getViewTreeObserver();
        vto.addOnPreDrawListener(new ViewTreeObserver.OnPreDrawListener() {
            @Override
            public boolean onPreDraw() {
                if (layout.getMeasuredWidth() <= 0) { return false; }
                layout.getViewTreeObserver().removeOnPreDrawListener(this);
                updateImageView();
                return true;
            }
        });

        vto.addOnGlobalLayoutListener (new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                layout.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                int width  = layout.getMeasuredWidth();
                int height = layout.getMeasuredHeight();

            }
        });

        windowwidth = getWindowManager().getDefaultDisplay().getWidth();
        windowheight = getWindowManager().getDefaultDisplay().getHeight();

        setDefaultValue();
        initOptions();

        imageView = findViewById(R.id.imageView);
        imageView.setScaleType(ImageView.ScaleType.MATRIX);

        textImage = new NexTextEffect(getApplicationContext());
        textImage.setTextSize(effectInfo.textSize);
        textImage.setTextColor(effectInfo.textColor);
        textImage.setTypeface(null, Typeface.BOLD);
        textImage.setText("Hello! This is Text effect Test");

        //You can change the horizontal align and vertical align
        textImage.setGravity(Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);
        //textImage.setBackgroundColor();


        final int MAX = 80;

        SeekBar textSizeBar = findViewById(R.id.seekbar_text_size);
        textSizeBar.setProgress((int)(effectInfo.textSize));
        textSizeBar.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                effectInfo.textSize = progress;
                textImage.setTextSize(effectInfo.textSize);
                updateImageView();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        Button colorpicker = (Button) findViewById(R.id.text_color);
        colorpicker.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new ColorPickerDialog(TextEffectActivity.this, new ColorPickerDialog.OnColorChangedListener() {
                    @Override
                    public void colorChanged(int color) {
                        effectInfo.textColor = color;
                        textImage.setTextColor(color);
                        updateImageView();
                    }
                }, effectInfo.textColor).show();
            }
        });

        Button textOk = (Button) findViewById(R.id.text_ok);
        textOk.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText textBox = (EditText) findViewById(R.id.edit_text);
                textImage.setText(textBox.getText());
                updateImageView();
            }
        });

        EditText textBox = (EditText) findViewById(R.id.edit_text);
        textBox.setText(textImage.getText());


        // letter sapce
        SeekBar spaceBar = findViewById(R.id.seekBar_letter_space);
        spaceBar.setProgress((int)(effectInfo.letterSpace + 50));
        spaceBar.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                effectInfo.letterSpace = progress - 50; // -50~ 50
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    textImage.setLetterSpacing((float)effectInfo.letterSpace / 50); //ems unit
                }
                updateImageView();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        SeekBar linespaceBar = findViewById(R.id.seekBar_line_space);
        linespaceBar.setProgress((int)(effectInfo.letterSpace + 50));
        linespaceBar.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                effectInfo.lineSpace = progress - 50; // -50~ 50
                textImage.setLineSpacing(0, (float)(effectInfo.lineSpace + 50) / 50);
                updateImageView();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        // Shadow
        Button.OnClickListener onClickListenerForShadow = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                final View parent = v;
                LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final View popupView = layoutInflater.inflate(R.layout.text_shadow_menu, null);

                final PopupWindow popupWindow = new PopupWindow(
                        popupView,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);

                popupWindow.setBackgroundDrawable(new BitmapDrawable());
                popupWindow.setOutsideTouchable(true);
                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        //TODO do sth here on dismiss
                    }
                });

                ImageButton colorpicker = popupView.findViewById(R.id.colorpicker);
                colorpicker.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        new ColorPickerDialog(popupView.getContext(), new ColorPickerDialog.OnColorChangedListener() {
                            @Override
                            public void colorChanged(int color) {
                                if (parent.getId() == R.id.btShadow) {
                                    effectInfo.shadow.color = color;
                                }
                            }
                        }, effectInfo.shadow.color).show();
                    }
                });

                Button bt = popupView.findViewById(R.id.shadowapply);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                        int progress = seekBar.getProgress();
                        effectInfo.shadow.radius = (float) progress;

                        seekBar = popupView.findViewById(R.id.seekBarDX);
                        progress = seekBar.getProgress();
                        effectInfo.shadow.dx = progress - (MAX / 2);

                        seekBar = popupView.findViewById(R.id.seekBarDY);
                        progress = seekBar.getProgress();
                        effectInfo.shadow.dy = progress - (MAX / 2);

                        applyTextEffect(0);
                        popupWindow.dismiss();
                    }
                });
                bt = popupView.findViewById(R.id.shadowcancel);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        popupWindow.dismiss();
                    }
                });
                SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                seekBar.setProgress((int)effectInfo.shadow.radius);


                int progress = 20;
                progress = (int)effectInfo.shadow.dx + (MAX / 2);
                seekBar = popupView.findViewById(R.id.seekBarDX);
                seekBar.setProgress(progress);

                progress = (int)effectInfo.shadow.dy + (MAX / 2);
                seekBar = popupView.findViewById(R.id.seekBarDY);
                seekBar.setProgress(progress);

                popupWindow.showAsDropDown(v);
            }
        };

        ImageButton bt = findViewById(R.id.btShadow);
        bt.setOnClickListener(onClickListenerForShadow);

        //Glow
        Button.OnClickListener onClickListenerForGlow = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                final View parent = v;
                LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final View popupView = layoutInflater.inflate(R.layout.text_glow_menu, null);

                final PopupWindow popupWindow = new PopupWindow(
                        popupView,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);

                popupWindow.setBackgroundDrawable(new BitmapDrawable());
                popupWindow.setOutsideTouchable(true);
                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        //TODO do sth here on dismiss
                    }
                });

                ImageButton colorpicker = popupView.findViewById(R.id.colorpicker);
                colorpicker.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        new ColorPickerDialog(popupView.getContext(), new ColorPickerDialog.OnColorChangedListener() {
                            @Override
                            public void colorChanged(int color) {
                                if (parent.getId() == R.id.btGlow) {
                                    effectInfo.glow.color = color;
                                }
                            }
                        }, effectInfo.glow.color).show();
                    }
                });

                Button bt = popupView.findViewById(R.id.btApply);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                        int progress = seekBar.getProgress();
                        effectInfo.glow.radius = (float) progress;

                        RadioGroup radioGroup = (RadioGroup) popupView.findViewById(R.id.radioGlowType);
                        int radioButtonID = radioGroup.getCheckedRadioButtonId();
                        int glowType = 0;
                        if(radioButtonID == R.id.radioOuter)
                            effectInfo.glow.glowType = GlowType.outerGlow;
                        else if(radioButtonID == R.id.radioInner)
                            effectInfo.glow.glowType = GlowType.innerGlow;
                        else
                            effectInfo.glow.glowType = GlowType.bothGlow;

                        applyTextEffect(1);
                        popupWindow.dismiss();
                    }
                });
                bt = popupView.findViewById(R.id.btCancel);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        popupWindow.dismiss();
                    }
                });
                SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                seekBar.setProgress((int)effectInfo.glow.radius);

                RadioGroup radioGroup = (RadioGroup) popupView.findViewById(R.id.radioGlowType);
                if (effectInfo.glow.glowType == GlowType.outerGlow)
                    radioGroup.check(R.id.radioOuter);
                else if (effectInfo.glow.glowType == GlowType.innerGlow)
                    radioGroup.check(R.id.radioInner);
                else
                    radioGroup.check(R.id.radioBoth);
                popupWindow.showAsDropDown(v);

            }
        };
        bt = findViewById(R.id.btGlow);
        bt.setOnClickListener(onClickListenerForGlow);

        // Outline
        Button.OnClickListener onClickListenerForOutline = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                final View parent = v;
                LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final View popupView = layoutInflater.inflate(R.layout.text_outline_menu, null);

                final PopupWindow popupWindow = new PopupWindow(
                        popupView,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);

                popupWindow.setBackgroundDrawable(new BitmapDrawable());
                popupWindow.setOutsideTouchable(true);
                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        //TODO do sth here on dismiss
                    }
                });

                ImageButton colorpicker = popupView.findViewById(R.id.colorpicker);
                colorpicker.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        new ColorPickerDialog(popupView.getContext(), new ColorPickerDialog.OnColorChangedListener() {
                            @Override
                            public void colorChanged(int color) {
                                if (parent.getId() == R.id.btOutline) {
                                    effectInfo.outline.color = color;
                                }
                            }
                        }, effectInfo.outline.color).show();
                    }
                });

                Button bt = popupView.findViewById(R.id.btApply);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                        int progress = seekBar.getProgress();
                        effectInfo.outline.strokeWidth = (float) progress;

                        applyTextEffect(2);
                        popupWindow.dismiss();
                    }
                });
                bt = popupView.findViewById(R.id.btCancel);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        popupWindow.dismiss();
                    }
                });
                SeekBar seekBar = popupView.findViewById(R.id.seekBarR);
                seekBar.setProgress((int)effectInfo.outline.strokeWidth);

                popupWindow.showAsDropDown(v);

            }
        };
        bt = findViewById(R.id.btOutline);
        bt.setOnClickListener(onClickListenerForOutline);

        // Emboss
        Button.OnClickListener onClickListenerForEmboss = new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                final View parent = v;
                LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                final View popupView = layoutInflater.inflate(R.layout.text_emboss_menu, null);

                final PopupWindow popupWindow = new PopupWindow(
                        popupView,
                        ViewGroup.LayoutParams.WRAP_CONTENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT);

                popupWindow.setBackgroundDrawable(new BitmapDrawable());
                popupWindow.setOutsideTouchable(true);
                popupWindow.setOnDismissListener(new PopupWindow.OnDismissListener() {
                    @Override
                    public void onDismiss() {
                        //TODO do sth here on dismiss
                    }
                });


                Button bt = popupView.findViewById(R.id.btApply);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        SeekBar seekBar = popupView.findViewById(R.id.seekBarX);
                        int progress = seekBar.getProgress();
                        effectInfo.emboss.direction[0] = (float) (progress - 10) / 10;

                        seekBar = popupView.findViewById(R.id.seekBarY);
                        progress = seekBar.getProgress();
                        effectInfo.emboss.direction[1] = (float) (progress - 10) / 10;

                        seekBar = popupView.findViewById(R.id.seekBarZ);
                        progress = seekBar.getProgress();
                        effectInfo.emboss.direction[2] = (float) (progress - 10) / 10;

                        seekBar = popupView.findViewById(R.id.seekBarAm);
                        progress = seekBar.getProgress();
                        effectInfo.emboss.ambient = (float) (progress) / 10;

                        seekBar = popupView.findViewById(R.id.seekBarCo);
                        progress = seekBar.getProgress();
                        effectInfo.emboss.specular = (float) (progress);

                        seekBar = popupView.findViewById(R.id.seekBarBl);
                        progress = seekBar.getProgress();
                        effectInfo.emboss.blurRadius = (float) (progress);

                        applyTextEffect(3);
                        popupWindow.dismiss();
                    }
                });
                bt = popupView.findViewById(R.id.btCancel);
                bt.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        popupWindow.dismiss();
                    }
                });
                SeekBar seekBar = popupView.findViewById(R.id.seekBarX);
                seekBar.setProgress((int)(effectInfo.emboss.direction[0] * 10 + 10));
                seekBar = popupView.findViewById(R.id.seekBarY);
                seekBar.setProgress((int)(effectInfo.emboss.direction[1] * 10 + 10));
                seekBar = popupView.findViewById(R.id.seekBarZ);
                seekBar.setProgress((int)(effectInfo.emboss.direction[2] * 10 + 10));
                seekBar = popupView.findViewById(R.id.seekBarAm);
                seekBar.setProgress((int)(effectInfo.emboss.ambient * 10));
                seekBar = popupView.findViewById(R.id.seekBarCo);
                seekBar.setProgress((int)(effectInfo.emboss.specular));
                seekBar = popupView.findViewById(R.id.seekBarBl);
                seekBar.setProgress((int)(effectInfo.emboss.blurRadius));
                popupWindow.showAsDropDown(v);
            }
        };
        bt = findViewById(R.id.btEmboss);
        bt.setOnClickListener(onClickListenerForEmboss);

    }

    private void initOptions() {

        Switch bt = findViewById(R.id.shadow);
        bt.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                effectInfo.shadow.used = isChecked;
                applyTextEffect(0);
            }
        } );

        bt = findViewById(R.id.glow);
        bt.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                effectInfo.glow.used = isChecked;
                applyTextEffect(1);
            }
        } );

        bt = findViewById(R.id.outline);
        bt.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                effectInfo.outline.used = isChecked;
                applyTextEffect(2);
            }
        } );

        bt = findViewById(R.id.emboss);
        bt.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                effectInfo.emboss.used = isChecked;
                applyTextEffect(3);
            }
        } );
    }

    int touchState = 0;
    float centerX = 0;;
    float centerY = 0;
    Matrix baseMatrix;
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getPointerCount() == 2) {
            int action = event.getAction();
            int pureaction = action & MotionEvent.ACTION_MASK;
            if (pureaction == MotionEvent.ACTION_POINTER_DOWN) {
                mBaseDist = getDistance(event);
                mBaseRatio = mRatio;
            } else {
                float delta = (getDistance(event) - mBaseDist) / STEP;
                float multi = (float) Math.pow(2, delta);
                mRatio = Math.min(1024.0f, Math.max(0.1f, mBaseRatio * multi));

                //textView.setTextSize(mRatio + 13);


                float scale = mRatio / 15.0f;
                Matrix matrix1 = new Matrix() ;
                matrix1.postScale(scale, scale);
                imageView.setImageMatrix(matrix1) ;
            }
            touchState = 0;
        } else if(event.getPointerCount() == 1) {
            if(event.getActionMasked() == MotionEvent.ACTION_DOWN) {
                touchState = 1;
                centerX = event.getX();
                centerY = event.getY();
                baseMatrix= imageView.getImageMatrix();
            } else if(event.getActionMasked() == MotionEvent.ACTION_MOVE) {
                if (touchState == 1) { //DRAG
                    float scale = mRatio / 15.0f;

                    float x_cord = event.getX();
                    float y_cord = event.getY();

                    float[] preValue = new float[9];
                    baseMatrix.getValues(preValue);
                    Matrix matrix1 = new Matrix() ;
                    //float move_x = preValue[Matrix.MTRANS_X] / scale  + (x_cord - centerX);
                    //float move_y = preValue[Matrix.MTRANS_Y] / scale+ (y_cord - centerY);

                    float move_x = (x_cord - centerX);
                    float move_y = (y_cord - centerY);

                    matrix1.postScale(scale, scale);
                    matrix1.postTranslate(move_x, move_y);
                    imageView.setImageMatrix(matrix1) ;
                }
            } else {
                touchState = 0;
            }
        }
        return true;
    }

    int getDistance(MotionEvent event) {
        int dx = (int) (event.getX(0) - event.getX(1));
        int dy = (int) (event.getY(0) - event.getY(1));
        return (int) (Math.sqrt(dx * dx + dy * dy));
    }

    private void setDefaultValue() {

        effectInfo.textColor = Color.WHITE;
        effectInfo.textSize = 50;

        effectInfo.shadow.color = 0xFF0088ff;
        effectInfo.shadow.radius = 10;
        effectInfo.shadow.dx = 12;
        effectInfo.shadow.dy = 12;

        effectInfo.outline.color = 0xFFD81B60;
        effectInfo.outline.strokeWidth = 2;

        effectInfo.glow.color = 0xFFF9D11E;
        effectInfo.glow.radius = 24;
        effectInfo.glow.glowType = GlowType.innerGlow;

        effectInfo.emboss.direction = new float[] { 0f, 1f, 0.5f };
        effectInfo.emboss.ambient = 0.3f;
        effectInfo.emboss.specular = 3f;
        effectInfo.emboss.blurRadius = 3f;
    }


    /*
    @param index 0-shadow 1-glow  2-outline 3-emboss 4-all
     */
    private void applyTextEffect(int index) {
        switch (index) {
            case 0: {//shadow
                textImage.clearOuterShadows();
                if(effectInfo.shadow.used)
                    textImage.addOuterShadow(effectInfo.shadow.radius, effectInfo.shadow.dx, effectInfo.shadow.dy, effectInfo.shadow.color);
                break;
            }
            case 1: {//glow
                textImage.clearOuterGlows();
                textImage.clearInnerGlows();
                if(effectInfo.glow.used) {
                    if(effectInfo.glow.glowType == GlowType.outerGlow)
                        textImage.addOuterGlow(effectInfo.glow.radius, effectInfo.glow.dx, effectInfo.glow.dy, effectInfo.glow.color);
                    else if(effectInfo.glow.glowType == GlowType.innerGlow)
                        textImage.addInnerGlow(effectInfo.glow.radius, effectInfo.glow.color);
                    else {
                        textImage.addOuterGlow(effectInfo.glow.radius, effectInfo.glow.dx, effectInfo.glow.dy, effectInfo.glow.color);
                        textImage.addInnerGlow(effectInfo.glow.radius, effectInfo.glow.color);
                    }
                }
                break;
            }
            case 2: {//outline
                if(effectInfo.outline.used)
                    textImage.setStroke(effectInfo.outline.strokeWidth, effectInfo.outline.color);
                else
                    textImage.clearStroke();
                break;
            }
            case 3: {//emboss
                textImage.clearEmbosses();
                if(effectInfo.emboss.used)
                    textImage.addEmboss(effectInfo.emboss.direction, effectInfo.emboss.ambient, effectInfo.emboss.specular, effectInfo.emboss.blurRadius);

                break;
            }
        }
        updateImageView();
    }

    private void updateImageView() {
        Bitmap bm = textImage.makeTextBitmap();
        if(bm != null)
            imageView.setImageBitmap(bm);
    }
}
