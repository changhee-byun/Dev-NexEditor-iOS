/******************************************************************************
 * File Name   :	SolidClipTableViewController.m
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

#import "SolidClipTableViewController.h"
#import "CommonTypeBTableViewCell.h"

@interface SolidClipTableViewController ()

@property (nonatomic, copy) void (^refreshProjectList)(NSString *);

@end

@implementation SolidClipTableViewController

- (void)refreshProjectListWithSolidCB:(refreshProjectList)refreshProjectList
{
    self.refreshProjectList = refreshProjectList;
}

- (void)viewDidLoad {
    [super viewDidLoad];
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
            
            return [UIColor colorWithRed:(i_red/255.0) green:(i_green/255.0) blue:(i_blue/255.0) alpha:1.0];
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
    
    CommonTypeBTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"SolidClipCell" forIndexPath:indexPath];
    
    NSString *colorNaming = [colorNames objectAtIndex:indexPath.row];
    
    cell.titleLabel.text = colorNaming;
    cell.imageView.image = [self creatImageWithColor:[self createColor:colorNaming]];
    cell.accessoryType = UITableViewCellAccessoryNone;
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSArray *colorValues = @[@"FF00FFFF", @"FF000000", @"FF0000FF", @"FFFF00FF", @"FF808080", @"FF008000", @"FF00FF00", @"FF800000", @"FF000080", @"FF808000", @"FFFFA500", @"FF800080", @"FFFF0000", @"FFC0C0C0", @"FF008080", @"FFFFFFFF", @"FFFFFF00"];

    
    self.refreshProjectList([colorValues objectAtIndex:indexPath.row]);
    
    [self.navigationController popViewControllerAnimated:YES];
}

@end
