/**
 * File Name   : AssetXMLParser.m
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
#import "AssetXMLParser.h"

@interface AssetXMLParser() <NSXMLParserDelegate>
@property (nonatomic, assign) id<AssetXMLParserClient> client;
@end

@implementation AssetXMLParser

- (void)parserDidStartDocument:(NSXMLParser *)parser
{
}

- (void)parser:(NSXMLParser *)parser didStartElement:(nonnull NSString *)elementName
  namespaceURI:(nullable NSString *)namespaceURI
 qualifiedName:(nullable NSString *)qName
    attributes:(nonnull NSDictionary<NSString *,NSString *> *)attributeDict
{
    [self.client parseBeginElementWithName:elementName
                                attributes:attributeDict];
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(nonnull NSString *)string
{
    if ([self.client respondsToSelector:@selector(parseFoundCharacters:)]) {
        [self.client parseFoundCharacters:string];
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(nonnull NSString *)elementName
  namespaceURI:(nullable NSString *)namespaceURI
 qualifiedName:(nullable NSString *)qName
{
    if ( [self.client respondsToSelector:@selector(parseEndElementWithName:)]) {
        [self.client parseEndElementWithName:elementName];
    }
}

- (void)parserDidEndDocument:(NSXMLParser *)parser
{

}

- (void) parseWithData:(NSData *) data client:(id<AssetXMLParserClient> _Nonnull) client
{
    self.client = client;
    NSXMLParser *xmlParser = [[NSXMLParser alloc] initWithData:data];
    xmlParser.delegate = self;
    [[xmlParser autorelease] parse];
    self.client = nil;
}

+ (instancetype _Nonnull) parser
{
    return [[[self.class alloc] init] autorelease];
}
@end


@interface AssetXMLParserClientChain()
@property (nonatomic, strong) NSArray<id<AssetXMLParserClient>> *clients;
@end

@implementation AssetXMLParserClientChain
- (instancetype) initWithClients:(NSArray<id<AssetXMLParserClient>> *) clients
{
    self = [super init];
    if ( self ) {
        self.clients = clients;
    }
    return self;
}

- (void) dealloc
{
    self.clients = nil;
    [super dealloc];
}

- (void)parseBeginElementWithName:(NSString * )elementName attributes:(NSDictionary<NSString *,NSString *> * )attributes
{
    for (id<AssetXMLParserClient> client in self.clients) {
        [client parseBeginElementWithName:elementName attributes:attributes];
    }
}

- (void)parseEndElementWithName:(NSString * )elementName
{
    for (id<AssetXMLParserClient> client in self.clients) {

        if ( [client respondsToSelector:@selector(parseEndElementWithName:)]) {
            [client parseEndElementWithName:elementName];
        }
    }
}
- (void)parseFoundCharacters:(NSString * )string
{
    for (id<AssetXMLParserClient> client in self.clients) {
        if ([client respondsToSelector:@selector(parseFoundCharacters:)]) {
            [client parseFoundCharacters:string];
        }
    }
}

+ (instancetype ) chainWithClients:(NSArray<id<AssetXMLParserClient>> *) clients
{
    return [[[self.class alloc] initWithClients: clients] autorelease];
}

@end
