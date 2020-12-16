/*
 * File Name   : LUTNames.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "LUTNames.h"

@interface LUTNames()
+ (NSString *) nameForIndex:(NSUInteger) index;
@end

@implementation LUTNames

+ (NSArray<NSString *> *) names
{
    return @[@"None",
             @"Afternoon",
             @"Almond Blossom",
             @"Autumn",
             @"Boring",
             @"Caramel Candy",
             @"Caribbean",
             @"Cinnamon",
             @"Cloud",
             @"Coral Candy",
             @"Cranberry",
             @"Daisy",
             @"Dawn",
             @"Disney",
             @"England",
             @"Espresso",
             @"Eye Shadow",
             @"Gloomy",
             @"Jazz",
             @"Lavendar",
             @"Moonlight",
             @"Newspaper",
             @"Paris",
             @"Peach",
             @"Rainy",
             @"Raspberry",
             @"Retro",
             @"Sherbert",
             @"Shiny",
             @"Smoke",
             @"Stone Edge",
             @"Sun Rising",
             @"Symphony Blue",
             @"Tangerine",
             @"Tiffany",
             @"Vintage Flower",
             @"Romance",
             @"Vivid",
             @"Warm"];
}

+ (NSString *) nameForIndex:(NSUInteger) index
{
    NSString *result = nil;
    if (index < self.names.count) {
        result = self.names[index];
    }
    return result;
}

+ (NSString *) nameForLutId:(NXELutID) lutId
{
    NSString *name;
    if (lutId == kLutIdNotFound) {
        lutId = 0;
    }
    if (lutId < LUTNames.names.count) {
        name = [LUTNames nameForIndex:lutId];
    } else {
        name = @"Custom";
    }
    return name;
}
@end
