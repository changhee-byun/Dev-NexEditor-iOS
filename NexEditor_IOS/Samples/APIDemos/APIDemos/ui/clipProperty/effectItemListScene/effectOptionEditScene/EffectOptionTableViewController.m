/**
 * File Name   : EffectOptionTableViewController.m
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

#import "EffectOptionTableViewController.h"
#import "CommonTypeCTableViewCell.h"
#import "EffectEditTypeBTableViewController.h"
#import "EffectEditTypeCViewController.h"

// for nexeditor sdk framework
@import NexEditorFramework;
#import "EffectOptionEditor.h"

typedef enum {
    SectionText = 0,
    SectionColor,
    NumSections
} SectionIndex;

static NSString * const headerTitles[] = { @"Text", @"Color" };

@interface EffectOptionTableViewController ()

@property (nonatomic, assign) int effectType;
@property (nonatomic, retain) NSString *effectId;
@property (nonatomic, retain) NXEEffectOptions *effectOptions;
@property (nonatomic, strong) EffectOptionsConfiguredBlock configuredBlock;
@property (nonatomic) BOOL optionsSetupSucceed;

@end

@implementation EffectOptionTableViewController

- (void)dealloc
{
    self.effectOptions = nil;
    self.configuredBlock = nil;
}

- (void)setupEffectOption:(NSString *)effectId
               effectType:(NXEEffectType)effectType
                 configured:(EffectOptionsConfiguredBlock) configured
{
    
    self.effectId = effectId;
    self.effectType = effectType; // 0: clip Effect, 1: transition Effect
    self.configuredBlock = configured;
    
    self.effectOptions = [[NXEEffectOptions alloc] init];
    self.optionsSetupSucceed = [self.effectOptions setupWithEffectId:self.effectId type:self.effectType];
    
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    //
    if([self.effectId containsString:@"com.nexstreaming."]) {
        self.title = [self.effectId substringFromIndex:[@"com.nexstreaming." length]];
    } else {
        self.title = self.effectId;
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Text
    // Color
    return NumSections;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    return headerTitles[section];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    NSInteger result = 0;
    if ( section == SectionText ) {
        result = self.effectOptions.textOptions.count;
    } else if ( section == SectionColor ) {
        result = self.effectOptions.colorOptions.count;
    }
    return result;
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"EffectOptionCell" forIndexPath:indexPath];
    
    if( indexPath.section == SectionText) {
        NXETextOption *textOption = self.effectOptions.textOptions[indexPath.row];
        
        cell.TableLabel.text = [NSString stringWithFormat:@"Text Option / %@",
                                textOption.key4textField];
        cell.ContentLabel.text = textOption.value4textField;
    } else if( indexPath.section == SectionColor) {
        NXEColorOption *colorOption = self.effectOptions.colorOptions[indexPath.row];
        cell.TableLabel.text = [NSString stringWithFormat:@"Color Option / %@",
                                colorOption.key4colorField];
        cell.ContentLabel.text = colorOption.value4colorField;
    }
    
    return cell;
}

#define TABLEVIEW_ROW_HEIGHT 60
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return TABLEVIEW_ROW_HEIGHT;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(indexPath.section == SectionText) {
        [self performSegueWithIdentifier:@"segueEffectOptionEditTypeC" sender:self];
    } else if(indexPath.section == SectionColor) {
        [self performSegueWithIdentifier:@"segueEffectOptionEditTypeB" sender:self];
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([segue identifier] == nil) {
        if ( self.configuredBlock ) {
            self.configuredBlock(self.effectOptions);
        }
    } else {
        id<EffectOptionEditor> optionEditor = nil;
        
        if([segue.identifier isEqualToString:@"segueEffectOptionEditTypeB"]
           || [segue.identifier isEqualToString:@"segueEffectOptionEditTypeC"]) {
            UIViewController *vc = [segue destinationViewController];
            if ( [vc conformsToProtocol:@protocol(EffectOptionEditor)]) {
                optionEditor = (id<EffectOptionEditor>) vc;
            } else {
                NSLog(@"ERROR: Unexpected segue '%@'. Implementation not complete?", segue.identifier);
            }
        }
        
        if ( optionEditor != nil ) {
            [optionEditor setupWithEffectOption:self.effectOptions
                                    selectedIdx:(int)self.tableView.indexPathForSelectedRow.row
                                     effectType:self.effectType
                                     configured:^{
                                         [self.tableView reloadData];
                                     }];
        }
    }
}

@end
