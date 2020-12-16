/******************************************************************************
 * File Name   :	ColorFilterTableViewController.m
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

#import "ColorFilterTableViewController.h"
#import "CommonTypeATableViewCell.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"
#include <stdlib.h>
#include <mach/mach_time.h>

@interface ColorFilterTableViewController ()

@property (copy, nonatomic) ColorFilterSelected didSelect;
@property (nonatomic, strong) NSArray *builtinLutNames;
@property (nonatomic) NXELutID selectedLutId;
@property (nonatomic) NXELutID customLutId;
@end

@implementation ColorFilterTableViewController

- (void)setupColorFilterWithCB:(ColorFilterSelected)refreshColorFilter
                   selectedLUT:(NXELutID) lutId
{
    self.selectedLutId = lutId;
    self.didSelect = refreshColorFilter;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"ColorFilter Types";
    
    self.builtinLutNames = [LUTNames names];
    self.customLutId = [self createCustomLUTFile];
}

#pragma mark -

/// Creates a randomly generated Cube LUT file if not yet created
- (NXELutID) createCustomLUTFile
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *resourceRootURL = [fm URLsForDirectory:NSDocumentDirectory
                                        inDomains:NSUserDomainMask][0];
    
    NSURL *URL = [resourceRootURL URLByAppendingPathComponent:@"custom.cube"];
    
    // Create on if not yet.
    if (![fm fileExistsAtPath:URL.path]) {
        NSMutableString *content = [NSMutableString string];
        
        [content appendString:@"# Custom Cube LUT for NexEditor SDK Demo\n"];
        [content appendString:@"LUT_3D_SIZE 32\n"];
        
        srandom((unsigned) mach_absolute_time());
        
        double step = 1.0 / 16;
        double cap = step;
        for( int i = 0; i < 16; i++) {
            for( int j = 0; j < 2048; j++ ) {
                double r = ((double) random() / INT32_MAX) * cap + 0.000001;
                double g = ((double) random() / INT32_MAX) * cap + 0.000001;
                double b = ((double) random() / INT32_MAX) * cap + 0.000001;
                [content appendFormat: @"%1.6f %1.6f %1.6f\n", r, g, b];
            }
            
            cap += step;
        }
        
        [[content dataUsingEncoding:kCFStringEncodingUTF8] writeToFile:URL.path atomically:YES];
    }
 
    NXECubeLutSource *source = [[NXECubeLutSource alloc] initWithPath:URL.path];
    NSError *error = nil;
    NXELutID lutId = [NXELutRegistry registerLUTWithSource:source error:&error];
    if (error) {
        NSLog(@"ERROR: %@", error.localizedDescription);
    }
    return lutId;
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.builtinLutNames count] + 1;
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"ColorFilterCell" forIndexPath:indexPath];
    

    NXELutID lutId = [self lutIdFromIndexPath:indexPath];
    NSString *name = [self lutNameFromIndexPath:indexPath];
    
    if ( self.selectedLutId == lutId ) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    cell.titleLabel.text = name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.navigationController popViewControllerAnimated:YES];
    
    NXELutID lutId = [self lutIdFromIndexPath:indexPath];
    NSString *name = [self lutNameFromIndexPath:indexPath];
    
    self.didSelect(lutId, name);
}

#pragma mark -

- (NXELutID) lutIdFromIndexPath:(NSIndexPath *) indexPath
{
    NXELutID lutId;
    NSUInteger index = indexPath.row;
    if ( index == 0 ) {
        lutId = kLutIdNotFound;
    } else if (index < self.builtinLutNames.count) {
        lutId = [NXELutRegistry lutIdFromType:(NXELutTypes)index];
    } else {
        lutId = self.customLutId;
    }
    return lutId;
}

- (NSString *) lutNameFromIndexPath:(NSIndexPath *) indexPath
{
    return [LUTNames nameForLutId:indexPath.row];
}

@end


