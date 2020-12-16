API Changes
===========
# 1.3.1
Since 1.2.2

## NexEditor
### New APIs
- - (void) releaseLUTTexture:(int)lut_resource_id;

# 1.2.2
Since 1.1.0

## ImageLoaderProtocol
### Removed APIs
- - (int) getWidth;
- - (int) getHeight;

### Added APIs
- - (int) getWidth:(IMAGE_RESOURCE_TYPE)type;
- - (int) getHeight:(IMAGE_RESOURCE_TYPE)type;

## NXEVisualclip
### Removed Properties
- @property (nonatomic, retain) NSArray *startMatrix;
- @property (nonatomic, retain) NSArray *endMatrix;
- @property (nonatomic, retain) NSArray *audioClipVec;
