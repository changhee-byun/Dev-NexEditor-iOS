/******************************************************************************
 * File Name   :	TextLayerPropertyTableViewController.m
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

#import "TextLayerPropertyTableViewController.h"
#import "TextLayerColorTableViewController.h"
#import "TextLayerFontTableViewController.h"
#import "TextInputViewController.h"
#import "LayerAnimationTableViewController.h"
#import "LayerExpressionTableViewController.h"
#import "AudioChangerTableViewController.h"
#import "Util.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@interface TextLayerPropertyTableViewController ()

@property (nonatomic, copy) void(^refreshTextLayerTableView)(int);

@end

@implementation TextLayerPropertyTableViewController
{
    NXETextLayer *textLayer;
    UIButton *focusedButton;
    CGFloat focusedAlpha;
    int positionLayer;
    int positionInAnimation;
    int positionExpression;
    int positionOutAnimation;
    int positionFont;
}

- (void)setupProperty:(NXETextLayer *)layer postion:(int)position refreshListItem:(refreshListItem)refreshListItem
{
    textLayer = layer;
    positionLayer = position;
    positionInAnimation = textLayer.inAnimation;
    positionExpression = textLayer.expression;
    positionOutAnimation = textLayer.outAnimation;
    self.refreshTextLayerTableView = refreshListItem;
}

- (NSString *)getPosition
{
    NSArray *positionList = @[@"LEFT:0, TOP:0", @"LEFT:640, TOP:0", @"LEFT:0, TOP:360", @"LEFT:640, TOP:360", @"Others"];
    return [positionList objectAtIndex:positionLayer];
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

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Inputed Text
    NSArray *array = [[textLayer getText] componentsSeparatedByString:@"\n"];
    if([array count] > 1) {
        self.editTextLabel.text = [NSString stringWithFormat:@"%@ ...", array[0]];
    } else {
        self.editTextLabel.text = array[0];
    }
    
    //
    self.kerningSlider.minimumValue = -50.;
    self.kerningSlider.maximumValue = 50.;
    self.kerningSlider.value = textLayer.textLayerProperty.kerningRatio;
    self.kerningLabel.text = [NSString stringWithFormat:@"%d", (int)self.kerningSlider.value];
    
    self.lineSpaceSlider.minimumValue = -50.;
    self.lineSpaceSlider.maximumValue = 50.;
    self.lineSpaceSlider.value = textLayer.textLayerProperty.spaceBetweenLines;
    self.lineSpaceLabel.text = [NSString stringWithFormat:@"%d", (int)self.lineSpaceSlider.value];
    
    self.horizontalAlignSegment.selectedSegmentIndex = textLayer.textLayerProperty.horizontalAlign;
    self.verticalAlignSegment.selectedSegmentIndex = textLayer.textLayerProperty.verticalAlign;

    if(textLayer.textLayerProperty.useUnderline) {
        self.underlineSwitch.on = YES;
    } else {
        self.underlineSwitch.on = NO;
    }
    //
    
    // In Animation
    self.inAnimationLabel.text = [self getAnimation:positionInAnimation type:0];
    self.inAnimationSlider.minimumValue = 0;
    self.inAnimationSlider.maximumValue = 5*1000;
    self.inAnimationSlider.value = textLayer.inAnimationDuration;
    self.inAnimationTimeLabel.text = [StringTools formatTimeMs:textLayer.inAnimationDuration];
    
    // Expression
    self.expressionLabel.text = [self getExpression:positionExpression];
    
    // Out Animation
    self.outAnimationLabel.text = [self getAnimation:positionOutAnimation type:1];
    self.outAnimationSlider.minimumValue = 0;
    self.outAnimationSlider.maximumValue = 5*1000;
    self.outAnimationSlider.value = textLayer.outAnimationDuration;
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:textLayer.outAnimationDuration];
    
    NXEEngine *engine = [NXEEngine instance];
    // Start Time
    self.startTimeLabel.text = [StringTools formatTimeMs:textLayer.startTime];
    self.startTimeSlider.minimumValue = 0;
    self.startTimeSlider.maximumValue = [engine.project getTotalTime];
    self.startTimeSlider.value = textLayer.startTime;
    
    // End Time
    self.endTimeLabel.text = [StringTools formatTimeMs:textLayer.endTime];
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue+[engine.project getTotalTime];
    self.endTimeSlider.value = textLayer.endTime;
    
    // Alpha
    self.alphaLabel.text = [NSString stringWithFormat:@"%d", (int)(100 * textLayer.alpha)];
    self.alphaSlider.minimumValue = 0;
    self.alphaSlider.maximumValue = 1;
    self.alphaSlider.value = textLayer.alpha;
    
    // Font Size
    self.fontSizeLabel.text = [NSString stringWithFormat:@"%d", (int)textLayer.getFont.pointSize];
    self.fontSizeSlider.value = [textLayer getFont].pointSize;
    self.fontSizeSlider.minimumValue = 46;
    self.fontSizeSlider.maximumValue = 360;
    
    // Text Font
    
    // i.   Text Font Naming
    if([[textLayer getFont].fontName isEqualToString:@".SFUIDisplay"]) {
        self.fontNamingLabel.text = @"system default";
    } else {
        self.fontNamingLabel.text = [textLayer getFont].fontName;
    }
    
    // ii.  Text Font Color
    self.fontColorButton.backgroundColor = textLayer.textLayerProperty.textColor;
    self.fontColorButton.layer.borderWidth = 1.0f;
    
    // Background Color
    
    // i.   Background Switch
    if(textLayer.textLayerProperty.useBackground) {
        self.bgColorSwitch.on = YES;
    } else {
        self.bgColorSwitch.on = NO;
    }
    
    // ii.  Background Color
    self.bgColorButton.backgroundColor = textLayer.textLayerProperty.bgColor;
    self.bgColorButton.layer.borderWidth = 1.0f;
    
    // Glow Color
    
    // i.   Glow Switch
    if(textLayer.textLayerProperty.useGlow) {
        self.glowColorSwitch.on = YES;
    } else {
        self.glowColorSwitch.on = NO;
    }
    
    // ii.  Glow Color
    self.glowColorButton.backgroundColor = textLayer.textLayerProperty.glowColor;
    self.glowColorButton.layer.borderWidth = 1.0f;

    self.glowSpreadSlider.minimumValue = 0.;
    self.glowSpreadSlider.maximumValue = 100.;
    self.glowSpreadSlider.value = textLayer.textLayerProperty.glowSpread;
    self.glowSpreadLabel.text = [NSString stringWithFormat:@"%d", (int)self.glowSpreadSlider.value];

    self.glowSizeSlider.minimumValue = 0.;
    self.glowSizeSlider.maximumValue = 100.;
    self.glowSizeSlider.value = textLayer.textLayerProperty.glowSize;
    self.glowSizeLabel.text = [NSString stringWithFormat:@"%d", (int)self.glowSizeSlider.value];

    
    // Shadow Color
    
    // i.   Shadow Switch
    if(textLayer.textLayerProperty.useShadow) {
        self.shadowColorSwitch.on = YES;
    } else {
        self.shadowColorSwitch.on = NO;
    }
    
    // ii.  Shadow Color
    self.shadowColorButton.backgroundColor = textLayer.textLayerProperty.shadowColor;
    self.shadowColorButton.layer.borderWidth = 1.0f;
    
    self.shadowAngleSlider.minimumValue = 0.;
    self.shadowAngleSlider.maximumValue = 360.;
    self.shadowAngleSlider.value = textLayer.textLayerProperty.shadowAngle;
    self.shadowAngleLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowAngleSlider.value];

    self.shadowDistanceSlider.minimumValue = 0.;
    self.shadowDistanceSlider.maximumValue = 50.;
    self.shadowDistanceSlider.value = textLayer.textLayerProperty.shadowDistance;
    self.shadowDistanceLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowDistanceSlider.value];

    self.shadowSpreadSlider.minimumValue = 0.;
    self.shadowSpreadSlider.maximumValue = 100.;
    self.shadowSpreadSlider.value = textLayer.textLayerProperty.shadowSpread;
    self.shadowSpreadLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowSpreadSlider.value];

    self.shadowSizeSlider.minimumValue = 0.;
    self.shadowSizeSlider.maximumValue = 100.;
    self.shadowSizeSlider.value = textLayer.textLayerProperty.shadowSize;
    self.shadowSizeLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowSizeSlider.value];

    // Outline Color
    
    // i.   Outline Switch
    if(textLayer.textLayerProperty.useOutline) {
        self.outlineColorSwitch.on = YES;
    } else {
        self.outlineColorSwitch.on = NO;
    }
    
    // ii.  Outline Color
    self.outlineColorButton.backgroundColor = textLayer.textLayerProperty.outlineColor;
    self.outlineColorButton.layer.borderWidth = 1.0f;

    self.outlineThicknessSlider.minimumValue = 0.;
    self.outlineThicknessSlider.maximumValue = 50.;
    self.outlineThicknessSlider.value = textLayer.textLayerProperty.outlineThickness;
    self.outlineThicknessLabel.text = [NSString stringWithFormat:@"%d", (int)self.outlineThicknessSlider.value];
}


#pragma mark - Listener

- (IBAction)kerningRatioValueChanged:(id)sender
{
    self.kerningLabel.text = [NSString stringWithFormat:@"%d", (int)self.kerningSlider.value];
}

- (IBAction)lineSpaceValueChanged:(id)sender
{
    self.lineSpaceLabel.text = [NSString stringWithFormat:@"%d", (int)self.lineSpaceSlider.value];
}

- (IBAction)horizontalAlignValueChanged:(id)sender
{
    //self.lineSpaceLabel.text = [NSString stringWithFormat:@"%d", (int)self.lineSpaceSlider.value];
}

- (IBAction)doFontColor:(id)sender
{
    focusedButton = self.fontColorButton;
    focusedAlpha = 1.0f;
    [self performSegueWithIdentifier:@"segueTextLayerColor" sender:self];
}

- (IBAction)doBGColor:(id)sender
{
    focusedButton = self.bgColorButton;
    focusedAlpha = 0.53;
    [self performSegueWithIdentifier:@"segueTextLayerColor" sender:self];
}

- (IBAction)doGlowColor:(id)sender
{
    focusedButton = self.glowColorButton;
    focusedAlpha = 0.63;
    [self performSegueWithIdentifier:@"segueTextLayerColor" sender:self];
}

- (IBAction)doShadowColor:(id)sender
{
    focusedButton = self.shadowColorButton;
    focusedAlpha = 1.0f;
    [self performSegueWithIdentifier:@"segueTextLayerColor" sender:self];
}

- (IBAction)doOutlineColor:(id)sender
{
    focusedButton = self.outlineColorButton;
    focusedAlpha = 1.0f;
    [self performSegueWithIdentifier:@"segueTextLayerColor" sender:self];
}

- (IBAction)startTimeValueChanged:(id)sender
{
    self.startTimeLabel.text = [StringTools formatTimeMs:self.startTimeSlider.value];
    //
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue+[[NXEEngine instance].project getTotalTime];
    self.endTimeSlider.value = self.endTimeSlider.minimumValue;
    self.endTimeLabel.text = [StringTools formatTimeMs:self.endTimeSlider.minimumValue];
}

- (IBAction)endTimeValueChanged:(id)sender
{
    self.endTimeLabel.text = [StringTools formatTimeMs:self.endTimeSlider.value];
}

- (IBAction)alphaValueChanged:(id)sender
{
    self.alphaLabel.text = [NSString stringWithFormat:@"%d", (int)(100 * self.alphaSlider.value)];
}

- (IBAction)fontSizeValueChanged:(id)sender
{
    self.fontSizeLabel.text = [NSString stringWithFormat:@"%d", (int)self.fontSizeSlider.value];
}

- (IBAction)inAnimationValueChanged:(id)sender
{
    self.inAnimationTimeLabel.text = [StringTools formatTimeMs:self.inAnimationSlider.value];
}

- (IBAction)outAnimationValueChanged:(id)sender
{
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:self.outAnimationSlider.value];
}

- (IBAction)shadowAngleValueChanged:(id)sender
{
    self.shadowAngleLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowAngleSlider.value];
}

- (IBAction)shadowDistanceValueChanged:(id)sender
{
    self.shadowDistanceLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowDistanceSlider.value];
}

- (IBAction)shadowSpreadValueChanged:(id)sender
{
    self.shadowSpreadLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowSpreadSlider.value];
}

- (IBAction)shadowSizeValueChanged:(id)sender
{
    self.shadowSizeLabel.text = [NSString stringWithFormat:@"%d", (int)self.shadowSizeSlider.value];
}

- (IBAction)outlineThicknessValueChanged:(id)sender
{
    self.outlineThicknessLabel.text = [NSString stringWithFormat:@"%d", (int)self.outlineThicknessSlider.value];
}

- (IBAction)glowSpreadValueChanged:(id)sender
{
    self.glowSpreadLabel.text = [NSString stringWithFormat:@"%d", (int)self.glowSpreadSlider.value];
}

- (IBAction)glowSizeValueChanged:(id)sender
{
    self.glowSizeLabel.text = [NSString stringWithFormat:@"%d", (int)self.glowSizeSlider.value];
}

- (IBAction)doNext:(id)sender
{
    textLayer.inAnimation = (NXEInAnimationType)positionInAnimation;
    textLayer.expression = (NXEExpressionType)positionExpression;
    textLayer.outAnimation = (NXEOutAnimationType)positionOutAnimation;
    textLayer.alpha = self.alphaSlider.value;

    [textLayer setStartTime:self.startTimeSlider.value endTime:self.endTimeSlider.value];
    [textLayer setInAnimationType:textLayer.inAnimation duration:self.inAnimationSlider.value];
    [textLayer setOutAnimationType:textLayer.outAnimation duration:self.outAnimationSlider.value];
    [textLayer setOverallAnimation:textLayer.expression];
    
    textLayer.textLayerProperty.kerningRatio = self.kerningSlider.value;
    textLayer.textLayerProperty.spaceBetweenLines = self.lineSpaceSlider.value;
    textLayer.textLayerProperty.horizontalAlign = self.horizontalAlignSegment.selectedSegmentIndex == 2 ? NSTextAlignmentRight : (self.horizontalAlignSegment.selectedSegmentIndex  == 1 ? NSTextAlignmentCenter:NSTextAlignmentLeft) ;
    textLayer.textLayerProperty.verticalAlign = self.verticalAlignSegment.selectedSegmentIndex == 2 ? NSTextAlignmentRight : (self.verticalAlignSegment.selectedSegmentIndex  == 1 ? NSTextAlignmentCenter:NSTextAlignmentLeft) ;
    textLayer.textLayerProperty.useUnderline = self.underlineSwitch.on;
    textLayer.textLayerProperty.textColor = self.fontColorButton.backgroundColor;
    textLayer.textLayerProperty.useBackground = self.bgColorSwitch.on;
    textLayer.textLayerProperty.bgColor = self.bgColorButton.backgroundColor;
    textLayer.textLayerProperty.useGlow = self.glowColorSwitch.on;
    textLayer.textLayerProperty.glowColor = self.glowColorButton.backgroundColor;
    textLayer.textLayerProperty.glowSpread = self.glowSpreadSlider.value;
    textLayer.textLayerProperty.glowSize = self.glowSizeSlider.value;

    textLayer.textLayerProperty.useShadow = self.shadowColorSwitch.on;
    textLayer.textLayerProperty.shadowColor = self.shadowColorButton.backgroundColor;
    textLayer.textLayerProperty.shadowAngle = self.shadowAngleSlider.value;
    textLayer.textLayerProperty.shadowDistance = self.shadowDistanceSlider.value;
    textLayer.textLayerProperty.shadowSpread = self.shadowSpreadSlider.value;
    textLayer.textLayerProperty.shadowSize = self.shadowSizeSlider.value;
    textLayer.textLayerProperty.useOutline = self.outlineColorSwitch.on;
    textLayer.textLayerProperty.outlineColor = self.outlineColorButton.backgroundColor;
    textLayer.textLayerProperty.outlineThickness = self.outlineThicknessSlider.value;

        
    [textLayer updateTextProperty];
    
    UIFont *font;
    if([self.fontNamingLabel.text isEqualToString:@"system default"]) {
        font = [UIFont systemFontOfSize:self.fontSizeSlider.value];
    } else {
        font = [UIFont fontWithName:self.fontNamingLabel.text size:self.fontSizeSlider.value];
    }
    [textLayer setText:[textLayer getText] Font:font];

    self.refreshTextLayerTableView(positionLayer);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 27;
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueEditText"]) {
        TextInputViewController *tvc = (TextInputViewController *)[segue destinationViewController];
        [tvc setupTextLayer4InputCB:^(NSString *text) {
            // Inputed Text
            NSArray *array = [text componentsSeparatedByString:@"\n"];
            if([array count] > 1) {
                self.editTextLabel.text = [NSString stringWithFormat:@"%@ ...", array[0]];
            } else {
                self.editTextLabel.text = array[0];
            }
            [textLayer setText:text Font:[textLayer getFont]];
        } textLayerTitle:[textLayer getText]];
    }
    else if([[segue identifier] isEqualToString:@"segueLayerInAnimation"]) {
        LayerAnimationTableViewController *aniTV = (LayerAnimationTableViewController *)[segue destinationViewController];
        [aniTV setupLayerAnimationWithCB:^(int position) {
            positionInAnimation = position;
            if(position == 0) {
                self.inAnimationSlider.value = 0;
            } else {
                if(self.inAnimationSlider.value == 0) {
                    self.inAnimationSlider.value = 1000;
                }
            }
            self.inAnimationLabel.text = [self getAnimation:positionInAnimation type:0];
            self.inAnimationTimeLabel.text = [StringTools formatTimeMs:self.inAnimationSlider.value];
        } type:0 selectedLayerAnimation:positionInAnimation];
    }
    else if([[segue identifier] isEqualToString:@"segueLayerExpression"]) {
        LayerExpressionTableViewController *expressTV = (LayerExpressionTableViewController *)[segue destinationViewController];
        [expressTV setupLayerExpressionWithCB:^(int position) {
            positionExpression = position;
            self.expressionLabel.text = [self getExpression:positionExpression];
        } selectedLayerExpression:positionExpression];
    }
    else if([[segue identifier] isEqualToString:@"segueLayerOutAnimation"]) {
        LayerAnimationTableViewController *aniTV = (LayerAnimationTableViewController *)[segue destinationViewController];
        [aniTV setupLayerAnimationWithCB:^(int position) {
            positionOutAnimation = position;
            if(position == 0) {
                self.outAnimationSlider.value = 0;
            } else {
                if(self.outAnimationSlider.value == 0) {
                    self.outAnimationSlider.value = 1000;
                }
            }
            self.outAnimationLabel.text = [self getAnimation:positionOutAnimation type:1];
            self.outAnimationTimeLabel.text = [StringTools formatTimeMs:self.outAnimationSlider.value];
        } type:1 selectedLayerAnimation:positionOutAnimation];
    }
    else if([[segue identifier] isEqualToString:@"segueTextLayerFont"]) {
        TextLayerFontTableViewController *fontTV = (TextLayerFontTableViewController *)[segue destinationViewController];
        [fontTV setupTextLayerFontWithCB:^(NSString *fontNaming) {
            if([fontNaming isEqualToString:@"system default"]) {
                self.fontNamingLabel.text = fontNaming;
            } else {
                self.fontNamingLabel.text = [Util getUIFontName:fontNaming];
            }
        } selectedTextFontNaming:self.fontNamingLabel.text];
    }
    else if([[segue identifier] isEqualToString:@"segueTextLayerColor"]) {
        TextLayerColorTableViewController *fontColorTV = (TextLayerColorTableViewController *)[segue destinationViewController];
        [fontColorTV setupTextLayerColorWithCB:^(UIColor *fontColor) {
            focusedButton.backgroundColor = fontColor;
        } selectedTextColor:focusedButton.backgroundColor selectedAlpah:focusedAlpha];
    }
}

@end
