/******************************************************************************
 * File Name   :
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#import <Foundation/Foundation.h>
#import "NexEditorLog.h"
#import "nexSAL_internal.h"
#import "NexSAL_Com.h"
#import "SALBody_Debug.h"

#define LOG_TAG @"NexEditorLog"
#define NEX_TRACE_CATEGORY_INVALID -1

static void _outstring(const char *p);
static const NSString* prefixForCategory(NexEditorLogCategory category);

const NSString* kPrefixAudio         = @"AUD";
const NSString* kPrefixVideo         = @"VID";
const NSString* kPrefixError         = @"ERR";
const NSString* kPrefixWarning       = @"WRN";
const NSString* kPrefixInfo          = @"INF";
const NSString* kPrefixDebug         = @"DBG";
const NSString* kPrefixMiscellaneous = @"MIS";

@interface NexEditorLog()

@property (nonatomic, retain) NSDictionary* categories;

@end

@implementation NexEditorLog

- (void) setLogLevel:(NSInteger)level forCategory:(NexEditorLogCategory)category
{
    const NSString* prefix = prefixForCategory(category);
    
    if ( prefix ) {
        NSArray* salCategories = [self.categories objectForKey:prefix];

        for( NSNumber* salCategory in salCategories ) {
            nexSAL_TraceSetPrefix( salCategory.intValue, [prefix UTF8String]);
            nexSAL_TraceSetCondition( salCategory.intValue, (int)level );
        }
    }
}

+ (void) setLogLevel:(NSInteger)level forCategory:(NexEditorLogCategory)category
{
    [[[self class] sharedInstance] setLogLevel:level forCategory:category];
}


- (void) logWithCategory:(NexEditorLogCategory)category tag:(NSString *)tag message:(NSString *) message
{
    const NSString* prefix = prefixForCategory(category);
    
    if (prefix) {
        NSArray<NSNumber *>* salCategories = [self.categories objectForKey:prefix];
        
        NSNumber *salCategory = salCategories.firstObject;
        if (salCategory) {
            int iKeyCategory = salCategory.intValue;

            if ( nexSAL_TraceCondition(iKeyCategory, 0) ) {
                if (tag)
                    message = [NSString stringWithFormat:@"%@: %@", tag, message];
                
                NEXSAL_TRACECAT(iKeyCategory, 0, "%s", [message UTF8String]);
            }
        }
    }
}

+ (void) logWithCategory:(NexEditorLogCategory)category tag:(NSString *)tag message:(NSString *) message
{
    [[[self class] sharedInstance] logWithCategory:category tag:tag message:message];
}

+ (int) salCategoryForNexEditorLogCategory:(NexEditorLogCategory)category {
    
    int result = NEX_TRACE_CATEGORY_INVALID;
    const NSString* prefix = prefixForCategory(category);
    NSDictionary *categories = (NSDictionary *)[self.class.sharedInstance categories];
    
    if (prefix) {
        NSArray<NSNumber *>* salCategories = categories[prefix];
        result = salCategories.firstObject.intValue;
    }
    return result;
}


- (id) init {
    self = [super init];
    if(self) {
        self.categories = @{
                            kPrefixAudio:@[@(NEX_TRACE_CATEGORY_E_AUDIO), @(NEX_TRACE_CATEGORY_P_AUDIO)],
                            kPrefixVideo:@[@(NEX_TRACE_CATEGORY_E_VIDEO), @(NEX_TRACE_CATEGORY_P_VIDEO)],
                            kPrefixError:@[@(NEX_TRACE_CATEGORY_E_SYS), @(NEX_TRACE_CATEGORY_ERR)],
                            kPrefixWarning:@[@(NEX_TRACE_CATEGORY_WARNING)],
                            kPrefixDebug:@[@(NEX_TRACE_CATEGORY_FLOW), @(NEX_TRACE_CATEGORY_SOURCE)],
                            kPrefixInfo:@[@(NEX_TRACE_CATEGORY_INFO)],
                            kPrefixMiscellaneous:@[@(NEX_TRACE_CATEGORY_P_SYS), @(NEX_TRACE_CATEGORY_NONE), @(NEX_TRACE_CATEGORY_USER_START),
                                                   @(NEX_TRACE_CATEGORY_TEXT), @(NEX_TRACE_CATEGORY_DLOAD), @(NEX_TRACE_CATEGORY_F_READER),
                                                   @(NEX_TRACE_CATEGORY_F_WRITER), @(NEX_TRACE_CATEGORY_PVPD), @(NEX_TRACE_CATEGORY_PROTOCOL),
                                                   @(NEX_TRACE_CATEGORY_DIVXDRM), @(NEX_TRACE_CATEGORY_RFC), @(NEX_TRACE_CATEGORY_TARGET),
                                                   @(NEX_TRACE_CATEGORY_FASTPLAY), @(NEX_TRACE_CATEGORY_CRAL)]
                            };
        
        for(int i=0; i<NEXSAL_MAX_TRACE_CATEGORY; i++) {
            nexSAL_TraceSetCondition(i, LogLevelDisabled);
        }
        SALBody_RegisterOutputStringFunction((SALBodyOutStringFunction) _outstring);
        nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, nexSALBody_DebugPrintf2, NULL );
        nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, nexSALBody_DebugOutputString, NULL );
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
    [_categories release];
    _categories = nil;
}

+ (id) sharedInstance
{
	static id inst = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		inst = [[[self class] alloc] init];
	});
	return inst;
}

+ (void) logMessage:(NSString*)tag message:(NSString*) message
{
    NSLog(@"%@: %@", tag, message);
}
@end

#pragma mark -

static void _outstring(const char *p)
{
	NSLog(@"%s", p);
}

static const NSString* prefixForCategory(NexEditorLogCategory category) {
    const NSString* prefix = nil;
    switch(category)
    {
        case NexEditorLogCategoryAudio:
            prefix = kPrefixAudio;
            break;
        case NexEditorLogCategoryVideo:
            prefix = kPrefixVideo;
            break;
        case NexEditorLogCategoryErr:
            prefix = kPrefixError;
            break;
        case NexEditorLogCategoryWarning:
            prefix = kPrefixWarning;
            break;
        case NexEditorLogCategoryInfo:
            prefix = kPrefixInfo;
            break;
        case NexEditorLogCategoryDebug:
            prefix = kPrefixDebug;
            break;
        case NexEditorLogCategoryMisc:
            prefix = kPrefixMiscellaneous;
            break;
        default:
            break;
    }
    return prefix;
}

static void _NexEditorLogVAList(NexEditorLogCategory cat, NSString *t, NSString *format, va_list args) {
    int salCategory = [NexEditorLog salCategoryForNexEditorLogCategory:cat];
    
    if ( salCategory != NEX_TRACE_CATEGORY_INVALID && nexSAL_TraceCondition(salCategory, 0) ) {
        NSString *message = [[[NSString alloc] initWithFormat:format arguments:args] autorelease];
        [NexEditorLog logWithCategory:cat tag:t message:message];
    }
}

void _NexLog(NexEditorLogCategory cat, NSString *t, NSString *format, ...) {
    va_list args;
    va_start(args, format);
    
    _NexEditorLogVAList(cat, t, format, args);
    va_end(args);
}

