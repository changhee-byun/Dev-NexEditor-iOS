/******************************************************************************
 * File Name   : NexImageLoader.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#import <Photos/Photos.h>

#import "NexImage.h"
//#import "NexSAL_Internal.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "NXEEnginePrivate.h"
#import "EditorUtil.h"
#import "AssetLibraryProxy.h"
#import "ImageUtil.h"
#import "LUTMap.h"
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"
#import "BundleResource.h"
#import "NSString+Color.h"
#import "UIImage+Resize.h"
#import "ThemeResourceLoader.h"

#define LOG_TAG @"NexImage"

/// Maximum clip image zoom-in scale according to CropModePanRandom
#define MAX_ZOOMIN_SCALE            (4.0 / 3.0)
/// FIXME: Should be passed in from the editor
#define MAX_RENDER_RESOLUTION_SIZE  (1920.0)

@interface NexImageResource : NSObject

@property (nonatomic, assign) CGImageRef imageRef;
@property (nonatomic, assign) unsigned char* pImageData;

@end

@implementation NexImageResource

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.imageRef = nil;
        self.pImageData = nil;
        
        return self;
    }
    return nil;
}

@end

@interface NexImage ()

@property (nonatomic, retain) NSMutableArray<NexImageResource *> *imageList;
@property (nonatomic, retain) NSDictionary<NSString *, UIColor*> *solidColors;
@property (nonatomic, retain) NSMutableArray<NSNumber *> *lutListinPreview;
@property (nonatomic, retain) NSMutableArray<NSNumber *> *lutListinExport;
@property (nonatomic, assign) int width;
@property (nonatomic, assign) int height;
@property (nonatomic) CGSize maxImageSize;
@property (nonatomic) NexImageLoaderConfig config;
@end

@implementation NexImage {
    GLint v_exportTextureId;
    GLint v_previewTextureId;
    GLint c_exportTextureId;
    GLint c_previewTextureId;
    NSData *themeData;
}

- (CGImageRef)createImage4PhAssetPath:(NSString*)path
{

    __block CGImageRef imageRef = nil;
    
    PHFetchOptions *fetchOptions = [[PHFetchOptions alloc] init];
    fetchOptions.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"creationDate" ascending:YES]];

    NSString *filepath = [EditorUtil removeSubString:@"phasset-image://" from:path];
    PHFetchResult *fetchResult = [PHAsset fetchAssetsWithLocalIdentifiers:@[filepath] options:fetchOptions];
    PHAsset *asset = fetchResult.firstObject;
    
    PHImageRequestOptions * imageRequestOptions = [[PHImageRequestOptions alloc] init];
    imageRequestOptions.synchronous = YES;
    imageRequestOptions.resizeMode = PHImageRequestOptionsResizeModeExact;
    imageRequestOptions.deliveryMode = PHImageRequestOptionsDeliveryModeHighQualityFormat;
    
    CGSize maxSize = self.maxImageSize;
    CGSize targetSize = CGSizeMake(asset.pixelWidth, asset.pixelHeight);
    if (targetSize.width > maxSize.width || targetSize.height > maxSize.height) {
        targetSize = maxSize;
    }

    __block UIImage *image = nil;
    __block int state = 0;
    __block int test_width = 0;
    
    [[PHImageManager defaultManager] requestImageForAsset:asset
                                               targetSize:targetSize
                                              contentMode:PHImageContentModeAspectFit
                                                  options:imageRequestOptions
                                            resultHandler:^(UIImage * _Nullable result, NSDictionary * _Nullable info) {
        state = 1;
        image = result;
        //[image retain];
        imageRef = result.CGImage;
        CGImageRetain(imageRef);
        test_width = (int)CGImageGetWidth(imageRef) & 0xFFFFFFFE;
    }];

    state = 2;
    [fetchOptions release];
    [imageRequestOptions release];
    
    if ( imageRef != NULL ) {
        self.width = (int)CGImageGetWidth(imageRef) & 0xFFFFFFFE;
        self.height = (int)CGImageGetHeight(imageRef) & 0xFFFFFFFE;
    }
    
    if ( self.width == 0 )
        return NULL;
    else
        return imageRef;
}

- (CGImageRef)createImage4Path:(NSString *)path
{
    UIImage *image = [UIImage imageWithContentsOfFile:path];
    
    // Normalize to get rid of image orientation or resize below the max image size
    CGSize maxSize = self.maxImageSize;
    CGSize size = image.size;
    if (size.width > maxSize.width || size.height > maxSize.height) {
        size = maxSize;
    }
    image = [image resizing:size];
    
    CGImageRef imageRef = image.CGImage;
    if ( imageRef ) {
        self.width = (int)CGImageGetWidth(imageRef) & 0xFFFFFFFE;
        self.height = (int)CGImageGetHeight(imageRef) & 0xFFFFFFFE;
    }
    
    return imageRef;
}

- (CGImageRef)creatImage4SolidColor:(UIColor *)color
{
    CGFloat scale = [UIScreen mainScreen].scale;
    
    CGFloat image_width = self.width / scale;
    CGFloat image_height = self.height / scale;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(image_width, image_height), YES, scale);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, color.CGColor);
    CGContextFillRect(context, CGRectMake(0, 0, image_width, image_height));
    
    UIImage *image_ = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    CGImageRef imageRef_ = image_.CGImage;
    
    return imageRef_;
}

- (CGImageRef)createImage4Theme:(NSString *)path
{
    CGImageRef image = [ThemeResourceLoader createImage4Theme:path config:self.config
                                             loadResourceData:^NSData *(NSString *itemId, NSString *subpath)
    {
        return [[AssetLibrary proxy] loadResourceDataAtSubpath:subpath forItemId:itemId];
    }];

    if(image != NULL) {
        self.width = (int)CGImageGetEvenNumberedWidth(image);
        self.height = (int)CGImageGetEvenNumberedHeight(image);
    }
    return image;
}

- (CGImageRef)createImage4Text:(NSString *)textinfo
{
    CGImageRef imageRef = [ThemeResourceLoader createImage4Text:textinfo];

    if(imageRef != NULL) {
        self.width = (int)CGImageGetEvenNumberedWidth(imageRef);
        self.height = (int)CGImageGetEvenNumberedHeight(imageRef);
    }
    return imageRef;
}

- (CGImageRef)createImage4Overlay:(NSString *)path
{
    // TODO.
    return NULL;
}

- (CGImageRef)createImage4PreviewTheme:(NSString *)path
{
    // TODO.
    return NULL;
}

- (void)dealloc
{
    [self.solidColors release];
    [self.imageList release];
    [_lutListinPreview release];
    [_lutListinExport release];

    [super dealloc];
}

- (instancetype) init
{
    self = [super init];
    if(self) {
        _width = 0; _height = 0;
        _imageList = [[NSMutableArray alloc] init];
        _lutListinExport = [[NSMutableArray alloc] init];
        _lutListinPreview = [[NSMutableArray alloc] init];
        
        v_exportTextureId = -1;
        v_previewTextureId = -1;
        c_exportTextureId = -1;
        c_previewTextureId = -1;
        
        self.solidColors = @{
                        @"red" : [UIColor redColor],
                        @"blue" : [UIColor blueColor],
                        @"green" : [UIColor greenColor],
                        @"black" : [UIColor blackColor],
                        @"white" : [UIColor whiteColor],
                        @"magenta" : [UIColor magentaColor],
                        @"yellow" : [UIColor yellowColor],
                        @"purple" : [UIColor purpleColor],
                        @"grey" : [UIColor grayColor],
                        @"gray" : [UIColor grayColor],
                        @"cyan" : [UIColor cyanColor],
                        @"aqua" : [UIColor cyanColor],
                        @"lightgray" : [UIColor lightGrayColor],
                        @"lightgrey" : [UIColor lightGrayColor],
                        @"darkgray" : [UIColor darkGrayColor],
                        @"darkgrey" : [UIColor darkGrayColor],
                        @"lime" : RGBA(0x0, 0xFF, 0x0, 1),
                        @"maroon" : RGBA(0x80, 0x00, 0x00, 1),
                        @"navy" : RGBA(0x00, 0x00, 0x80, 1),
                        @"olive" : RGBA(0x80, 0x80, 0x00, 1),
                        @"silver" : RGBA(0xC0, 0xC0, 0xC0, 1),
                        @"teal" : RGBA(0x00, 0x80, 0x80, 1),
                        @"fuschia" : RGBA(0xFF, 0x00, 0x80, 1)
                    };
        
        NexImageLoaderConfig config;
        config.subsampleThemeResourceRate = 0.5;
        config.subsampleThemeResourceMinSize = CGSizeMake(256, 256);
        self.config = config;
        self.maxImageSize = CGSizeMake( MAX_RENDER_RESOLUTION_SIZE * MAX_ZOOMIN_SCALE, MAX_RENDER_RESOLUTION_SIZE * MAX_ZOOMIN_SCALE);

    }
    return self;
}

#pragma mark - ImageLoaderProtocol

- (BOOL) openFile:(NSString *)path width:(NSUInteger *)width height:(NSUInteger *)height pixelData:(void **)pixelData imageResource:(void **)imageResource
{
    BOOL result = NO;
    
    @autoreleasepool {
        
    NexImageResource *resource = [[NexImageResource alloc] init];
    
    if([path hasPrefix:@"@solid:"] && [path hasSuffix:@".jpg"]) {
        self.width = 480;
        self.height = 270;
        
        int fromIndex = (int)@"@solid:".length;
        
        NSString *colorName = [path substringFromIndex:fromIndex].stringByDeletingPathExtension;
        UIColor *solidColor = self.solidColors[colorName];
        
        if (solidColor == nil) {
            solidColor = UIColor.blackColor;
            
            if(colorName.length == 9 && [colorName characterAtIndex:0] == [@"#" characterAtIndex:0]) {
                solidColor = [colorName hashHexARGBColor];
            }
        }
        resource.imageRef = [self creatImage4SolidColor:solidColor];
    } else if([path hasPrefix:@"phasset-image://"]) {
        resource.imageRef = [self createImage4PhAssetPath:path];
    } else {
        resource.imageRef = [self createImage4Path:path];
    }

    *width = CGImageGetEvenNumberedWidth(resource.imageRef);
    *height = CGImageGetEvenNumberedHeight(resource.imageRef);
    resource.pImageData = [ThemeResourceLoader getPixelsWithImage:resource.imageRef];
    *pixelData = resource.pImageData;
    *imageResource = resource;

    if (resource.imageRef) {
//        CGImageRetain(resource.imageRef);
        
        @synchronized(self.imageList) {
        
        [self.imageList addObject:resource];
        result = YES;
        }
    }
    [resource release];
    }
    return result;
}

- (BOOL) openThemeImage:(NSString *)path width:(NSUInteger *)width height:(NSUInteger *)height pixelData:(void **)pixelData imageResource:(void **)imageResource
{
    BOOL result = NO;

    @autoreleasepool {
        
    NexImageResource *resource = [[NexImageResource alloc] init];
    
    if([path hasPrefix:@"[Text]"]) {
        resource.imageRef = [self createImage4Text:path];
    } else if([path hasPrefix:@"[Overlay]"]) {
        resource.imageRef = [self createImage4Overlay:path];
    } else if([path hasPrefix:@"[ThemeImage]"]) {
        resource.imageRef = [self createImage4Theme:path];
    } else if([path hasPrefix:@"[PvwThImage]"]) {
        resource.imageRef = [self createImage4PreviewTheme:path];
    } else {
        NexLogE(@"[NexImage", @" %d] Not create image in %@", __LINE__, path);
    }
    
    *width = CGImageGetEvenNumberedWidth(resource.imageRef);
    *height = CGImageGetEvenNumberedHeight(resource.imageRef);
    resource.pImageData = [ThemeResourceLoader getPixelsWithImage:resource.imageRef];
    *pixelData = resource.pImageData;
    *imageResource = resource;

    if (resource.imageRef) {
        CGImageRetain(resource.imageRef);
        
        @synchronized(self.imageList) {
        
        [self.imageList addObject:resource];
        result = YES;
        }
    }
    [resource release];
    }
    return result;
}

- (void)releaseImageResource:(void *)param
{
    if(param == NULL) {
        return;
    }
    
    @synchronized(self.imageList) {
        
    NSUInteger index = [self.imageList indexOfObject:(NexImageResource *)param];
    if(index != NSNotFound) {
        NexImageResource *imageResource = [self.imageList objectAtIndex:index];

//    Comment: You just need to remember the Core Foundation memory management rules.
//             If the function name has "Create" in it, then you manage its memory; if not, then you don't.
        CGImageRelease(imageResource.imageRef);
        delete [] imageResource.pImageData;
        
        [self.imageList removeObjectAtIndex:index];
    } else {
        @throw [NSException exceptionWithName:@"GeneralException"
                                       reason:[NSString stringWithFormat:@"NexImageResource resource may have released already"]
                                     userInfo:nil];
    }
    }
}

- (BOOL) openThemeFile:(NSString *)path dataLength:(NSUInteger *)dataLength fileData:(void **)fileData
{
    BOOL result = NO;

    NSString *itemid = path; NSString *subpath = nil;
    
    if ([path containsString:@"/"]) {
        NSArray *splitArray = [path componentsSeparatedByString:@"/"];
        if( splitArray.count > 0 ) {
            itemid = (NSString *)splitArray[0];
            subpath = [path substringFromIndex:itemid.length+1];
        }
    }
    
    if (subpath != nil) {
        themeData = [[AssetLibrary proxy] loadResourceDataAtSubpath: subpath forItemId: itemid];
    } else {
        themeData = [[AssetLibrary library] itemForId:itemid].fileInfoResourceData;
    }

    if ( themeData ) {
        *dataLength = [self getThemeFileDataLength];
        *fileData = [self getThemeFileData];
        result = YES;
    } else {
        *dataLength = 0;
        *fileData = NULL;
    }
    
    return result;
}

- (NSUInteger)getThemeFileDataLength
{
    return themeData == nil ? 0 : themeData.length;
}

- (void *)getThemeFileData
{
    if( [self getThemeFileDataLength] != 0 ) {
        char* fileData = new char[themeData.length+1];
        
        memset(fileData, 0x0, themeData.length+1);
        memcpy(fileData, themeData.bytes, themeData.length);
        
        return (void *)fileData;
    } else {
        return NULL;
    }
}

#pragma mark - LUTLoaderProtocol

/// returns 0 if texture couldn't be created
- (int)getLUTWithID:(int)lutResourceId exportFlag:(int)exportFlag
{
    if((NXELutTypes)lutResourceId == NXE_LUT_NONE) {
        return 0;
    }
    
    NSMutableArray *lutList = nil;
    
    if(exportFlag) {
        lutList = self.lutListinExport;
    } else {
        lutList = self.lutListinPreview;
    }
    
    int textureId = 0;

    NXEFileLutSource* desc = [LUTMap.instance entryWith:lutResourceId];
    
    GLuint result = [desc createTexture];
    if(result != GL_NONE) {
        textureId = (int)result;
        [lutList addObject:@(textureId)];
    }
    return textureId;
}

- (void)deleteTextureForLUT
{
    for(NSNumber *textureId in self.lutListinExport) {
        [ImageUtil deleteTexture:textureId.intValue];
    }
    [self.lutListinExport removeAllObjects];
    
    for(NSNumber *textureId in self.lutListinPreview) {
        [ImageUtil deleteTexture:textureId.intValue];
    }
    [self.lutListinPreview removeAllObjects];
}

#pragma mark - VignetteLoaderProtocol

- (int)getVignetteTextID:(int)exportFlag
{
    UIImage *image = nil;
    
    BundleResourceKey resourceKey = BundleResourceKeyVignetteHorizontal;
    NXESizeInt ratio = NXEEngine.aspectRatio;
    if (ratio.width < ratio.height) {
        resourceKey = BundleResourceKeyVignetteVertical;
    }
    
    NSString *path = [BundleResource pathFor:resourceKey];
    image = [UIImage imageWithContentsOfFile:path];
    if(image == nil) {
        return 0;
    }
    
    if(exportFlag) {
        if(v_exportTextureId == -1) {
            v_exportTextureId = [ImageUtil glTextureIDfromImage:image];
        }
        return v_exportTextureId;
    } else {
        if(v_previewTextureId == -1) {
            v_previewTextureId = [ImageUtil glTextureIDfromImage:image];
        }
        return v_previewTextureId;
    }
}

- (void)deleteTextureForVignette
{
    if(v_exportTextureId != -1) {
        [ImageUtil deleteTexture:v_exportTextureId];
        v_exportTextureId = -1;
    }
    if(v_previewTextureId != -1) {
        [ImageUtil deleteTexture:v_previewTextureId];
        v_previewTextureId = -1;
    }
}

@end
