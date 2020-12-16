//
//  TextLayerTableViewController.h
//  APIDemos
//
//  Created by Sunghyun Yoo on 2016. 11. 11..
//  Copyright © 2016년 NexStreaming. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "LayerListEditor.h"

@interface TextLayerTableViewController : UITableViewController <LayerListEditor>
@property (nonatomic, copy) NSArray<NXELayer *> *layers;
@property (nonatomic, strong) LayerListEditorDismissed onDismiss;
@end
