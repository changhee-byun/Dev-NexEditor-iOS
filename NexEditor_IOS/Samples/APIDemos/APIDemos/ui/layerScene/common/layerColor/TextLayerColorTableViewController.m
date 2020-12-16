/******************************************************************************
 * File Name   :	TextLayerColorTableViewController.m
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

#import "TextLayerColorTableViewController.h"
#import "CommonTypeBTableViewCell.h"

@interface TextLayerColorTableViewController ()

@property (nonatomic, copy) void (^refreshTextColor)(UIColor *);

@end

@implementation TextLayerColorTableViewController
{
    UIColor *fontColor;
    CGFloat fontColorAlpha;
}

- (void)setupTextLayerColorWithCB:(refreshTextColor)refreshTextColor selectedTextColor:(UIColor *)textFontColor selectedAlpah:(CGFloat)alpha
{
    self.refreshTextColor = refreshTextColor;
    fontColor = textFontColor;
    fontColorAlpha = alpha;
}

- (NSString *)getColorVia:(UIColor *)color
{
    CGFloat red; CGFloat green; CGFloat blue;
    [color getRed:&red green:&green blue:&blue alpha:nil];
    
    NSString *red_ = [NSString stringWithFormat:@"%x", (int)(red*255.0)];
    NSString *green_ = [NSString stringWithFormat:@"%x", (int)(green*255.0)];
    NSString *blue_ = [NSString stringWithFormat:@"%x", (int)(blue*255.0)];
    
    if([red_ isEqualToString:@"0"]) red_ = @"00";
    if([green_ isEqualToString:@"0"]) green_ = @"00";
    if([blue_ isEqualToString:@"0"]) blue_ = @"00";
    
    return [NSString stringWithFormat:@"#%@%@%@", red_, green_, blue_];
}

- (unsigned long long)convertHexLongFromString:(NSString *)stringValue
{
    unsigned long long value;
    [[NSScanner scannerWithString:stringValue] scanHexLongLong:&value];
    return value;
}

- (UIColor *)createColor:(NSString *)colorNaming
{
    NSDictionary *colors = @{
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
                             };
    
    for(id colorNaming_ in colors) {
        if([colorNaming isEqualToString:colorNaming_]) {
            NSString *color = [colors[colorNaming] substringWithRange:NSMakeRange(1, 6)];
            NSString *red = [color substringWithRange:NSMakeRange(0,2)];
            NSString *green = [color substringWithRange:NSMakeRange(2,2)];
            NSString *blue = [color substringWithRange:NSMakeRange(4, 2)];
            
            int i_red = (int)[self convertHexLongFromString:red];
            int i_green = (int)[self convertHexLongFromString:green];
            int i_blue = (int)[self convertHexLongFromString:blue];
            
            return [UIColor colorWithRed:(i_red/255.0) green:(i_green/255.0) blue:(i_blue/255.0) alpha:fontColorAlpha];
        }
    }
    
    return nil;
}

- (UIImage *)creatImageWithColor:(UIColor *)color
{
    CGFloat scale = [UIScreen mainScreen].scale;
    CGRect rect = CGRectMake(0, 0, 160, 120);
    
    UIGraphicsBeginImageContextWithOptions(rect.size, YES, scale);
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, color.CGColor);
    CGContextFillRect(context, rect);
    
    CGImageRef imageRef = UIGraphicsGetImageFromCurrentImageContext().CGImage;
    
    UIGraphicsEndImageContext();
    
    return [UIImage imageWithCGImage:imageRef];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 17;
}

- (CommonTypeBTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    NSArray *colorNames = @[@"Aqua", @"Black", @"Blue", @"Fuchsia", @"Gray", @"Green", @"Lime", @"Maroon", @"Navy", @"Olive", @"Orange", @"Purple", @"Red", @"Silver", @"Teal", @"White", @"Yellow"];
    
    NSArray *colorValues = @[@"#00FFFF", @"#000000", @"#0000FF", @"#FF00FF", @"#808080", @"#008000", @"#00FF00", @"#800000", @"#000080", @"#808000", @"#FFA500", @"#800080", @"#FF0000", @"#C0C0C0", @"#008080", @"#FFFFFF", @"#FFFF00"];
    
    CommonTypeBTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"TextFontColorCell" forIndexPath:indexPath];
    
    NSString *colorNaming = [colorNames objectAtIndex:indexPath.row];
    
    cell.titleLabel.text = colorNaming;
    cell.imageView.image = [self creatImageWithColor:[self createColor:colorNaming]];
    if([[self getColorVia:fontColor] compare:[colorValues objectAtIndex:indexPath.row] options:NSCaseInsensitiveSearch] == NSOrderedSame) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSArray *colorNames = @[@"Aqua", @"Black", @"Blue", @"Fuchsia", @"Gray", @"Green", @"Lime", @"Maroon", @"Navy", @"Olive", @"Orange", @"Purple", @"Red", @"Silver", @"Teal", @"White", @"Yellow"];
    
    UIColor *color = [self createColor:[colorNames objectAtIndex:indexPath.row]];
    
    self.refreshTextColor(color);
    
    [self.navigationController popViewControllerAnimated:YES];
}

@end
