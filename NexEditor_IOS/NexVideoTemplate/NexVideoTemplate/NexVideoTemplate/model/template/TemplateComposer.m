/******************************************************************************
 * File Name   :	TemplateComposer.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/
#import "TemplateComposer.h"
#import "NexEditorFramework/NexEditorFramework.h"
#import "Util.h"

@interface TemplateComposer()

@property (nonatomic, retain) NSMutableArray *templateList;
@property (nonatomic, retain) NXETemplateParser *templateParser;

@end

@implementation TemplateComposer

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.templateList = [[NSMutableArray alloc] init];
        self.templateParser = [[NXETemplateParser alloc] init];
        [self initializeTemplateList];
        return self;
    }
    return nil;
}

- (NSArray *)recursivePathsForResourcesOfType:(NSString *)type inDirectory:(NSString *)directoryPath
{
    NSMutableArray *filePaths = [[NSMutableArray alloc] init];
    NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingString:directoryPath]];
    
    NSString *filePath;
    
    while((filePath = [enumerator nextObject]) != nil) {
        if([[filePath pathExtension] isEqualToString:type]) {
            NSArray *splitArray = [filePath componentsSeparatedByString:@"/"];
            
            NSString *fullname = [splitArray objectAtIndex:[splitArray count]-1];
            [filePaths addObject:fullname];
        }
    }
    
    return filePaths;
}

- (void)initializeTemplateList
{
    NSArray *filePathArray = [self recursivePathsForResourcesOfType:@"txt" inDirectory:@"/Resource/asset/template"];
    
    for(NSString *fileName in filePathArray) {
        NSString *name = [fileName stringByDeletingPathExtension];
        NSString *extenstion = [fileName pathExtension];
        NSString *filePath = [[NSBundle mainBundle] pathForResource:name ofType:extenstion inDirectory:@"Resource/asset/template"];
        NXETemplateInfo *templateInfo = [self.templateParser getTemplateInfo:fileName inDirectory:@"Resource/asset/template"];
        
        NSString *imagePath = [[[NSString stringWithFormat:@"%@.png", templateInfo.templateName] lowercaseString] stringByReplacingOccurrencesOfString:@" " withString:@""];
        UIImage *image = [UIImage imageNamed:imagePath];
        [self addTemplateItemWithName:templateInfo.templateName
                          description:templateInfo.templateDescription
                                 path:filePath
                                image:image];
        
    }
    
    return;
}

- (void)addTemplateItemWithName:(NSString *)name description:(NSString *)description path:(NSString *)path image:(UIImage*)image
{
    if([description hasPrefix:@"Portrait"]) {
        // [0]: portrait / landscape
        // [1]: template name
        // [2]: template description
        // [3]: template file path
        [self.templateList addObject:@[@"portrait", name, description, path, image]];
    } else {
        // [0]: portrait / landscape
        // [1]: template name
        // [2]: template description
        // [3]: template file path
        [self.templateList addObject:@[@"landscape", name, description, path, image]];
    }
}

- (NSUInteger)listCount
{
    return [self.templateList count];
}

- (UIImage*)getTemplateImageAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio
{
    if(aspectRatio == NXE_ASPECT_16v9) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // landscape에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template name값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"landscape"]) {
                [arrays addObject:array[4]];
            }
        }
        
        return arrays[index];
    } else if(aspectRatio == NXE_ASPECT_9v16) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // portrait에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template name값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"portrait"]) {
                [arrays addObject:array[4]];
            }
        }
        
        return arrays[index];
    }
    return nil;
}

- (NSString*)getTemplateNameAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio
{
    if(aspectRatio == NXE_ASPECT_16v9) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // landscape에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template name값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"landscape"]) {
                [arrays addObject:array[1]];
            }
        }
        
        return arrays[index];
    } else if(aspectRatio == NXE_ASPECT_9v16) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // portrait에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template name값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"portrait"]) {
                [arrays addObject:array[1]];
            }
        }
        
        return arrays[index];
    }
    return nil;
}

- (NSArray *)getTemplateItemAtIndex:(NSUInteger)index aspectRatio:(NXEAspectType)aspectRatio
{
    if(aspectRatio == NXE_ASPECT_16v9) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // landscape에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template info값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"landscape"]) {
                [arrays addObject:array];
            }
        }
        
        return arrays[index];
    } else if(aspectRatio == NXE_ASPECT_9v16) {
        NSMutableArray *arrays = [NSMutableArray array];
        
        // self.templateList 배열에는 landscape, portrait에 대한 값이 모두 들어있다.
        // portrait에 대한 자료만 거르고 인자로 들어온 index에 해당하는 template info값을 리턴 해 준다.
        //
        for(NSArray *array in self.templateList) {
            if([array[0] isEqualToString:@"portrait"]) {
                [arrays addObject:array];
            }
        }
        
        return arrays[index];
    }
    
    return nil;
}

- (void)applyTemplateToProject:(NXEProject*)project templateFileName:(NSString *)templateFileName inDirectory:(NSString *)subpath
{
    [self.templateParser applyTemplateToProject:project templateFileName:templateFileName inDirectory:subpath];
}

@end
