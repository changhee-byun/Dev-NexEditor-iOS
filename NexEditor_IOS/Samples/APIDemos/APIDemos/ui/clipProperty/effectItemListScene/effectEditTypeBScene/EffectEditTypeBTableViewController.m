/******************************************************************************
 * File Name   :	EffectEditTypeBTableViewController.h
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

#import "EffectEditTypeBTableViewController.h"
#import "CommonTypeBTableViewCell.h"
@import NexEditorFramework;

@interface EffectEditTypeBTableViewController ()

@property (copy, nonatomic) EffectOptionConfiguredBlock configuredBlock;
@property (assign, nonatomic) NXEEffectType effectType;
@property (assign, nonatomic) int selectedIdx;
@property (retain, nonatomic) NXEEffectOptions *effectOption;
@property (retain, nonatomic) NSMutableDictionary *colorPallet;

@end

@implementation EffectEditTypeBTableViewController

- (void)setupWithEffectOption:(NXEEffectOptions *) effectOption
                  selectedIdx:(int) selectedIdx
                   effectType:(NXEEffectType) effectType
                   configured:(EffectOptionConfiguredBlock) configured
{
    self.effectOption = effectOption;
    self.selectedIdx = selectedIdx;
    self.effectType = effectType; // 0: clip effect, 1: transition effect
    self.configuredBlock = configured;
    self.colorPallet = [NSMutableDictionary dictionaryWithDictionary:@{
                                                                       @"Aqua" : @"#00FFFF",
                                                                       @"Black" : @"#000000",
                                                                       @"Blue" : @"#0000FF",
                                                                       @"Fuchsia" : @"#FF00FF",
                                                                       @"Gray" : @"#808080",
                                                                       @"Green" : @"#008000",
                                                                       @"Lime" : @"#00FF00",
                                                                       @"Maroon" : @"#800000",
                                                                       @"Navy" : @"#000080",
                                                                       @"Olive" : @"#808000",
                                                                       @"Orange" : @"#FFA500",
                                                                       @"Purple" : @"#800080",
                                                                       @"Red" : @"#FF0000",
                                                                       @"Silver" : @"#C0C0C0",
                                                                       @"Teal" : @"#008080",
                                                                       @"White" : @"#FFFFFF",
                                                                       @"Yellow" : @"#FFFF00",
                                                                       }];
    BOOL isFindValue = NO;
    
    NSString *curValue = ((NXEColorOption *)self.effectOption.colorOptions[self.selectedIdx]).value4colorField;
    
    for(NSString *value in self.colorPallet.allValues) {
        if([curValue isEqualToString:value]) {
            isFindValue = YES;
            break;
        }
    }
    if(isFindValue == NO) {
        [self.colorPallet setObject:curValue forKey:@"Custom"];
    }
}

- (unsigned long long)convertHexLongFromString:(NSString *)stringValue
{
    unsigned long long value;
    [[NSScanner scannerWithString:stringValue] scanHexLongLong:&value];
    return value;
}

- (CGImageRef)creatImageWithColor:(UIColor *)color
{
    CGFloat scale = [UIScreen mainScreen].scale;
    CGRect rect = CGRectMake(0, 0, 160, 120);
    
    UIGraphicsBeginImageContextWithOptions(rect.size, YES, scale);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, color.CGColor);
    CGContextFillRect(context, rect);
    
    CGImageRef imageRef_ = UIGraphicsGetImageFromCurrentImageContext().CGImage;
    
    UIGraphicsEndImageContext();
    
    return imageRef_;
}

- (UIImage *)createImage:(int)listIdx
{
    if([[self getValue:listIdx] hasPrefix:@"#"]) {
        NSString *color = [[self getValue:listIdx] substringWithRange:NSMakeRange(1, 6)];
        NSString *red = [color substringWithRange:NSMakeRange(0,2)];
        NSString *green = [color substringWithRange:NSMakeRange(2,2)];
        NSString *blue = [color substringWithRange:NSMakeRange(4, 2)];
    
        int i_red = (int)[self convertHexLongFromString:red];
        int i_green = (int)[self convertHexLongFromString:green];
        int i_blue = (int)[self convertHexLongFromString:blue];
        
        UIColor *colorinfo = [UIColor colorWithRed:(i_red/255.0) green:(i_green/255.0) blue:(i_blue/255.0) alpha:1];
        CGImageRef imageRef = [self creatImageWithColor:colorinfo];
        
        return [UIImage imageWithCGImage:imageRef];
    } else {
        NSArray *colors = [[self getValue:listIdx] componentsSeparatedByString:@" "];
        
        float red = [[colors objectAtIndex:0] floatValue];
        float green = [[colors objectAtIndex:1] floatValue];
        float blue = [[colors objectAtIndex:2] floatValue];
        
        UIColor *colorinfo = [UIColor colorWithRed:red green:green blue:blue alpha:1];
        CGImageRef imageRef = [self creatImageWithColor:colorinfo];
        
        return [UIImage imageWithCGImage:imageRef];
    }
}

- (NSString *)getValue:(int)listIdx
{
    return [[self.colorPallet allValues] objectAtIndex:listIdx];
}

- (NSString *)getKey:(int)listIdx
{
    return [[self.colorPallet allKeys] objectAtIndex:listIdx];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = ((NXEColorOption *)self.effectOption.colorOptions[self.selectedIdx]).key4colorField;
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.colorPallet count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    return 60;
}

- (CommonTypeBTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    CommonTypeBTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"EffectypeBCell" forIndexPath:indexPath];
    
    cell.imageView.image = [self createImage:(int)indexPath.row];
    cell.titleLabel.text = [self getKey:(int)indexPath.row];
    
    NSString *curValue = nil;
    curValue = ((NXEColorOption *)self.effectOption.colorOptions[self.selectedIdx]).value4colorField;
    
    if([curValue isEqualToString:[self getValue:(int)indexPath.row]]) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    ((NXEColorOption *)self.effectOption.colorOptions[self.selectedIdx]).value4colorField = [self getValue:(int)indexPath.row];
    
    [tableView cellForRowAtIndexPath:indexPath].accessoryType = UITableViewCellAccessoryCheckmark;
    
    self.configuredBlock();
    [self.navigationController popViewControllerAnimated:YES];
}

@end
