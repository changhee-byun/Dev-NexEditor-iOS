/******************************************************************************
 * File Name   : ImageUtil.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "ImageUtil.h"
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <CoreText/CoreText.h>
#import "UIImage+Resize.h"
#import <NexEditorEngine/NexEditorConfiguration.h>
#import <NexEditorEngine/NexEditorLog.h>

#define NUMBER_OF_COMPONENTS_PER_PIXEL 4
#define BITS_PER_PIXEL 8

@implementation ImageUtil

+ (ImageUtil *)shared {
    static ImageUtil *sharedInfoManager = nil;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        sharedInfoManager = [[self alloc] init];
    });
    return sharedInfoManager;
}

+ (UIImage*)imageOfValidTextureWithImage:(UIImage*) image {
    double width = CGImageGetWidth(image.CGImage);
    double height = CGImageGetHeight(image.CGImage);
    
    NexEditorConfiguration *editorConfiguration = [[NexEditorConfiguration alloc] init];
    CGFloat imageMaxSize = [[editorConfiguration valueOf:NexEditorConfigParamMaxClipImageSize] floatValue];
    [editorConfiguration release];
    
    double widthRate = width < imageMaxSize? 1.0 : imageMaxSize/width;
    double heightRate = height < imageMaxSize ? 1.0 : imageMaxSize/height;
    
    double rate = MIN(widthRate, heightRate);
    
    if(rate < 1.0) {
        image = [image resizing:CGSizeMake((int)(width*rate), (int)(height*rate))];
    }
    
    return image;
}

+ (UIImage *)imageWithColor:(UIColor *)color Size:(CGSize)size{
    CGRect rect = CGRectMake(0.0f, 0.0f, size.width, size.height);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, 0);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage *)circleImageWithColor:(UIColor *)color Size:(float)diameter {
    float scale = [UIScreen mainScreen].scale;
    CGRect rect = CGRectMake(0.0f, 0.0f, diameter, diameter);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    //draw bg
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextSetStrokeColorWithColor(context, [color CGColor]);
    CGContextFillEllipseInRect(context, CGRectInset(rect, 1, 1));
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage *)circleImageWithColor:(UIColor *)color Size:(float)width Diameter:(float)diameter Scale:(float)scale {
    CGRect rect = CGRectMake(0.0f, 0.0f, width, width);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    //draw bg
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextSetStrokeColorWithColor(context, [color CGColor]);
    CGContextFillEllipseInRect(context, CGRectMake((width-diameter)/2.0, (width-diameter)/2.0, diameter, diameter));
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage *)circleImageWithbgColor:(UIColor *)bgColor bgSize:(float)bgRadius color:(UIColor*)color Size:(float)radius {
    CGRect rect = CGRectMake(0.0f, 0.0f, bgRadius, bgRadius);
    UIGraphicsBeginImageContextWithOptions(rect.size, NO, 0);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    //draw bg
    CGContextSetFillColorWithColor(context, [bgColor CGColor]);
    CGContextSetStrokeColorWithColor(context, [bgColor CGColor]);
    CGContextFillEllipseInRect(context, CGRectInset(rect, 1, 1));
    
    //draw inner
    float margin = (bgRadius-radius)/2.0;
    CGRect innerRect = CGRectMake(margin, margin, radius, radius);
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextSetStrokeColorWithColor(context, [color CGColor]);
    CGContextFillEllipseInRect(context, innerRect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage*)imageWithColor:(UIColor *)color
{
    return [ImageUtil imageWithColor:color Size:CGSizeMake(2, 2)];
}

+ (UIImage *)dimImageWithSize:(CGSize)size emptyRect:(CGRect)rect Scale:(float)scale Angle:(float)angle {
    UIGraphicsBeginImageContextWithOptions(size, NO, 0);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    //draw bg
    CGContextSetFillColorWithColor(context, RGBA(0, 0, 0, 0.3).CGColor);
    CGContextFillRect(context, CGRectMake(0, 0, size.width, size.height));
    
    double tx = rect.origin.x + rect.size.width/2.0;
    double ty = rect.origin.y + rect.size.height/2.0;
    double radian = angle / 180.0 * M_PI;
    
    rect = CGRectMake(-rect.size.width/2.0, -rect.size.height/2.0, rect.size.width, rect.size.height);
    CGContextTranslateCTM(context, tx, ty);
    CGContextScaleCTM(context, scale, scale);
    CGContextRotateCTM(context, radian);
    CGContextClearRect(context, rect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage *)imageHueCircle:(float)radius Divisor:(int)divisor Thickness:(float)thicknessRate {
    float thickness = radius * thicknessRate;     //이미지의 두께
    float fDivisor = divisor;
    
    //Core graphic 사용
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(radius * 2, radius*2), NO, [[UIScreen mainScreen] scale]);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    float oneStepAngle = 2.0f * M_PI / fDivisor;     //한번에 그릴 각도. 값이 작아야 자연스럽게 그려진다.
    CGMutablePathRef path = CGPathCreateMutable();  //path 생성
    
    //한 조각 부분에 해당하는 path를 저장
    CGPathMoveToPoint(path, NULL, cos(-oneStepAngle /2.0f) * (radius - thickness), sin(-oneStepAngle/2.0f) * (radius - thickness));
    CGPathAddArc(path, NULL, 0.0f, 0.0f, radius - thickness, -oneStepAngle/2.0f, oneStepAngle/2.0f + 1.0e-2f, false);
    CGPathAddArc(path, NULL, 0.0f, 0.0f, radius, oneStepAngle/2.0f + 1.0e-2f, -oneStepAngle/2.0f, true);
    CGPathCloseSubpath(path);
    
    //중심점으로 이동
    CGContextTranslateCTM(context, radius, radius);
    CGContextRotateCTM(context, -M_PI_2);
    
    for ( int i=0; i <fDivisor; i++) {
        UIColor * color = [UIColor colorWithHue:i/fDivisor saturation:1 brightness:1 alpha:1];  //해당 각도에 해당하는 색상 생성
        
        CGContextAddPath(context, path);            //path 추가
        CGContextSetFillColorWithColor(context, color.CGColor); //색상 지정
        CGContextFillPath(context);                 //path에 색상 입히기
        CGContextRotateCTM(context, -oneStepAngle); //다음을 위해 회전
    }
    CGPathRelease(path);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+ (UIImage *)getWaveFormWithAsset:(AVURLAsset *)songAsset Size:(CGSize)size{
    NSError * error = nil;
    AVAssetReader * reader = [[AVAssetReader alloc] initWithAsset:songAsset error:&error];
    AVAssetTrack * songTrack = [songAsset.tracks objectAtIndex:0];
    
    NSDictionary* outputSettingsDict = [[NSDictionary alloc] initWithObjectsAndKeys:
                                        [NSNumber numberWithInt:kAudioFormatLinearPCM],AVFormatIDKey,
                                        [NSNumber numberWithInt:16],AVLinearPCMBitDepthKey,
                                        [NSNumber numberWithBool:NO],AVLinearPCMIsBigEndianKey,
                                        [NSNumber numberWithBool:NO],AVLinearPCMIsFloatKey,
                                        [NSNumber numberWithBool:NO],AVLinearPCMIsNonInterleaved,
                                        nil];
    
    AVAssetReaderTrackOutput* output = [[AVAssetReaderTrackOutput alloc] initWithTrack:songTrack outputSettings:outputSettingsDict];
    
    [reader addOutput:output];
    
    UInt32 sampleRate,channelCount;
    
    NSArray* formatDesc = songTrack.formatDescriptions;
    CMAudioFormatDescriptionRef item = (__bridge CMAudioFormatDescriptionRef)[formatDesc objectAtIndex:0];
    const AudioStreamBasicDescription* fmtDesc = CMAudioFormatDescriptionGetStreamBasicDescription (item);
    if(fmtDesc ) {
        sampleRate = fmtDesc->mSampleRate;
        channelCount = fmtDesc->mChannelsPerFrame;
    }
    else return nil;
    
    UInt32 bytesPerSample = 2 * channelCount;
    SInt16 normalizeMax = 0;
    
    NSMutableData * fullSongData = [[NSMutableData alloc] init];
    [reader startReading];
    
    UInt64 totalBytes = 0;
    SInt64 totalLeft = 0;
    SInt64 totalRight = 0;
    NSInteger sampleTally = 0;
    
    NSInteger samplesPerPixel = sampleRate / 100;
    
    while (reader.status == AVAssetReaderStatusReading){
        
        AVAssetReaderTrackOutput * trackOutput = (AVAssetReaderTrackOutput *)[reader.outputs objectAtIndex:0];
        CMSampleBufferRef sampleBufferRef = [trackOutput copyNextSampleBuffer];
        
        if (sampleBufferRef){
            CMBlockBufferRef blockBufferRef = CMSampleBufferGetDataBuffer(sampleBufferRef);
            
            size_t length = CMBlockBufferGetDataLength(blockBufferRef);
            totalBytes += length;
            
            NSMutableData * data = [NSMutableData dataWithLength:length];
            CMBlockBufferCopyDataBytes(blockBufferRef, 0, length, data.mutableBytes);
            
            SInt16 * samples = (SInt16 *) data.mutableBytes;
            int sampleCount = (int)(length / bytesPerSample);
            for (int i = 0; i < sampleCount ; i ++) {
                
                SInt16 left = *samples++;
                totalLeft  += left;
                
                SInt16 right;
                if (channelCount==2) {
                    right = *samples++;
                    totalRight += right;
                }
                
                sampleTally++;
                
                if (sampleTally > samplesPerPixel) {
                    
                    left  = totalLeft / sampleTally;
                    
                    SInt16 fix = abs(left);
                    if (fix > normalizeMax) {
                        normalizeMax = fix;
                    }
                    
                    [fullSongData appendBytes:&left length:sizeof(left)];
                    if(fullSongData.length == 24798) {
                        int a =0;
                        a = 9+a;
                    }
                    
                    if (channelCount==2) {
                        right = totalRight / sampleTally;
                        
                        SInt16 fix = abs(right);
                        if (fix > normalizeMax) {
                            normalizeMax = fix;
                        }
                        
                        [fullSongData appendBytes:&right length:sizeof(right)];
                        
                    }
                    
                    totalLeft   = 0;
                    totalRight  = 0;
                    sampleTally = 0;
                }
            }
            
            CMSampleBufferInvalidate(sampleBufferRef);
            CFRelease(sampleBufferRef);
        }
    }
    
    if (reader.status == AVAssetReaderStatusCompleted){
        UIImage *image = [self audioImageGraph:(SInt16 *)fullSongData.bytes
                                 normalizeMax:normalizeMax
                                  sampleCount:fullSongData.length / 2
                                 channelCount:channelCount
                                   imageWidth:size.width
                                  imageHeight:size.height];
        return image;
    }        
    
    return nil;
}

+ (UIImage *)audioImageGraph:(SInt16 *) samples
                normalizeMax:(SInt16) normalizeMax
                 sampleCount:(NSInteger) sampleCount
                channelCount:(NSInteger) channelCount
                  imageWidth:(float) imageWidth
                 imageHeight:(float) imageHeight {
    
    CGSize imageSize = CGSizeMake(imageWidth, imageHeight);
    UIGraphicsBeginImageContext(imageSize);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGColorRef waveColor = [[UIColor whiteColor] CGColor];
    CGContextSetLineWidth(context, 1.0);
    
//    BOOL isTwo = channelCount == 2;
//    if(!isTwo) channelCount = 1;
//    float halfGraphHeight = imageHeight / 2;
//    float sampleAdjustmentFactor = halfGraphHeight / (float) normalizeMax;
//    
////    int cnt = (int)sampleCount * 0.45; //원래는 1/2 하고 앞뒤 5% 제거
////    int padding = sampleCount * 0.05;
//    int cnt = sampleCount * 0.5;
//    for (int i=0; i<cnt; i++) {
//        float rate = (float)i / (float)(cnt - 1);
//        int x = imageWidth * rate;
//
//        SInt16 left = samples[i*channelCount];
//        SInt16 right = isTwo ? samples[i*channelCount+1] : 0;
//
//        float pixels = (float) MAX(left, right);
//        pixels *= sampleAdjustmentFactor;
//
//        CGContextMoveToPoint(context, x, halfGraphHeight-pixels);
//        CGContextAddLineToPoint(context, x, halfGraphHeight+pixels);
//        CGContextSetStrokeColorWithColor(context, waveColor);
//        CGContextStrokePath(context);
//    }

    //
    BOOL isTwo = channelCount == 2;
    if(!isTwo) channelCount = 1;
    float sampleAdjustmentFactor = imageHeight / (float) normalizeMax;

//    int cnt = (int)sampleCount * 0.45; //원래는 1/2 하고 앞뒤 5% 제거
//    int padding = sampleCount * 0.05;
    int cnt = sampleCount * 0.5;
    for (int i=0; i<=imageWidth; i++) {
        float rate = (float)i / imageWidth;

        int index = (int)(cnt*rate);
        SInt16 left = samples[index*channelCount];
        SInt16 right = isTwo ? samples[index*channelCount+1] : 0;

        float pixels = (float) MAX(abs(left), abs(right));
        pixels *= sampleAdjustmentFactor;

        CGContextMoveToPoint(context, i, imageHeight);
        CGContextAddLineToPoint(context, i, imageHeight - pixels);
        CGContextSetStrokeColorWithColor(context, waveColor);
        CGContextStrokePath(context);
    }

    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return newImage;
}

#pragma mark OpenGL methods
//==========================================================================================================================
//  OpenGL methods
//==========================================================================================================================

+(GLuint) glTextureIDfromName:(NSString*)name
{
    UIImage *image = [UIImage imageNamed:name];             //리소스로 등록된 이미지를 생성
    GLuint textureId = [self glTextureIDfromImage:image];   //텍스쳐 생성
    
    if (image) {
        image = nil;
    }
    
    return textureId;
}

+(GLuint) glTextureIDfromImage:(UIImage*)image
{
    CGImageRef imageRef = [image CGImage];
    int width = (int)CGImageGetWidth(imageRef);     //이미지 가로 크기
    int height = (int)CGImageGetHeight(imageRef);   //이미지 세로 크기
    
    //텍스쳐를 생성한다.
    return [self getTextureIDWithHeight:height width:width drawingBlock:^(CGContextRef context) {
        //CGContext에 이미지를 그려서 데이터를 채운다.
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    }];
}

+(GLuint) glTextureIDfromData:(GLubyte*)data Width:(int)w Height:(int)h {
    //데이터 버퍼를 OpenGL 텍스쳐로 생성한다.
    GLuint textureID;                               [self checkGL:@"create variable(texture id)"];
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);          [self checkGL:@"glPixelStorei"];
    glGenTextures(1, &textureID);                   [self checkGL:@"glGenTextures"];
    
    glActiveTexture(GL_TEXTURE0);                   [self checkGL:@"glActiveTexture"];
    glBindTexture(GL_TEXTURE_2D, textureID);        [self checkGL:@"glBindTexture"];
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       [self checkGL:@"GL_TEXTURE_MIN_FILTER"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);       [self checkGL:@"GL_TEXTURE_MAG_FILTER"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    [self checkGL:@"GL_TEXTURE_WRAP_S"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    [self checkGL:@"GL_TEXTURE_WRAP_T"];
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);  [self checkGL:@"glTexImage2D"];
    
    return textureID;
}

+(GLuint) glTextureIDfromColorRed:(float)red Green:(float)green Blue:(float)blue Alpha:(float)alpha {
    //단색이라서 크기가 클 필요가 없다.
    int width = 16;
    int height = 16;

    //텍스쳐를 생성한다.
    return [self getTextureIDWithHeight:height width:width drawingBlock:^(CGContextRef context)
            {
                //*왼쪽 아래가 0,0으로 된다.
                //사용자가 설정한 색상으로 데이터 채운다.
                CGContextSetRGBFillColor(context, red, green, blue, alpha);
                CGContextAddRect(context, CGRectMake(0, 0, width, height));
                CGContextFillPath(context);
            }];
}

+(void) deleteTexture:(GLuint) textureId {
    if (textureId == 0 || (textureId == UINT_MAX)) {
        NexLogW(@"ImageUtil", @"Deleting wrong texture name:%d", textureId);
        return;
    }
    glDeleteTextures(1, &textureId);
}

+(GLuint) getTextureIDWithHeight:(int)height width:(int)width drawingBlock:(void (^)(CGContextRef context))specificContent
{
    //*왼쪽 아래가 0,0으로 된다.
    
    //텍스쳐를 생성할 크기 만큼 데이터 버퍼를 만든다.
    size_t size = width * height * NUMBER_OF_COMPONENTS_PER_PIXEL;  // 4 components per pixel (RGBA)
    GLubyte* textureData = (GLubyte *)malloc(size);
    memset(textureData, 0, size);
    
    //데이터 버퍼로 CGContext를 만든다.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    NSUInteger bytesPerPixel = NUMBER_OF_COMPONENTS_PER_PIXEL;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = BITS_PER_PIXEL;
    CGContextRef context = CGBitmapContextCreate(textureData, width, height,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 (CGBitmapInfo)kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    
    
    //각 상황에 맞는 이미지를 드로잉한다. 이미지, 텍스트, 컬러
    specificContent(context);
    

    //자원 해제
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);

    //데이터 버퍼를 OpenGL 텍스쳐로 생성한다.
    GLuint textureID;                               [self checkGL:@"create variable(texture id)"];
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);          [self checkGL:@"glPixelStorei"];
    glGenTextures(1, &textureID);                   [self checkGL:@"glGenTextures"];
    
    glActiveTexture(GL_TEXTURE0);                   [self checkGL:@"glActiveTexture"];
    glBindTexture(GL_TEXTURE_2D, textureID);        [self checkGL:@"glBindTexture"];
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       [self checkGL:@"GL_TEXTURE_MIN_FILTER"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);       [self checkGL:@"GL_TEXTURE_MAG_FILTER"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);    [self checkGL:@"GL_TEXTURE_WRAP_S"];
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);    [self checkGL:@"GL_TEXTURE_WRAP_T"];
    
    //CIFilter* _coreFilter = [CIFilter filterWithName:@"CIColorInvert"];
    //[_coreFilter setDefaults];
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);  [self checkGL:@"glTexImage2D"];

    free(textureData);
    
    return textureID;
}

/**
 * Queries GL_MAX_TEXTURE_SIZE
 * \return 0.0 if failed.
 */
+(float) glMaxTextureSize
{
    static GLfloat maxTextureSize = 0.0;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        glGetFloatv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    });
    return (float)maxTextureSize;
}

+(void) checkGL:(NSString*)tag {
    GLint error = glGetError();
    if (error != GL_NO_ERROR) {
        NSLog(@"TAG : %@, GL ERROR : %x", tag, error);
    }
}

#pragma - Misc

/**
 * Returns MIN(size, max). diff is called if size.width > max.width or size.height > max.height.
 */
+ (CGSize) limitSize:(CGSize) size maxSize:(CGSize) max diff: ( void (^)(CGSize) ) diff
{
    CGSize diffSize = CGSizeZero;
    if ( size.width > max.width) {
        diffSize.width = size.width - max.width;
        size.width = max.width;
    }
    if (size.height > max.height) {
        diffSize.height = size.height - max.height;
        size.height = max.height;
    }
    
    if ( diff && !CGSizeEqualToSize(diffSize, CGSizeZero)) diff(diffSize);
    
    return size;
}

@end
