/******************************************************************************
 * File Name   :	TextLayerTableViewController.m
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

#import "TextLayerTableViewController.h"
#import "TextLayerPropertyTableViewController.h"
#import "TextInputViewController.h"
#import "CommonTypeCTableViewCell.h"
#import "PreviewViewController.h"
#import "Util.h"
#import "NSArray+NXELayer.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@implementation TextLayerTableViewController
{
    UIAlertController *alertController;
    
    NSArray *textLayerList;
    int positionId;
    int backgroundColorId;
    int shadowColorId;
    int glowColorId;
    int outlineColorId;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    textLayerList = [self.layers layersWithType:NXE_LAYER_TEXT];
}

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if ( parent == nil ) {
        [self informDismissDone:NO];
    }
}

#pragma mark -

/// Merge video layer list change into self.layers and call back to the calling view controller
- (void) informDismissDone:(BOOL) done
{
    self.layers = [self.layers mergingLayers:textLayerList type:NXE_LAYER_TEXT];
    if (self.onDismiss) {
        self.onDismiss(self, done);
        self.onDismiss = nil;
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [textLayerList count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 165;
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"TextLayerCell" forIndexPath:indexPath];
    
    NXETextLayer *textLayer = [textLayerList objectAtIndex:indexPath.row];
    
    if(textLayer.x == 0) {
        if(textLayer.y == 0) {
            positionId = 0;
        } else {
            positionId = 2;
        }
    } else if(textLayer.x == 640) {
        if(textLayer.y == 0) {
            positionId = 1;
        } else {
            positionId = 3;
        }
    } else {
        positionId = 4;
    }
    
    NSString *contentText = nil;
    contentText = [NSString stringWithFormat:@"StartTime:%@, EndTime:%@,\nFont(%@, %d), Alpha(Opacity):%d,\nBackground(%@, %@), Shadow(%@, %@),\nGlow(%@, %@), Outline(%@, %@),\nIn Animation(%@), Out Animation(%@),\nIn Animation Duration(%@), Out Animation Duration(%@),\nOverall Animation(%@)",
                   [StringTools formatTimeMs:textLayer.startTime],
                   [StringTools formatTimeMs:textLayer.endTime],
                   [textLayer getFont].fontName,
                   (int)([textLayer getFont].pointSize),
                   (int)(100 * textLayer.alpha),
                   textLayer.textLayerProperty.useBackground ? @"YES" : @"NO",
                   [self getFontColor:backgroundColorId],
                   textLayer.textLayerProperty.useShadow ? @"YES" : @"NO",
                   [self getFontColor:shadowColorId],
                   textLayer.textLayerProperty.useGlow ? @"YES" : @"NO",
                   [self getFontColor:glowColorId],
                   textLayer.textLayerProperty.useOutline ? @"YES" : @"NO",
                   [self getFontColor:outlineColorId],
                   [self getAnimation:textLayer.inAnimation type:0],
                   [self getAnimation:textLayer.outAnimation type:1],
                   [StringTools formatTimeMs:textLayer.inAnimationDuration],
                   [StringTools formatTimeMs:textLayer.outAnimationDuration],
                   [self getExpression:textLayer.expression]];
    
    NSArray *array = [[textLayer getText] componentsSeparatedByString:@"\n"];
    if([array count] > 1) {
        cell.TableLabel.text = [NSString stringWithFormat:@"%@ ...", array[0]];
    } else {
        cell.TableLabel.text = array[0];
    }
    cell.ContentLabel.text = contentText;
    
    return cell;
}

- (NSString *)getPosition:(long)index
{
    NSArray *positionList = @[@"LEFT:0, TOP:0", @"LEFT:640, TOP:0", @"LEFT:0, TOP:360", @"LEFT:640, TOP:360", @"Others"];
    return [positionList objectAtIndex:positionId];
}

- (NSString *)getAnimation:(int)index type:(int)type
{
    NSArray *animationList = @[@"None", @"Fade", @"Pop", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down", @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"];
    if(type == 1) {
        animationList = @[@"None", @"Fade", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down", @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"];
    }
    return [animationList objectAtIndex:index];
}

- (NSString *)getExpression:(int)index
{
    NSArray *expressionList = @[@"None", @"Blink Slow", @"Flicker", @"Pulse", @"Jitter", @"Fountain", @"Ring", @"Floating", @"Drifting", @"Squishing"];
    return [expressionList objectAtIndex:index];
}

- (NSString *)getFontColor:(int)index
{
    NSArray *fontColorList = @[@"Black", @"White", @"Blue", @"Red", @"Yellow", @"Lime", @"Magenta", @"Orange", @"Grey", @"Cyan"];
    return [fontColorList objectAtIndex:index];
}

- (NSString *)getFontNaming:(int)index
{
    NSArray *fontList = @[@"System default", @"bevan", @"creepster-regular", @"justanotherhand-regular", @"lato-bold", @"knewave", @"releway-thin", @"leaguegothic", @"redressed", @"junctionregular", @"goudystm-italic",@"greatvibes-regular",@"Tangerine_Regular"];
    return [fontList objectAtIndex:index];
}

#pragma mark - Listener

- (IBAction)doDeleteCell:(id)sender
{
    if(((UILongPressGestureRecognizer *)sender).state == UIGestureRecognizerStateBegan) {
        CGPoint p = [((UILongPressGestureRecognizer *)sender) locationInView:self.tableView];
        NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];
        //
        __block NXETextLayer *textLayer = [textLayerList objectAtIndex:indexPath.row];
        NSString *track_name = [textLayer getText];
        
        alertController = [UIAlertController alertControllerWithTitle:@"Information"
                                                              message:[NSString stringWithFormat:@"Delete text layer:\n%@.", track_name]
                                                       preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
            alertController = nil;
                        
            textLayerList = [textLayerList removingObject:textLayer];
            
            [self.tableView reloadData];
        }];
        UIAlertAction *nok = [UIAlertAction actionWithTitle:@"NO" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
            alertController = nil;
        }];
        
        [alertController addAction:ok];
        [alertController addAction:nok];
        
        [self presentViewController:alertController animated:YES completion:nil];
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([segue identifier] == nil) {
        
        [self informDismissDone:YES];
        
    } else if([[segue identifier] isEqualToString:@"segueAddTextLayer"]) {
        TextInputViewController *tiv = [segue destinationViewController];
        [tiv setupTextLayer4InputCB:^(NSString *text) {
            positionId = 4;
            
            // TextLayer 생성 후, layer manageList에 등록해준다.
            NXETextLayer *textLayer = [[NXETextLayer alloc] initWithText:text font:nil point:CGPointMake(-1, -1)];
            textLayerList = [textLayerList arrayByAddingObject:textLayer];
            
            [self.tableView reloadData];
        } textLayerTitle:nil];
    } else if([[segue identifier] isEqualToString:@"segueTextLayerProperty"]) {
        TextLayerPropertyTableViewController *tptv = [segue destinationViewController];
        
        NXETextLayer *layer = textLayerList[self.tableView.indexPathForSelectedRow.row];
        [tptv setupProperty:layer postion:positionId refreshListItem:^(int postion) {
            positionId = postion;
            [self.tableView reloadData];
        }];
    }
}

@end
