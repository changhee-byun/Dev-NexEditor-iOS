/**
 * File Name   : AssetXMLParser.h
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

#import <Foundation/Foundation.h>

@protocol AssetXMLParserClient <NSObject>
- (void)parseBeginElementWithName:(NSString * _Nonnull)elementName attributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributeDict;
@optional
- (void)parseEndElementWithName:(NSString * _Nonnull)elementName;
- (void)parseFoundCharacters:(NSString * _Nonnull)string;
@end

@interface AssetXMLParser: NSObject
- (void) parseWithData:(NSData * _Nonnull) data client:(id<AssetXMLParserClient> _Nonnull) client;
+ (instancetype _Nonnull) parser;

@end

@interface AssetXMLParserClientChain: NSObject <AssetXMLParserClient>
+ (instancetype _Nonnull) chainWithClients:(NSArray<id<AssetXMLParserClient>> *_Nonnull) clients;
@end
