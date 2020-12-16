/******************************************************************************
 * File Name   :	EffectItemTableViewController.m
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

#import "EffectItemTableViewController.h"
#import "EffectOptionTableViewController.h"
#import "CommonTypeATableViewCell.h"
#import "ItemQuery.h"

@import NexEditorFramework;

@interface EffectItemTableViewController ()

@property (copy, nonatomic) void (^refreshEffect)(int, NSString*);
@property (retain, nonatomic) NSArray<NSString *> *itemIds;
@property (retain, nonatomic) NXEClip *clip;
@property (assign, nonatomic) NXEEffectType itemType;

@end

@implementation EffectItemTableViewController {
    NSString *effectId;
}

- (void)setupEffectListrWithCB:(refreshEffect)refreshEffect itemType:(NXEEffectType)type clip:(NXEClip *)clip
{
    self.refreshEffect = refreshEffect;
    //
    self.itemType = type;
    self.clip = clip;
}

- (void)dealloc
{

}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSString *label;
    if(self.itemType == NXE_CLIP_EFFECT) {
        label = @"Clip Effects";
    } else {
        label = @"Transition Effects";
    }
    self.title = [NSString stringWithFormat:@"%@/%@", @"Effect Items", label];
    
    NSArray<NSString *> *ids = [ItemQuery itemIdsForType:self.itemType];
    
    NSString *aspectTypeString = @"_9v16";
    NSString *format;
    if (NXEEngine.instance.aspectType == NXEAspectTypeRatio9v16) {
        format = @"SELF CONTAINS %@";
    } else {
        format = @"NOT (SELF CONTAINS %@)";
    }
    NSPredicate *predicate = [NSPredicate predicateWithFormat:format, aspectTypeString];
    ids = [ids filteredArrayUsingPredicate:predicate];
    
    self.itemIds = ids;
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.itemIds.count + 1; /*for "none"*/
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"EffectItemCell"
                                                                     forIndexPath:indexPath];
    NSString *selectedItemId = nil;
    if(self.itemType == NXE_CLIP_EFFECT) {
        selectedItemId = self.clip.clipEffectID;
    } else {
        selectedItemId = self.clip.transitionEffectID;
    }
    
    NSString *itemIdAtRow = kEffectIDNone;
    if ( indexPath.row > 0 ) {
        itemIdAtRow = self.itemIds[indexPath.row - 1];
    }
    
    if([selectedItemId isEqualToString:itemIdAtRow]) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    if([itemIdAtRow containsString:@"com.nexstreaming."]) {
        cell.titleLabel.text = [itemIdAtRow substringFromIndex:[@"com.nexstreaming." length]];
    } else {
        cell.titleLabel.text = itemIdAtRow;
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *selectedEffectId = nil;

    BOOL hasOptions = NO;
    if(indexPath.row == 0) {
        selectedEffectId = kEffectIDNone;
    } else {
        selectedEffectId = [self.itemIds objectAtIndex:indexPath.row-1];
        
        NXEEffectOptions *effectOption = [[NXEEffectOptions alloc] init];
        hasOptions = [effectOption setupWithEffectId:selectedEffectId type:self.itemType];
    }
    
    if (!hasOptions) {
        [self.navigationController popViewControllerAnimated:YES];
        
        if (self.itemType == NXE_CLIP_EFFECT) {
            self.clip.clipEffectID = selectedEffectId;
        } else {
            self.clip.transitionEffectID = selectedEffectId;
        }
        self.refreshEffect(self.itemType, selectedEffectId);
    } else {
        effectId = selectedEffectId;
        [self performSegueWithIdentifier:@"segueEffectOptionEditList" sender:self];
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueEffectOptionEditList"]) {
        EffectOptionTableViewController *eo = (EffectOptionTableViewController *)[segue destinationViewController];
        NXEEffectType effectType = self.itemType;
        [eo setupEffectOption:effectId effectType:self.itemType configured:^(NXEEffectOptions *options) {
            if (effectType == NXE_CLIP_EFFECT) {
                [self.clip setClipEffect:effectId effectOption:options];
            } else if ( effectType == NXE_CLIP_TRANSITION ){
                [self.clip setTransitionEffect:effectId effectOption:options];
            }
        }];
    }
}

@end
