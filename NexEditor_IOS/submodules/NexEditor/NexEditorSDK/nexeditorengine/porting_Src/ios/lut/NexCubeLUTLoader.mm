
#import "NexCubeLUTLoader.h"
#import "NexTheme_CubeLUT.h"

@implementation NexCubeLUTLoader

+ (NexCubeLUTBytes) loadCubeLUTBytesFromData:(NSData *) data
{
    const char *source = (const char*)[data bytes];
    std::string string(source);
    std::istringstream stream(string);
    
    CubeLUT cubelut;
    cubelut.LoadCubeFile(stream);
    
    return (NexCubeLUTBytes) cubelut.createLUT();
}

+ (void) unloadCubeLUTBytes:(NexCubeLUTBytes) bytes
{
    // CubeLUT created bytes should be freed by delete[] (int *)
   delete[] ((int *) bytes);
}

@end
