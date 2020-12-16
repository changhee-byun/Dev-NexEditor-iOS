
# NexEditor Engine for KineMaster Android

## v.2.12.41 based on v2.12.39
SDK Version : 2.12.41
GIT Tag : ios-r2.12.41
Q-TAG:
ReleaseFileName : NexEditor_v2.12.41_for_Kinemaster
Fixed issue :
1. Fixed "KIOS-2202"
   (특정 MP3 파일에서 ID3 tag가 뒤에 있을경우 MP3 Sync word를 찾지 못하는 증상 해결)


## v.2.12.39 based on v2.12.38
SDK Version : 2.12.39
GIT Tag :
Q-TAG:
ReleaseFileName : NexEditor_v2.12.39_for_Kinemaster
Fixed issue :
For Android
1. added nexTextEffect.java for Android

## v.2.12.38 based on v2.12.37
SDK Version : 2.12.38
GIT Tag : ios-r2.12.38
Q-TAG:
ReleaseFileName : NexEditor_v2.12.38_for_Kinemaster

Fixed issue :
For IOS
1. Fixed KIOS-2191

## v.2.12.37 based on v2.12.33
SDK Version : 2.12.37
GIT Tag : ios-r2.12.37
Q-TAG:
ReleaseFileName : NexEditor_v2.12.37_for_Kinemaster

Fixed issue :
For IOS
1. getAvailabilityOfHardwareResourceWithIdentifier()를 통해 얻은 info로 decoder max count & memory size 설정

For Android
1. Fixed KM-6379
   NexEditorUtils.java 파일이 변경 되었습니다.
   아래 commit 참조 부탁 드립니다.

   커밋: c78d260fa4741c017683950a6d3477414d6b61ad [c78d260]
   상위 항목: a86d6d860e
   작성자: kyudae.jeong <kyudae.jeong@nexstreaming.com>
   날짜: 2019년 5월 22일 수요일 오후 5:06:51
   커밋한 사람: kyudae.jeong
   Fixed KM-6379


## v.2.12.33 based on v2.12.31
SDK Version : 2.12.33
GIT Tag : ios-r2.12.33
Q-TAG:
ReleaseFileName : NexEditor_v2.12.33_for_Kinemaster

Fixed issue :
For IOS
1. NexEditor.h 에 property 추가 (kDecoderMaxCount, kDecoderMaxMemorySize)


## v.2.12.31 based on v2.12.29
SDK Version : 2.12.31
GIT Tag : ios-r2.12.31
Q-TAG:
ReleaseFileName : NexEditor_v2.12.31_for_Kinemaster

Fixed issue :
1. Fixed KIOS-2159
    -> KM-6350 와 비슷한 이슈(clip 100ms 추가)로 꼭 적용 부탁 드립니다.

For IOS
1. getAvailabilityOfHardwareResourceWithIdentifier API 추가
2. checkAvailabilityOfHardwareResourceForIdentifier API 이름 변경
    -> checkAvailabilityOfHardwareResourceWithIdentifier


## v.2.12.29 based on v2.12.27
SDK Version : 2.12.29
GIT Tag :
Q-TAG:
ReleaseFileName : NexEditor_v2.12.29_for_Kinemaster

Fixed issue :
1. getKenBurnsRect() 링크 되지 않아 crash 나는 현상
2. getKenBurnsRect 이름 변경 -> getKenBurnsRects


## v.2.12.27 based on v2.12.25
SDK Version : 2.12.27
GIT Tag : ios-r2.12.27
Q-TAG:
ReleaseFileName : NexEditor_v2.12.27_for_Kinemaster

Fixed issue :
1. Fixed KM-6333 (mp3 VBR 인 경우 발생, duration 수정 하여 seek 재생 시 문제되는 부분 수정)  
2. Fixed KM-6335 (mp3 VBR 인 경우 발생, duration 수정) 


## v.2.12.25 based on v2.12.23
SDK Version : 2.12.25
GIT Tag : ios-r2.12.25
Q-TAG:
ReleaseFileName : NexEditor_v2.12.25_for_Kinemaster

Fixed issue :
1. KM-6350 (공통)
    비디오 decoder 여는 순서 변경
   (main video에서 열어야 할 videos의 decoder init 후 video layer의 decoder init.-> clip의 video start time 순서로 video decoder init)

Android Kinemaster만 해당
1. getKenBurnsRect함수 위치 변경
   위치 : NexEditor.java, NexEditor_jni.cpp -> NexEditorUtils.java NexEditorUtils_jni.cpp

   해당 git은 아래와 같습니다.
   커밋: 9cf35ec85c25815d829f3890559b474e6eaf2c05 [9cf35ec]
   상위 항목: f4a05971f7
   작성자: kyudae.jeong <kyudae.jeong@nexstreaming.com>
   날짜: 2019년 5월 15일 수요일 오후 4:40:06
   커밋한 사람: kyudae.jeong
   커밋 날짜: 2019년 5월 15일 수요일 오후 5:01:44
   nexEditorUtils.java 위치 변경(Android)

2. getKenBurnsRect함수에 faceRects 가 NULL일 경우, random 하게 처리하도력 변경


## v.2.12.23 based on v2.12.19
SDK Version : 2.12.23
GIT Tag : ios-r2.12.23
Q-TAG:
ReleaseFileName : NexEditor_v2.12.23_for_Kinemaster

Fixed issue :
1. getKenBurnsRectsInSize 함수에 faceRects 가 NULL일 경우, random 하게 처리하도력 변경


## v.2.12.19 based on v2.12.15
SDK Version : 2.12.19
GIT Tag : ios-r2.12.19
Q-TAG:
ReleaseFileName : NexEditor_v2.12.19_for_Kinemaster

Fixed issue :
1. 특정 함수 2개 위치 및 이름 변경
    위치 : NexEditor.h -> NexEditorUtil.h  
    이름 : getKenBurnsRect -> getKenBurnsRectsInSize
    이름 :  hardwareResourceAvailableForIdentifier   -> checkAvailabilityOfHardwareResourceForIdentifier

2. 입력 Rectangle 및 결과물 ratio가 변화는 문제


## v.2.12.15 based on v2.12.13
SDK Version : 2.12.15
GIT Tag : ios-r2.12.15
Q-TAG:
ReleaseFileName : NexEditor_v2.12.15_for_Kinemaster

Fixed issue :
1. getKenBurnsRect API 위치 변경 -> NexEditorUtil.h

(IOS) + (KenburnsRects) getKenBurnsRect:(CGSize)resolution projectRatio:(CGSize)ratio duration:(int)duration faceRects:(NSArray <NSValue*>*)faceRects;
(Android) JNIEXPORT jint JNICALL _EDITOR_JNI_(getKenBurnsRect)(JNIEnv *env, jobject obj,jint width,jint height, jobjectArray faceRects, jint aspectWidth, jint aspectHeight, jint duration , jobjectArray resultRects);

in -> (CGSize)resolution : 해당 clip의 image 크기
      projectRatio:(CGSize)ratio : ratio with, height
      duration:(int)duration : 해당 clip의 duration
      faceRects:(NSArray <NSValue*>*)faceRects : 얼굴 인식이 된 영역들
out -> start & end position


## v.2.12.13 based on v2.12.5
SDK Version : 2.12.13
GIT Tag : ios-r2.12.13
Q-TAG:
ReleaseFileName : NexEditor_v2.12.13_for_Kinemaster

Fixed issue :
1. getKenBurnsRect API 추가   
2. Audio Equalizer 추가
3. KM-6037
4. KM-6289
5. KIOS-2041
6. KIOS-2044
6. KIOS-2109

 
1. getKenBurnsRect API 추가
(IOS) - (KenburnsRects) getKenBurnsRect:(CGSize)resolution projectRatio:(CGSize)ratio duration:(int)duration faceRects:(NSArray <NSValue*>*)faceRects;
(Android) JNIEXPORT jint JNICALL _EDITOR_JNI_(getKenBurnsRect)(JNIEnv *env, jobject obj,jint width,jint height, jobjectArray faceRects, jint aspectWidth, jint aspectHeight, jint duration , jobjectArray resultRects);

IOS
in -> (CGSize)resolution : 해당 clip의 image 크기
      projectRatio:(CGSize)ratio : ratio with, height
      duration:(int)duration : 해당 clip의 duration
      faceRects:(NSArray <NSValue*>*)faceRects : 얼굴 인식이 된 영역들
out -> start & end position

Android  
in ->   width, height,   : 해당 clip의 image 크기
      faceRects: 얼굴 인식이 된 영역들  
       aspectWidth,  aspectHeight   : ratio with, height
      duration: 해당 clip의 duration
out -> resultRects : start & end position

해당 커밋은 아래와 같습니다.
커밋: 3abd0c5c36ea8f0ec92ac47a8236c8eafe94889c [3abd0c5]
상위 항목: b3ca8be2f0
작성자: kyudae.jeong <kyudae.jeong@nexstreaming.com>
날짜: 2019년 5월 2일 목요일 오전 11:25:35
커밋한 사람: kyudae.jeong
getKenBurnsRect API 추가


2. Audio Equalizer
해당 기능 관련해서 clip class에 "mEqualizer" 변수가 추가 되었습니다.

해당 커밋
커밋: f3da340ea5ce5d7b8d4f4cd36f12140bac315d32 [f3da340]
상위 항목: 51c325dc3e
작성자: kyudae.jeong <kyudae.jeong@nexstreaming.com>
날짜: 2019년 4월 18일 목요일 오후 1:25:29
커밋한 사람: kyudae.jeong
커밋 날짜: 2019년 4월 30일 화요일 오후 5:35:26
Audio Equalizer (SDK 부분) 추가



## v.2.12.5 based on v2.10.15
SDK Version : 2.12.5
GIT Tag : ios-r2.12.5
Q-TAG:
ReleaseFileName : NexEditor_v2.12.5_for_Kinemaster

Fixed issue :
1. LayerRenderer.java의 렌더링코드 sdk에서 렌더처리 하도록 native화함
2. 3d overlay resolution fix적용하여 스티커 선명하게하는 기능 적용(Android만)
3. external texture 기능은 텍스쳐 컨버팅 관련최적화 적용(Android만)
4. KM-6037(Android만)

Android
1. 이번버전부터 64bit so files 가 추가 되었습니다.
2. 위 수정 사항 1번과 관련해서 interface가 바뀌었습니다.
    첨부되어 있는 nexvideoeditor 폴더 참조 하시면 됩니다.
    참고로 Alex가 생성한 feature/nativerenderer 브랜치에 interface가 변경되어 적용 되어 있습니다.
    해당 브랜치 참고 하시면 더 도움이 될 거 같습니다.

## v.2.10.15 based on v2.10.9
SDK Version : 2.10.15
GIT Tag : ios-r2.10.15
Q-TAG: v2.10.8_GeneralQ
ReleaseFileName : NexEditor_v2.10.15_for_Kinemaster

Jira issue :
1. KIOS-2059

## v.2.10.13 based on v2.10.9
SDK Version : 2.10.13
GIT Tag : ios-r2.10.13
Q-TAG: v2.10.8_GeneralQ
ReleaseFileName : NexEditor_v2.10.13_for_Kinemaster

Jira issue :
1. KIOS-2041, Fix crash caused by setting new callback while executing (added by jake you)
2. KIOS-2044

3. Add the readonly attribute to the 'layer' property in NexEditor (added by mj.kong)

## v.2.10.9 based on v2.10.8
SDK Version : 2.10.9
GIT Tag : 
Q-TAG: v2.10.8_GeneralQ
ReleaseFileName : NexEditor_v2.10.9_for_Kinemaster

Jira issue :
KM-6226

## v.2.10.8 based on v2.10.6
SDK Version : 2.10.8
GIT Tag : 
Q-TAG: v2.10.8_GeneralQ
ReleaseFileName : NexEditor_v2.10.9_for_Kinemaster

Fixed issue :

1. IOS: (Testing_v2.10.0_General_QA 브랜치의 Tag:ios-r2.10.8)

2. NESA-1429 : FlAC, PCM  코덱 재생되지 않는 현상 수정 (kinemaster v2.8.5 부터 발생)



## v.2.10.6 based on v2.10.4
SDK Version : 2.10.6
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.10.6_for_Kinemaster

Fixed issue :
1. audio 파일 trim 된 상태로 loop 될때 재생 안 되는 이슈 수정(From LGE)
2. Fixed KIOS-1947 (track 에 대한 video cache codec count 2로 고정)
## v.2.10.4 based on v2.10.2

SDK Version : 2.10.4
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.10.4_for_Kinemaster

Fixed issue :

1. KIOS-1943
2. KIOS-1944
 

## v.2.10.2 based on v2.9.2

SDK Version : 2.10.2
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.10.2_for_Kinemaster

Fixed issue :

1. KM-6216   
2. NESA-1427
3. NESA-14278 
4. KIOS-1927,1933 (audio clip 갯수 무제한으로 사용 가능)


## v.2.9.2 based on v2.8.5

SDK Version : 2.9.2
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.9.2_for_Kinemaster

Fixed issue :

1.   IOS Codec Momory Size 체크 루틴 적용
2.   EditBox 컨텐츠 지원
3.   KM-6032
4.   KM-6040
5.   KM-6055
6.   KM-6089
7.   KM-6088
 

## v.2.8.5 based on v2.8.3

SDK Version : 2.8.5
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.8.5_for_Kinemaster

Fixed issue :

The symbol names are hidden from so file.

NexPKGVerifier is upgrade to 0.4.4. The symbols are hidden and the limit file size is changed from 4MB to 3MB.
Changed patch number in version information. The odd number is release version and the even number is debug version.


## v.2.8.3 based on v2.8.2

SDK Version : 2.8.3
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.8.3_for_Kinemaster

Jira issue :

Fixed memory leak in NexPKGVerifier(0.4.2).



## v.2.8.2 based on v2.8.1

SDK Version : 2.8.2
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.8.2_for_Kinemaster

Jira issue :

KM-6188, KM-6193



## v.2.8.1 based on v2.6.3

SDK Version : 2.8.1
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.8.1_for_Kinemaster

Jira issue :

KM-6088

KM-6155
DES-673
DES-614
KIOS-1831
KIOS-1839 
KIOS (다른 단말에서 생성된 project 를 가져왔을 때, project 생성하지 않고 getClipInfo_Sync() 호출 할때 fail되는 경우 수정)
NexPKGVerifier 0.4.0 적용.

## v.2.6.3 based on v2.6.1

SDK Version : 2.6.3
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.6.3_for_Kinemaster

Jira issue :

KM-6120



## v.2.6.1 based on v2.2.5.3

SDK Version : 2.6.1
GIT Tag : 
Q-TAG: NO
ReleaseFileName : NexEditor_v2.6.1_for_Kinemaster

Jira issue :

KM-6089

- kinemaser 설치 후 dci 실행하지 않을 경우, 지원가능한 해상도가 0으로 나오게 된 현상 수정



## v.2.2.5.4 based on v2.2.5.3

SDK Version : 2.2.5.4
GIT Tag :
Q-TAG: 
ReleaseFileName : NexEditor_v2.2.5.4_for_Kinemaster

Jira issue :

KM-5076

KM-6032

KM-6040

Releated to KM-6044

KM-6055



## v.2.2.5.3 based on v2.2.5.0

SDK Version : 2.2.5.3
GIT Tag :
Q-TAG: 
ReleaseFileName : NexEditor_v2.2.5.3_for_Kinemaster

Jira issue :

KM-5275
KM-5475
KM-5241
KM-6024
## v.2.2.5.0 based on v2.2.4.0_General_Q

SDK Version : 2.2.5.0
GIT Tag :
Q-TAG: 
ReleaseFileName : NexEditor_v2.2.5.0_for_Kinemaster

Jira issue :

KM-3810
KM-6008
NESA-1422

기능 추가(아래 링크 참조):

Speed Control 적용시 음정 변화 유무 적용
Guide For NexEngine


## v.2.2.4.0_General_Q(., 2018) based on v2.2.3.0_General_Q

SDK Version : 2.2.4.1
GIT Tag :v2.2.4.0_GeneralQ
Q-TAG: YES.
ReleaseFileName : NexEditor_v2.2.4.1_for_Kinemaster

 

Jira issue :

KM-5682
KM-5785
KM-5709
KM-5603
DES-445

기능 추가(아래 링크 참조):

I-FramePlayMode
Experimental mode 
         Guide For NexEngine


기타 수정사항:

fix miss-matching function on RI - related with supporting array input
support reverse, and cycling on Renderitem
fix for text issue
          cause)effect not reset when effect id is changed if effect option is equal.
          resolution)effect is reset when effect id is changed.

support current video customlut-fastpreview
fix duplicated shader loading by adding flag on theme context
fix thread issue related with effectitemvec


릴리즈 위치:

\\EditorPC\Editor\NexEditor_for_KineMaster

 

## v.2.0.13_General_Q(., 2018) based on v2.0.12_General_Q

1.Add DenyFeaturedlist option. 
2.Add routine of asset encrypt/decrypt.
3.Support Template3.0(SDK Level 7) 
4.Add freeze duration feature for template 2.0.
5.Support 0.2x speed play for template 2.0.
6.Fixed bugs(error when seek over 4gb file) 

 

## v.2.0.12_General_Q(., 2018) based on v2.0.10_General_Q

1. Add ratio-based autotrim. 
2. Add Facedect Interface (supported setting rect) 
3. Add bit_template function (change to sdk level 6) 
4. Fixed bugs

Note: 1, 2, and 3 are not official functions.

 

## v.2.0.10_General_Q(., 2018) based on v2.0.8_General_Q

1.Improved direct export performance
2. Added Vignette, sharpness effect
3. Fixed duplicate play problem in collage bgm
4. Fixed bugs



## v.2.0.8_General_Q(., 2018) based on v2.0.7_General_Q

1.Improved stability of SDK and collage
2.Fixed bugs

 

## v.2.0.7_General_Q(., 2017) based on v2.0.5_General_Q

1.GIF 기능 추가 

2. Collage 관련 bug fix 

3. HDR Preview 지원 (예외 사항: HDR 지원 단말에서만)

4. Gif , Facedectection 를 external module 화. 

 

(in English)

Supported Animated GIF export option.
Supported HDR video content for new chipsets enabling HDR video playback.
Changed Animated GIF encoder and Face Detector as external modules for plug-in architecture.
Fixed bugs. 

## v.2.0.5_General_Q(., 2017) based on v2.0.4_General_Q
1. 꼴랴쥬 버그 수정 및 성능 개선.
- 콜라쥬에 Face dector 1차 적용
2. face detect 이후에 화면을 이전보다 천천히 움직이게 조정. 
3. NESA-1204 - missing control value setting on vr360 contents 
4. springwedding template play 시에 intro 호면이 정지 되어야 하는데 움직이는 문제 수정. 
5. 워터 마크 교체 
6. 기타 엔진 버그 수정.

 

(in English)

Supported face detection when photos are imported to static/dynamic collage assets.
Adjusted Ken Burns effect's start/end positions to move them slowly.
Changed NexEditor SDK watermark logo with new one.
Fixed the bug that unintentionally Ken Burns effect is applied to template's intro image.
Fixed bugs.
 

## v.2.0.4_General_Q(., 2017) based on v2.0.3_General_Q

1. Direct Export시 video only content도 지원.
2. Fixed a little memory leak issue.
3. Add Motion tracked Video support
4. intro clip 이 있는 특정 템프릿 (modernwhite) intro 화면이 정상적으로 preview 안되는 문제 수정.

 

(in English)

Supported Direct Export for video only import files.
Supported Motion-Tracked Video for Dynamic Collage Assets.
Fixed memory leak issue. 
Fixed bugs.


## v.2.0.3_General_Q(., 2017) based on v2.0.2_General_Q

1. Face Detector FIT Mode 제거.
2. 1/16, 1/32 배속 추가 (120fps 이상 컨텐츠에서만 동작)
3. Fixed CHC-151 The SO file can't load success after refactor
4. 3:2 비율로 export 시에 title UI 오른쪽으로 치우치는 버그 수정.
5. Fixed NESA-1111 실시간으로 Brightness , Saturation , Contrast 조절 기능 추가.

 

(in English)

Added APIs to adjust brightness/contrast/saturation of template video. Refer Template20TestActivity.java. 
Fixed bugs. 


## v.2.0.2_General_Q(., 2017) based on v2.0.1_General_Q

1. Fixed abnormal transition operation on the "Snapdragon 660".
2. CHC-148. Add APIs to adjust brightness/contrast/saturation of template video.
3. Improved export function.
4. When face detection is fail, apply blur effect to letter box.
5. Fixed NESI-431, NESI-438. => Fixed the exported file is shorter than preview.
6. Fixed KM-5106. => Fixed abnormal result when apply audio filter to "voice recording".
7. Fixed wrong flag setting issue on "Direct Export Task".
8. Optimization facedetection performance.
9. Added exception code for face detection.
10. Fixed timechecker bug. And added property to enable or disable the time checker.
11. The collison between updateProject() and seek() by adding "clearProject()" was removed.
12. Fixed the image is inverted issue during processing "nexEngine.captureCurrentFrame()".
13. Supported 1/16 audio speed control.
14. Improved collage feature.
15. Fixed NESA-1116. => It is processed by zoom or fit mode when face detection is fail.
16. Fixed NESA-1121. => Fixed the end of video is abnomal issue during directExport() of some content.



## v.2.0.1_General_QA(., 2017) based on v2.0.0_General_Q  

1. Face Detector 사용시에 aspect ratio 에 따라 letter box 유무 조절. (Template20TestAcitivity 참고 Engine.setLetterBox(boolean))

2. Google Framework FD에서 bitmap을 생성하지 못할때 예외처리 추가

3. Seek중에도 CaptureFrame이 정상적으로 동작하도록 수정

4. SW aac encoder 추가. 

5. Custom LUT 추가(가이드 문서 :https://docs.google.com/document/d/1echnbr2H8E81xp013xAZCVRJb8zFsi0KRi_7SePbVDQ/edit?ts=59a4ffbf)

6. NESA-1083 : [Audio BGM] Flac 5s Trim & Loop On Play시 Trim 구간 이전/이후 지점 출력되는 현상

7. NESA-1101 : [Multi Trim] 2개의 Trim 구간에 Speed 설정하여 Direct시 비정상 동작

8. NESA-1084 : 특정 Transition effect 적용 후 Play시 Transition Err

9. CHC-143 : [ZTE]The screen is show small size in after remove some clip and play in first time.

10. KM-5173 : Crash when launching KineMaster



 

(in English)

Added an option whether to allow letter/blur boxes or not when a template is applied to photos with face detection - setLetterBox(boolean).
Added SW AAC encoder.
Supported custom LUT - nexColorEffect.addCustomLUT(). (refer to NexStreaming’s Custom LUT APIs.pdf for the details)
Fixed an issue that CaptureFrame fails if it's called during seek.
Fixed bugs.
 

## v.2.0.0_General_QA(., 2017) based on v1.7.58_General_Q
1. Template 3.0 구현중
2. Collage 구현중
3. Face Detection 기능 추가
4. Overlapped Transition On/Off 기능 추가
5. NESA-999 : [Template 20] 10s Content Snow Template 적용 시 Effect 상이한 현상
6. NESA-998 : [Nexus5][Multi Trim] Trim 설정 후 Export 안됨
7. NESA-1008 : [Template 20] Transition On/Off 중에 Crash 발생 (일회성)
8. NESA-1002 : [Template 20] Fit되는 Image Clip Template Clear 하여 Play 및 Exported Play시 동작 상이
9. NESA-1011 : [Template 20] Transition OFF 시 임의 Template Play 중 3dcouple 보여지는 현상 (2회)
10. NESA-1005 : [Template 20] Transition OFF 시 Soft Wipe 에서 black transition 발생
11. Fix memory leak and add null check
12. Preloaded asset의 BGM file을 FD(File Descriptor)로 접근하도록 수정.
13. Wbmp format file 미지원 리스트로 추가
14. updateProject() 성능 개선
15. 60fps ~240fps 컨텐츠의 재생을 위한 개선작업. (task sleep 제거, reference만 디코딩)
16. NESA-1013 : [AutoTrimSpeedCheck] Interval 1s 이하 Autotrimnexport 시 export failed
17. NESA-1017 : [Template 20] 임의의 template play중에 다른 template 선택 반복시 crash 발생.
18. Modify KineMaster launching intent.
19. CHC-95 : [Vivo]Encoding Done isError=true result=48 but the storage still has free space
20. 비디오 균등하게 display 및 skip되지 않는 문제 수정(LGE)
21. NESA-1010 : [Template 20] Symmetry template 설정시 black screen 발생.
22. CHC-99 : [Vivo]The App is blocking sometimes.
23. CHC-100 : [Tinno]The App is ANR after apply new installed template
24. NESA-1027 : [Overlay User Image] Expression 미적용
25. Harmony 670
26. directExport, forceMixExport 시 진행률이 비정상으로 올라가는 문제 해결
27. NESA-1030 : [Template 20] 10 Image clips Snow 적용 후 Audio Sampling Rate 8000로 Export 시 느려지는 현상
28. NESA-1039 : [Thumbnail] Angle Rotate 후 Seek Tables 선택시 Thumbnail results Err
29. fix effect preview - side-effect of template30 implementation
30. fixed preview thumb
31. audio bgm과 효과음이 ID 가 겹치는 문제 수정
32. cachedSeek() 예외처리 추가 및 fail시 이벤트 발생하도록 수정
33. NESA-1045 : [Project Edit] Clip Effect 적용 후 Rotate시 Black Box 노출
34. NESA-1049 : [Template20] MPEG4V Video Codec Export불가
35. NESA-1048 : [Template 20] H.265로 Export 시 Crash
36. NESA-1059 : [Template20] 일부 Template letterbox UI Err
37. Audio Sync시에 EndTrimTime을 세팅하도록 함.
38. KM-5162 : Samsung Galaxy J1, Android 4.4.4 - App Crashes after playing video in editing mode
39. NESA-1065 : [Project Edit] Clip Effect 에서 TEXT or BG Color변경 시 미적용됨
40. CHC-136 : [XiaoMI App]DirectExport is fail when add filter and BGM
41. CHC-134 : [Xiaomi App][Android] directExport output duration is shorter than setting.
42. NESA-1073 : [Template 20] Overlap Off 후 Tropical Island Play시 Crash
43. NESA-1074 : [Nexus 6P][Template20] VASSET에서 ASSET Download 중 H/W Back key 누른 후 Crash 발생(1회성)
44. NESA-1079 : [Template 20] Overlap Off 후 Soft Wipe Play시 Crash
45. NESA-1081 : [Engine View] Direct 실패 후 EngineView진입 시 Crash됨
46. CHC-61 : [Tinno]MSM8909 can't initialize codec. Nexus 5에서 이슈 발생하여 다시 수정.

(in English)

Added face detection option for images of template inputs. (please refer Template20TestActivity.java and FaceDetectorActivity.java)
Added overlapped Transition On / Off option for smooth template preview at low performance devices.
Improved smooth skipping at preview for heavy video content input.
Improved stability.
Fixed bugs.
