API Changes
===========
# 1.5.1
Since 1.5.0

## New Classes
- NXEBeatTemplateProject
- NXEBeatAssetItem
- NXEClipTransform

## NXEEngine
### New
- @property (nonatomic, readonly) CGSize logicalRenderSize

## NXEAssetLibrary
### New
- @property (class, readonly) NSString * _Nonnull beatTemplate
- + (void) addAssetPackageDirectoryURLs:
- + (void) removeAssetPackageDirectoryURLs:
- + (void) removeAllAssetPackageDirectoryURLs

## NXEEditorErrorCode
### New Types
- ERROR_TEMPALTE_CONFIGURE_ERROR
- ERROR_TEMPLATE_ASSET_UNSUPPORT_VIDEOCLIPS

## NXEEditorType
### Removed Types
- NXEClipType

## NXEClip
### New
- @property (nonatomic) NXEClipTransformRamp transformRamp
- @property (nonatomic, readonly) CGSize size
- @property (getter=getClipEffectID, setter=setClipEffect:, nonatomic, strong) NSString *clipEffectID
- @property (getter=getTransitionEffectID, setter=setTransitionEffect:, nonatomic, strong) NSString *transitionEffectID

# 1.5.0
Since 1.3.6

## New Classes
- NXETemplateProject
- NXECollageTitleConfiguration
- NXECollageSlotConfiguration
- NXECollageProject
- NXECollageAssetItem
- NXEAssetItemGroup
- CollageTextStyle
- CollageInfo

## New Types
- FlipType
- CollageType
- CollageSizeInt

## NXELutRegistry
### New
- +(NSString *) lutStringFromType:

## NXEEngine
### New
- -(void) updatePreview

## NXEAssetLibrary
### New
- @property (class,readonly) NSString * collage
- - (NSArray<NXEAssetItemGroup *> *) groupsInCategory:
    
# 1.3.6
Since 1.3.5

## New Types
- NXEExportParams 
- NXELayerEditorStatusKey

## New Functions
- NXEExportParams NXEExportParamsMake(int width, int height)

## New Classes
- NXELayerEditorCoordinates

## NXEEngine
### New
- -(int) exportToPath:withParams:

## NXELayerEditor
### New
- @property (nonatomic, readonly) NXELayer *selectedLayer
- -(void) editorView:didChangeStatus:value:

## NXELayerEditorView
### New
- @property (nonatomic, readonly) NXEEngine *nxeeditor
- @property (nonatomic, readonly) NXELayerEditorCoordinates *layerCoordinates
- -(void) updatePreview
- -(NXELayer *) layerAtUIViewPoint:layers:

# 1.3.5
Since 1.3.4

## NXELayerEditorView
### New
- @property (nonatomic) BOOL isEditingEnabled 

## NXESimpleLayerEditor
### New
#### NXESimpleLayerEditorButtonType
- NXESimpleLayerEditorButtonTypeScaleRotate

# 1.3.4
Since 1.3.0

## NXEProject
### New
- NXEColorAdjustments NXEColorAdjustmentsMake(float brightness, float contrast, float saturation)
- @property (nonatomic) NXEColorAdjustments colorAdjustments

# 1.3.0
Since 1.2.0

## New Classes
- NXELutRegistry
- NXECubeLutSource
- NXELayerEditorView replaces deprecated NXEEngineView
- NXESimpleLayerEditor
- NXESimpleLayerEditorCustomization
- NXESimpleLayerEditorButtonCustomization
- NXESimpleLayerEditorSelectionBorderStyle

## NXEClip
### New
- @proeprty (nonatomic) NXELutID lutId

## NXELayer
### New
- @property (nonatomic) NXELutID lutId
- @property (nonatomic) CGFloat scale

# 1.2.0
Since 1.1.0

## New Classes
- NXEAssetItem
- NXEAssetLibrary
- NXEAssetItemCategory
- NXEClipSource
- NXEEffectAssetItem
- NXEEngineView
- NXETextEffect
- NXETextEffectParams
- NXEProject(TextEffect)
- NXETextEffectAssetItem
- NXETransitionAssetItem

## Removed Classes
- NXEColorEffect
- NXECrop
- NXEEffectLibrary
- NXETheme
- NXEThumbnailUICallbackBlock

## NXEVisualclip
### Removed
- @property (nonatomic, retain) NSArray *startMatrix
- @property (nonatomic, reatin) NSArray *endMatrix
- @property (nonatomic, retain) NSArray *audioClipVec

## NXEClip
### Removed
- @property (nonatomic, assign) float bgmVolumeScale
- @proeprty (nonatomic, retain) NXECrop* crop
- @property (nonatomic, retain) NXEColorEffect* colorEffect
- @property (nonatomic, retain) NSMutableDictionary* effectOptions
- @property (nonatomic, retain) NSMutableDictionary* transitionEffectOptions
- -(void) setColorEffectValues:
- -(void) setCropMode:
- -(instancetype) initWithPath:error:
- -(void) getVideoClipTndTime:MaxCount:Rotate:ThumbnailUICallbackBlock
- +(instancetype) newSupportedClip:

### New
- @proeprty (nonatomic) CropMode cropMode
- @property (nonatomic, readonly) NXETimeMillis durationMs
- @property (nonatomic, readonly) NXEClipSource* clipSource
- +(instancetype) clipWithSource:error:

## NXEEngine
### Removed
- +(NSString *) getVersionSDK
- -(int) getClipVideoThumbs:ThumbnailPath:Width:StartTime:EndTime:Count:Flag:UserFlag:videoThumbnailUICallback:
- -(int) captureCurrentFrame
- -(NSArray *) getLayerList:
- -(NXELayer *) getHittedLayer
- -(void) removeLayerForIndex:
- -(void) removeAllLayers
- -(void) addLayer:

### New
- +(NSString *) sdkVersionString
- -(void) captureFrameWithBlock:

##NXEEngineView
### Removed
- @property (nonatomic, readonly) NXELayer *selectedLayer

##NXEError
### Removed
- @property (nonatomic, retain) NSString *errorUserDescription

##NXEProject
### Removed
- -(int) getTotalClipCount:
- -(void) allClear:
- -(void) replaceClip:atIndex:
- -(void) addClip:
- -(void) addClip:atIndex:
- -(void) removeClip:
- -(void) removeClip:atIndex:
- -(BOOL) setBGM2Path:volumeScale:
- -(NXEClip *) getBGMClip

### New
- @property (nonatomic, copy) NSArray < NXEClip *> *visualClips
- @property (nonatomic, copy) NSArray < NXEClip *> *audioClips
- @property (nonatomic, copy) NSArray < NXELayer *> *layers
- @property (nonatomic, strong) NXEClip *bgmClip

## NXETemplateParser
### Removed
- -(NXETemplateInfo *) getTemplateInfo:inDirectory
- -(void) configureProject:withTemplateFile:inDirectory