//
//  MyDocument.m
//  NexSecureBinaryEditor
//
//  Created by Matthew Feinberg on 8/13/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "MyDocument.h"
#import "NexSecureWare.h"
#import "NexSecureUpdater.h"
#import <stdlib.h>

@implementation MyDocument

@synthesize binaryData;
@synthesize tableView;
@synthesize addItem;
@synthesize delItem;
@synthesize versionInfo;
@synthesize frameworkData;

- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		props = [[NSMutableArray alloc] init];
		
		//[props addObject:[NSArray arrayWithObjects: @"one", @"two", nil]];
		[self setHasUndoManager:NO];
    
    }
    return self;
}

- (NSInteger) numberOfRowsInTableView:(NSTableView *)tableView {
	return [props count];
}

- (id) tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	return [[props objectAtIndex:row] objectAtIndex:[[tableColumn identifier] integerValue] ];
}

- (void) tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	NSString *insertObject = [NSString stringWithFormat:@"%@", object];
	NSArray *rowData = [props objectAtIndex:row];
	switch( [[tableColumn identifier] integerValue] ) {
		case 0:
			if( [insertObject length] >= ITEM_NAME_MAX_LEN ) {
				insertObject = [insertObject substringToIndex:ITEM_NAME_MAX_LEN-1];
			}
			rowData = [NSArray arrayWithObjects: insertObject, [rowData objectAtIndex:1], nil];
			break;
		case 1:
			if( [insertObject length] >= ITEM_VALUE_MAX_LEN_V3 ) {
				insertObject = [insertObject substringToIndex:ITEM_VALUE_MAX_LEN_V3-1];
			}
			rowData = [NSArray arrayWithObjects: [rowData objectAtIndex:0], insertObject, nil];
			break;
		default:
			break;
	}
	[props replaceObjectAtIndex:row withObject:rowData];
	[self updateChangeCount:NSChangeDone];
}

- (BOOL) tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	return YES;
}

- (void)didClickAdd: (id)sender {
	if( [props count] < ITEM_MAX_COUNT_V3 ) {
		[props addObject:[NSArray arrayWithObjects: @"(item)", @"(value)", nil]];
		[tableView reloadData];
		[self updateChangeCount:NSChangeDone];
		
	}
}

- (void)didClickDel: (id)sender {
	if( [tableView selectedRow] >= 0 ) {
		[props removeObjectAtIndex:[tableView selectedRow]];
		[tableView reloadData];
		[self updateChangeCount:NSChangeDone];
	
	}
}

- (BOOL) tableView:(NSTableView *)tableView shouldSelectTableColumn:(NSTableColumn *)tableColumn {
	return NO;
}

- (void) tableViewSelectionDidChange:(NSNotification *)notification {
	
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
}

- (NSFileWrapper *) fileWrapperOfType:(NSString *)typeName error:(NSError **)outError {
	
	BOOL wantsFramework = NO;
	if( [typeName isEqualToString:@"Framework"] ) {
		wantsFramework = YES;
	}
	
	if( wantsFramework != isFramework ) {
		*outError = [NSError errorWithDomain:@"NexBinaryError" 
										code:NexBinError_CannotConvertFormat
									userInfo:nil];
		return NO;
	}
	
	NSFileWrapper *wrapper = [[NSFileWrapper alloc] initWithSerializedRepresentation: self.frameworkData];
	
	if( NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION) == NEXSECURE_ERROR_NONE ) {
		
		NEXSECUREINFO secureInfo = {0};
		secureInfo.m_SecureItem = (NEXSECUREITEM*)malloc(sizeof(NEXSECUREITEM)*ITEM_MAX_COUNT_V3);
		for(int i=0;i<ITEM_MAX_COUNT_V3;i++)
		{
			secureInfo.m_SecureItem[i].m_strItemValue = (char*)malloc(ITEM_VALUE_MAX_LEN_V3);
			
		}
		
		strcpy(secureInfo.m_strKey, "NEXTREAMING.COM_V3");
		
		if( [props count] <= ITEM_MAX_COUNT_V3 ) {
			secureInfo.m_uiSecureItemCount = [props count];
			for( int i=0; i<[props count]; i++ ) {
				strncpy( secureInfo.m_SecureItem[i].m_strItemName, [[[props objectAtIndex:i] objectAtIndex:0] UTF8String], ITEM_NAME_MAX_LEN );
				strncpy( secureInfo.m_SecureItem[i].m_strItemValue, [[[props objectAtIndex:i] objectAtIndex:1] UTF8String], ITEM_VALUE_MAX_LEN_V3 );
			}
		}
		
		NexSecure_SetInfo(&secureInfo);
		
		unsigned char resultBuf[25000] = {0};
		int resultLen = 0;
		
		NexSecure_GetBitsData(resultBuf, sizeof(resultBuf), &resultLen);
		
		NexSecure_Destroy();
        
		if( isFramework ) {
		
			for( NSString *name in [wrapper fileWrappers] ) {
				
				NSFileWrapper *child = [[wrapper fileWrappers] objectForKey:name];
				
				if( [name isEqual:@"NexPlayerSDK"] || [name isEqual:@"NexEditorFramework"] ) {
					NSMutableData *fileData = [[child regularFileContents] mutableCopy];
					
                    int updatedChunks = NexSecureUpdater_updateAllChunks([fileData mutableBytes], 
                                                                         [fileData length], 
                                                                         (char*)resultBuf);
					if( updatedChunks ) {
                        NSLog(@"Updated %i chunks.", updatedChunks);
						[wrapper removeFileWrapper:child];
						[wrapper addRegularFileWithContents:fileData preferredFilename:@"NexEditorFramework"];
					} else {
                        NSLog(@"Unable to update any chunks.");
                    }
				}
			}
		
		} else {
			
			NSMutableData *fileData = [[wrapper regularFileContents] mutableCopy];
			
			if( NexSecureUpdater_updateAllChunks([fileData mutableBytes], [fileData length], (char*)resultBuf) ) {
				wrapper = [[NSFileWrapper alloc] initRegularFileWithContents:fileData];
			}
		}
	}
	
	return wrapper;	
}

- (BOOL)loadFileData: (NSData*)fileData error:(NSError **)outError {
	char *firstChunk = NexSecureUpdater_getFirstChunk([fileData bytes], [fileData length]);
	if( !firstChunk ) {
		*outError = [NSError errorWithDomain:@"NexBinaryError" 
										code:NexBinError_CouldNotFindNexSecureData
									userInfo:nil];
		return NO;
	}
	
	char *vinf = NexSecureUpdater_getVersionInfo([fileData bytes], [fileData length]);
	if( vinf ) {
		versionInfo = [NSString stringWithUTF8String:vinf];
	} else {
		versionInfo = nil;
	}
	
	chunkCount = NexSecureUpdater_countAllChunks([fileData bytes], [fileData length]);
	NSLog(@"Found %i chunks.", chunkCount);
	
	if( NexSecure_Create(NEXSECURE_MAJOR_VERSION, NEXSECURE_MINOR_VERSION) != NEXSECURE_ERROR_NONE ) {
		*outError = [NSError errorWithDomain:@"NexBinaryError" 
										code:NexBinError_NexSecureWareError
									userInfo:nil];
		return NO;
	}
	
	NexSecure_SetBitsData((unsigned char*)firstChunk, strlen(firstChunk)+1);
	
	NEXSECUREINFO secureInfo = {0};
	
	NexSecure_GetInfo(&secureInfo);
	
	for( int i=0; i<secureInfo.m_uiSecureItemCount; i++ ) {
		[props addObject:[NSArray arrayWithObjects: 
						  [NSString stringWithUTF8String:secureInfo.m_SecureItem[i].m_strItemName],
						  [NSString stringWithUTF8String:secureInfo.m_SecureItem[i].m_strItemValue],
						  nil]];
	}
	
	NexSecure_Destroy();
	
	return YES;
}


- (BOOL) readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError {

	fileName = [fileWrapper filename];
	
	if( [typeName isEqualToString:@"Framework"] ) {
		
		isFramework = YES;
		
		BOOL bFoundObjectFile = NO;
		
		for( NSString *name in [fileWrapper fileWrappers] ) {
			
			NSFileWrapper *child = [[fileWrapper fileWrappers] objectForKey:name];
			
			if( [name isEqual:@"NexEditorFramework"] || [name isEqual:@"NexPlayerSDK"] ) {
				
				bFoundObjectFile = YES;
				
				NSData *fileData = [child regularFileContents];
				if( ![self loadFileData:fileData error:outError] )
					return NO;
			}
		}
		
		self.frameworkData = [fileWrapper serializedRepresentation];
		
		if( !bFoundObjectFile ) {
			if ( outError != NULL ) {
				*outError = [NSError errorWithDomain:@"NexBinaryError" 
												code:NexBinError_CouldNotFindObjectFileInFramework 
											userInfo:nil];
				return NO;
			}
		}
		
	} else {
		
		isFramework = NO;
		NSData *fileData = [fileWrapper regularFileContents];
		if( ![self loadFileData:fileData error:outError] )
			return NO;
		self.frameworkData = [fileWrapper serializedRepresentation];
	}
	
    return YES;
}

- (IBAction)didEditInEditor:(id)sender
{
    
    
}

@end
