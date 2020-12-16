//
//  MyDocument.h
//  NexSecureBinaryEditor
//
//  Created by Matthew Feinberg on 8/13/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//


#import <Cocoa/Cocoa.h>

enum NexBinError {
	NexBinError_CouldNotFindObjectFileInFramework = 1,
	NexBinError_CouldNotFindNexSecureData = 2,
	NexBinError_NexSecureWareError = 3,
	NexBinError_CannotConvertFormat = 4
};

@interface MyDocument : NSDocument <NSTableViewDelegate, NSTableViewDataSource> 
{
	NSData *binaryData;
	NSData *frameworkData;
	NSTableView *tableView;
	NSButton *addItem;
	NSButton *delItem;
	NSMutableArray *props;
	NSString *versionInfo;
	NSString *fileName;
	BOOL isFramework;
	int chunkCount;
    BOOL pendingCCUpd;
}

@property (nonatomic,retain) NSData *binaryData;
@property (nonatomic,retain) IBOutlet NSTableView *tableView;
@property (nonatomic,retain) IBOutlet NSButton *addItem;
@property (nonatomic,retain) IBOutlet NSButton *delItem;
@property (nonatomic,retain) NSString *versionInfo;
@property (nonatomic,retain) NSData *frameworkData;

- (void)didClickAdd: (id)sender;
- (void)didClickDel: (id)sender;
- (BOOL)loadFileData: (NSData*)fileData error:(NSError **)outError;

@end
