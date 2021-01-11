/**
 * File Name   : LUTMap.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import "LUTMap.h"
#import "AssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import "NXELutSource.h"

#define BUILTIN_LUT_SUBPATH     @"resource/asset/lut"
#define BUILTIN_LUT_EXTENSION   @"png"
#define BUILTIN_LUT_NAME_PREFIX @"lut_"


@interface LUTMap()
@property (nonatomic, strong) NSArray<NXEFileLutSource *> *lutSources;
@end

@implementation LUTMap
- (instancetype) init
{
    self = [super init];
    if ( self ) {
        [[self.class builtinLUTNames] enumerateObjectsUsingBlock:^(NSString * _Nonnull name, NSUInteger idx, BOOL * _Nonnull stop) {
            NSString *lutName = [NSString stringWithFormat:@"%@%@", BUILTIN_LUT_NAME_PREFIX, name];
            NXEPNGLutSource* source = [[[NXEPNGLutSource alloc] initWithPath:[self.class fullPathForLUTName:lutName] uniqueKey:name] autorelease];
            
            [self addEntry:source];
        }];
    }
    return self;
}

- (void) dealloc
{
    self.lutSources = nil;
    [super dealloc];
}

- (NXEFileLutSource*) entryWith:(NXELutID)lutId
{
    NXEFileLutSource* result = nil;
    if(lutId < self.lutSources.count) {
        result = self.lutSources[lutId];
    }
    return result;
}

- (NSUInteger) addEntry:(NXEFileLutSource*)source
{
    self.lutSources = [self.lutSources arrayByAddingObject:source];
    return self.lutSources.count-1;
}

- (NSUInteger) indexOfEntry:(NXEFileLutSource*)entry
{
    __block NSUInteger result = NSNotFound;
    
    [self.lutSources enumerateObjectsUsingBlock:^(NXEFileLutSource * _Nonnull source, NSUInteger idx, BOOL * _Nonnull stop) {
        if( [entry isEqual:source] ) {
            result = idx;
            *stop = YES;
        }
    }];
    return result;
}

#pragma mark -
- (NSArray<NXEFileLutSource *> *) lutSources
{
    if ( _lutSources == nil) {
        _lutSources = @[];
        [_lutSources retain];
    }
    return _lutSources;
}

/**
 * List of built-in LUT names for NXE_LUT_NONE through NXE_LUT_WARM, and up to "transsion_sports"
 */
+ (NSArray<NSString *> *) builtinLUTNames
{
    static NSArray<NSString *> *names = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        names = @[@"none",
                  @"afternoon",
                  @"almond_blossom",
                  @"autumn",
                  @"boring",
                  @"caramel_candy",
                  @"caribbean",
                  @"cinnamon",
                  @"cloud",
                  @"coral_candy",
                  @"cranberry",
                  @"daisy",
                  @"dawn",
                  @"disney",
                  @"england",
                  @"espresso",
                  @"eyeshadow",
                  @"gloomy",
                  @"jazz",
                  @"lavendar",
                  @"moonlight",
                  @"newspaper",
                  @"paris",
                  @"peach",
                  @"rainy",
                  @"raspberry",
                  @"retro",
                  @"sherbert",
                  @"shiny",
                  @"smoke",
                  @"stoneedge",
                  @"sunrising",
                  @"symphony_blue",
                  @"tangerine",
                  @"tiffany",
                  @"vintage_flower",
                  @"vintage_romance",
                  @"vivid",
                  @"warm",
                  @"blue",
                  @"blueonly",
                  @"dbright",
                  @"heat",
                  @"ludwig",
                  @"negative",
                  @"oldfilm",
                  @"rosy",
                  @"salmon_teal",
                  @"sunprint",
                  @"sunset",
                  @"sweet"];
        
        [names retain];
    });
    return names;
}

+ (NSString *) fullPathForLUTName:(NSString *) name
{
    
    NSBundle *bundle = [NSBundle bundleForClass:self.class];
    return [bundle pathForResource:name
                            ofType:BUILTIN_LUT_EXTENSION
                       inDirectory:BUILTIN_LUT_SUBPATH];
}


#pragma mark -
+ (NSUInteger)indexFromLutName:(NSString *)lutName;
{
    NSUInteger result = [[self instance] indexOfEntryForKey:lutName];
    
    if( result == NSNotFound ) {
        NXEAssetItem *item = [[AssetLibrary library] itemForId:lutName];
        NSString *path = nil;
        if ( item && (path = item.fileInfoResourcePath)) {
            NXEPNGLutSource *source = [[[NXEPNGLutSource alloc] initWithPath:path uniqueKey:lutName] autorelease];
            result = [self.instance addEntry:source];
        }
        if( result == NSNotFound ) {
            result = NXE_LUT_NONE;
        }
    }
    return result;
}

- (NSUInteger) indexOfEntryForKey:(NSString*)key
{
    __block NSUInteger index = NSNotFound;
    
    [self.lutSources enumerateObjectsUsingBlock:^(NXEFileLutSource * _Nonnull source, NSUInteger idx, BOOL * _Nonnull stop) {
        if( [source.uniqueKey isEqualToString:key] ) {
            index = idx;
            *stop = YES;
        }
    }];
    return index;
}


+ (NSString *)pathForIndex:(NSUInteger)index
{
    return [self.instance entryWith:(NXELutID)index].path;
}

#pragma mark -
+ (instancetype) instance
{
    static LUTMap *_instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _instance = [[self.class alloc] init];
    });
    return _instance;
}
@end

