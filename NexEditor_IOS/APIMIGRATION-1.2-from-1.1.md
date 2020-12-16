# NexEditor SDK API Migration Guide: 1.2 from 1.1

## Introduction

The most significant changes in NexEditor SDK 1.2 are replacing layer management methods in NXEEngine with *layers* property of NXEProject, replacing clip management methods in NXEProject with NSArray based visualClips and audioClips properties, and moving *-getHittedLayer* from NXEEngine class to NXEEngineView class with a correction to the name: selectedLayer, as a property.

In this document, we suggest alternatives for removed or changed API methods from NexEditor SDK 1.1.x and with introduction to corresponding new API methods or properties as well as example code snippets.

## NXEEngine Changes
 + getVersionSDK -> sdkVersionString
 + getLayerList:, removeLayerForIndex:, removeAllLayers, addLayer: -> NXEProject.layers
 + getHittedLayer -> NXEEngineView.selectedLayer

### Removed +(NSString *) getVersionSDK
    // removed
    +(NSString *) getVersionSDK;

    // new
    +(NSString *) sdkVersionString;


To get the SDK version string, use *+(NSString *) sdkVersionString* instead.
    
    NSLog(@"Version:%@", [NXEEngine sdkVersionString]);

### Removed Layer related methods from NXEEngine and added layers property NXEProject

	// removed from NXEEngine
	-(void) addLayer:
	-(NSArray *) getLayerList:
	-(void) removeLayerForIndex:
	-(void) removeAllLayers
	
	// new to NXEProject
	- @property (nonatomic, copy) NSArray < NXELayer *> *layers


From SDK version 1.2, NXELayer objects are managed by NXEProject instance, not NXEEngine. Hence, to add a new NXELayer object, use *layers* property of a NXEProject instance.

	NXEEngine *editor = NXEEngine.instance;
	
	// SDK 1.1.x
	NXETextLayer *textLayer = [[NXETextLayer alloc] initWithText:text font:nil point:CGPointMake(-1, -1)];
	[editor addLayer:textLayer];
	
	// SDK 1.2
	NXETextLayer *textLayer = [[NXETextLayer alloc] initWithText:text font:nil point:CGPointNXELayerCenter];
	editor.project.layers = [editor.project.layers arrayByAddingObject:textLayer];


*-getLayerList:* alternative

	NXEEngine *editor = NXEEngine.instance;
	
	// SDK 1.1.x
	NSArray *textLayers = [editor getLayerList:NXE_LAYER_TEXT];
	
	// SDK 1.2.x
	NSPredicate *predicate = [NSPredicate predicateWithFormat:@"layerType == %@", @(NXELayerType)];
	NSArray *textLayers = [editor.project.layers filteredArrayUsingPredicate:predicate];


*-removeLayerForIndex:* alternative

	NXELayer *layerToRemove = ...;
	NXEEngine *editor = NXEEngine.instance;
	
	// SDK 1.1.x
	[editor removeLayerFroIndex:layerToRemove.layerId];
	
	// SDK 1.2.x (if ARC enabled)
	NSMutableArray *layers = [editor.project.layers mutableCopy];
	[layers removeObject:layerToRemove];
	editor.project.layers = [layers copy];

*-removeAllLayers* alternative

	NXEEngine *editor = NXEEngine.instance;
	// SDK 1.1.x
	[editor removeAllLayers];
	
	// SDK 1.2.x
	editor.project.layers = @[];

### Removed NXEEngine's -getHittedLayer. Use NXEEngineView's selectedLayer property instead

	// SDK 1.1.x
	NXELayer *selected = [NXEEngine.instance getHittedLayer];
	
	// SDK 1.2
	NXEEngineView *engineView = /* The NXEEngineView configured in a UIViewController */;
	NXELayer *selected = engineView.selectedlayer;


## NXEClip Changes
+ +newSuportedClip:, -initWithPath:error: -> +clipWithSource:error

### Removed +newSupportedClip: and -initWithPath:error:. Use +clipWithSource:error: instead
To instantiate a NXEClip, use +(instancetype) clipWithSource:error: with NXEClipSource object.


	// removed
	- (instancetype)initWithPath:(NSString *)path error:(NXEError **)error;
	+ (instancetype)newSupportedClip:(NSString *)path;
	
	// new
	 + (instancetype)clipWithSource:(NXEClipSource *) source error:(NSError **)error;

Create a NXEClipSource from one of various sources such as PHAsset photo, AVURLAsset video, or path to a local image or video file. 

The following code adds a clip from a PHAsset object:

	NXEProject *project = [[NXEProject alloc] init];
	PHAsset *asset = // fetch a PHAsset from Photos using PHAssetCollection;
	NXEClipSource *clipSource = [NXEClipSource sourceWithPHAsset:asset];
	NXEClip *clip = [NXEClip clipWithSource:clipSource error:nil];
	project.visualClips =  @[clip];

If a photo or video is located at a path on the file system, instantiate NXEClipSource with +sourceWithPath:

	NSString *path = "/path/from/root/to/image.jpg";
	NXEClipSource *clipSource = [NXEClipSource sourceWithPath:path];

 
## NXEProject Changes
+ getTotalClipCount:, allClear:, replaceClip:atIndex:, addClip:, addClip:atIndex:,removeClip:, removeClip:atIndex: -> visualClips, audioClips
+  setBGM2Path:volumeScale:
+ getBGMClip -> bgmClip with     [bgmClip setClipVolume:(int)(scale*200)];

### Clip management methods are replaced by visualClips and audioClips properties

NXEProject's clip management methods for adding, removing, and counting are replaced by visualClips and audioClips property. Since these properties are NSArray<NXEClip *> type, clips still can be set, added, and removed using NSArray methods.

	// removed
	-(int) getTotalClipCount:
	-(void) allClear:
	-(void) addClip:
	-(void) addClip:atIndex:
	-(void) removeClip:
	-(void) removeClip:atIndex:
	-(void) replaceClip:atIndex:
	
	// new
	@property (nonatomic, copy) NSArray<NXEClip *> *visualClips;
	@property (nonatomic, copy) NSArray<NXEClip *> *audioClips;


*-getTotalClipCount:* and allClear: alternatives

	NXEProject *project = ...;
	
	// SDK 1.1.x
	int vcount = [project getTotalClipCount:YES]; // number of visual clips
	int acount = [project getTotalClipCount:NO];  // number of audio clips
	
	[project allClear:YES]; // clear visual clips
	[project allClear:NO];  // clear audio clips
	
	// SDK 1.2
	int vcount = project.visualClips.count;
	int acount = project.audioClips.count;
	
	project.visualClips = @[];
	project.audioClips = @[];


*-addClip:* and *-addClip:atIndex:* alternatives

	NXEProject *project = ...;
	NXEClip *imageClip = ...;
	NXEClip *videoClip = ...;
	NXEClip *audioClip = ...;
	NXEClip *imageClip2 = ...;
	
	// SDK 1.1.x
	[project addClip:imageClip];
	[project addClip:videoClip];
	[project addClip:audioClip];
	
	[project addClip:imageClip2 atIndex:1];
	
	// SDK 1.2 (if ARC enabled)
	project.visualClips = @[imageClip, videoClip];
	project.audioClips = @[audioClip];
	
	NSMutableArray *clips = [project.visualClips mutableCopy];
	[clips insertObject:imageClip2 atIndex:1];
	project.visualClips = [clips copy];


*-removeClip:* alternative

> We decided to remove <b>-removeClip:atIndex:</b> because it did not make much sense to pass the clip as the first parameter only to determine if it is a visual or audio clip.

```
	NXEProject *project = ...;
	int clipIndexToRemove = 1;
	NXEClip *anotherClipToRemove = ...;
	
	// SDK 1.1.x
	NXEClip *toRemove = project.visualItems[clipIndexToRemove];
	[project removeClip:toRemove atIndex:clipIndexToRemove];
	
	[project removeClip:anotherClipToRemove];
	
	// SDK 1.2
	
	/// A helper NSArray extension for convenient access to visualClips and audioClips properties of NXEProject
	
	@interface NSArray (Helper)
	- (NSArray *) removingObject:(id) object;
	@end
	
	@implementation NSArray (Helper)
	- (NSArray *) removingObject:(id) object
	{
	    NSMutableArray *mutable = [self mutableCopy];
	    [mutable removeObject:object];
	    return [mutable copy];
	}
	@end
	
	NXEClip *toRemove = project.visualClips[clipIndexToRemove];
	project.visualClips = [project.visualClips removingObject:toRemove];
	
	project.visualClips = [project.visualClips removingObject:anotherClipToRemove];
```

*-replaceClip:atIndex:* alternative
```
	NXEProject *project = ...;
	int clipIndexToReplace = 2;
	NXEClip *newClip = ...;
	
	// SDK 1.1.x
	[project replaceClip:newClip atIndex:clipIndexToReplace];
	
	// SDK 1.2 (if ARC enabled)
	
	NSMutableArray *clips = [project.visualClips mutableCopy];
	[clips replaceObjectAtIndex:clipIndexToReplace withObject:newClip];
	project.visualClips = [clips copy];
```

### BGM API Change

```
	// removed
	-(BOOL) setBGM2Path:volumeScale:
	-(NXEClip *) getBGMClip
	
	// new
	- @property (nonatomic, strong) NXEClip *bgmClip;
```
Alternatives
```
	NXEProject *project = ...;
	NSString *bgmPath = @"path/to/music/clip.aac";
	
	// SDK 1.1.x
	[project setBGM2Path:bgmPath volumeScale: 0.7f];
	NXEClip *bgmClip = [project getBGMClip];
	
	// SDK 1.2
	NXEClipSource *clipSource = [NXEClipSource sourceWithPath:bgmPath];
	NXEClip *musicClip = [NXEClip clipWithSource:clipSource error:nil];
	[musicClip setClipVolume:(int)(0.7f * 200)];
	project.bgmClip = musicClip;
	
	NXEClip *bgmClip = project.bgmClip;
```