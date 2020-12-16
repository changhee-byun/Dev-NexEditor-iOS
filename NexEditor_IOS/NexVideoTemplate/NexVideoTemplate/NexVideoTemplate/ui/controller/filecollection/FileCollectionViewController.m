/******************************************************************************
 * File Name   :	FileCollectionViewController.m
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
@import Photos;
#import "FileCollectionViewController.h"
#import "FileCollectionHeaderView.h"
#import "FileCollectionViewCell.h"
#import "PreviewViewController.h"
#import "FilePicker.h"
#import "Util.h"

#define MAX_DISPLAY_COUNT 12
@interface FileCollectionViewController() <UICollectionViewDataSource, UICollectionViewDelegate, UICollectionViewDelegateFlowLayout>
{
    
}

/* file colleciton */
@property (strong, nonatomic) IBOutlet UICollectionView *collectionView;
@property (strong, nonatomic) FilePicker *filePicker;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *nextButton;
@property (assign, nonatomic) BOOL bViewDidAppear;
@property (assign, nonatomic) int currentPage;
@property (strong, nonatomic) IBOutlet UILabel *pageLabel;
@property (strong, nonatomic) IBOutlet UIButton *nextPageButton;
@property (strong, nonatomic) IBOutlet UIButton *prevPageButton;
@property (assign, nonatomic) int cellLoadingCount;
@end

@implementation FileCollectionViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    self.navigationController.navigationBar.backItem.title = @"";
    self.navigationItem.rightBarButtonItem = nil;
    
   
    /* init filepicker */
    _filePicker = [[FilePicker alloc] init];
    
    /* init views */
    self.collectionView.allowsMultipleSelection = YES;
    self.collectionView.delegate = self;
    self.collectionView.dataSource = self;
    
    UICollectionViewFlowLayout *collectionViewLayout = (UICollectionViewFlowLayout*)self.collectionView.collectionViewLayout;
    collectionViewLayout.sectionInset = UIEdgeInsetsMake(20, 0, 20, 0);
    
    _currentPage = 0;
    _cellLoadingCount = 0;
}


- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    if(self.bViewDidAppear == false) {
        [self.filePicker removeAllExportFileInDocument];
        
        if ([PHPhotoLibrary authorizationStatus] == PHAuthorizationStatusAuthorized) {
            
            UIActivityIndicatorView *activityIndicator = [self showLoadingIndicator];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                [self.filePicker updateListFromPHAsset];
                
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self.collectionView reloadData];
                    [self.collectionView layoutIfNeeded];
                    [self updatePageLabel];
                    [self hideLoadingIndicator:activityIndicator];
                });
            });
        } else {
            UIActivityIndicatorView *activityIndicator = [self showLoadingIndicator];
            [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
                if (status == PHAuthorizationStatusAuthorized) {
                    [self.filePicker updateListFromPHAsset];
                    
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [self.collectionView reloadData];
                        [self.collectionView layoutIfNeeded];
                        [self updatePageLabel];
                        [self hideLoadingIndicator:activityIndicator];
                    });
                }
            }];
        }
        
        self.bViewDidAppear = true;
    }
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (UIActivityIndicatorView*)showLoadingIndicator
{
    UIActivityIndicatorView *activityIndicator = [[UIActivityIndicatorView alloc] initWithFrame:CGRectMake(0, 0, 32, 32)];
    [activityIndicator setCenter:self.view.center];
    [activityIndicator setActivityIndicatorViewStyle:UIActivityIndicatorViewStyleGray];
    [self.view addSubview : activityIndicator];
    activityIndicator.hidden= FALSE;
    [activityIndicator startAnimating];
    
    return activityIndicator;
}

- (void)hideLoadingIndicator:(UIActivityIndicatorView*)activityIndicator
{
    [activityIndicator stopAnimating];
    activityIndicator.hidden= TRUE;
}

#pragma mark - UICollectionViewDelegate
- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView
{
    int countSection = 0;
    if([[self.filePicker getVideoList] count] != 0) {
        countSection++;
    }

    return countSection;
}

- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    if(section == VideoType) {
        return MAX_DISPLAY_COUNT;
    }
    
    return 0;
}

- (UICollectionReusableView *)collectionView:(UICollectionView *)collectionView viewForSupplementaryElementOfKind:(NSString *)kind atIndexPath:(NSIndexPath *)indexPath
{
    UICollectionReusableView *reusableview = nil;
    
    if (kind == UICollectionElementKindSectionHeader) {
        FileCollectionHeaderView *headerView = [collectionView dequeueReusableSupplementaryViewOfKind:UICollectionElementKindSectionHeader withReuseIdentifier:@"FileCollectionHeaderView" forIndexPath:indexPath];
        
        if(indexPath.section == VideoType) {
            headerView.headerLabel.text = [[NSString alloc]initWithFormat:@"Video"];
        } 
        
        reusableview = headerView;
    }
    
    return reusableview;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath{
    static NSString *identifier = @"FileCollectionViewCell";
    FileCollectionViewCell *cell = (FileCollectionViewCell*)[collectionView dequeueReusableCellWithReuseIdentifier:identifier forIndexPath:indexPath];
    

    int currentindex = ((int)indexPath.row + (MAX_DISPLAY_COUNT * self.currentPage));
    if(currentindex < (int)[[self.filePicker getVideoList] count]) {
        self.cellLoadingCount++;
        
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
            __block UIImage *thumbnail = nil;
            __block NSString *duration = nil;
            __block bool isChecked = false;
            
            thumbnail = [Util imageFromVideoAsset:((ListItem*)[[self.filePicker getVideoList] objectAtIndex:currentindex]).avURLAsset];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                if(thumbnail != nil) {
                    cell.thumbnail.image = thumbnail;
                } else {
                    cell.thumbnail.image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"default_black" ofType:@"png"]];
                }
                
                duration = [Util getStringTime:((ListItem*)[[self.filePicker getVideoList] objectAtIndex:currentindex]).duration];
                if(duration != nil) {
                    cell.duration.text = duration;
                } else {
                    cell.duration.text = @"00:00";
                }
                
                isChecked = ((ListItem*)[[self.filePicker getVideoList] objectAtIndex:currentindex]).isChecked;
                if(isChecked) {
                    cell.isChecked.hidden = NO;
                } else {
                    cell.isChecked.hidden = YES;
                }
                
                self.cellLoadingCount--;
                if(self.cellLoadingCount == 0) {
                    self.nextPageButton.enabled = YES;
                    self.prevPageButton.enabled = YES;
                }
            });
        });
    }
    
    return cell;
}

- (void)collectionView:(UICollectionView *)collectionView didHighlightItemAtIndexPath:(NSIndexPath *)indexPath
{
}

- (void)collectionView:(UICollectionView *)collectionView didUnhighlightItemAtIndexPath:(NSIndexPath *)indexPath
{
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
    FileCollectionViewCell *cell = (FileCollectionViewCell*)[collectionView cellForItemAtIndexPath:indexPath];
    
    int currentindex = ((int)indexPath.row + (MAX_DISPLAY_COUNT * self.currentPage));
    if(currentindex < (int)[[self.filePicker getVideoList] count]) {
        ListItem *cellItem = [[self.filePicker getVideoList] objectAtIndex:currentindex];
        cellItem.isChecked = true;
        cell.isChecked.hidden = NO;
        
        if(self.navigationItem.rightBarButtonItem == nil) {
            self.navigationItem.rightBarButtonItem = self.nextButton;
        }
    }
}

- (void)collectionView:(UICollectionView *)collectionView didDeselectItemAtIndexPath:(NSIndexPath *)indexPath
{
    FileCollectionViewCell *cell = (FileCollectionViewCell*)[collectionView cellForItemAtIndexPath:indexPath];
    
    int currentindex = ((int)indexPath.row + (MAX_DISPLAY_COUNT * self.currentPage));
    if(currentindex < (int)[[self.filePicker getVideoList] count]) {
        ListItem *cellItem = [[self.filePicker getVideoList] objectAtIndex:currentindex];
        cellItem.isChecked = false;
        cell.isChecked.hidden = YES;
        
        if([[self.collectionView indexPathsForSelectedItems] count] == 0) {
            self.navigationItem.rightBarButtonItem = nil;
        }
    }
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    float margin = 1.0f;
    float cellWidth = self.collectionView.frame.size.width / 3.0f - margin;
    float cellHeight = cellWidth;
    return CGSizeMake(cellWidth, cellHeight);
}
#pragma end

#pragma mark - Button Click
- (IBAction)didNextButtonClick:(id)sender
{
    [self performSegueWithIdentifier:@"seguePreview" sender:self];
}

- (IBAction)didNextPageButtonClick:(id)sender
{
    if(self.currentPage + 1 < [self getTotalPage]) {
        for(FileCollectionViewCell *cell in self.collectionView.visibleCells) {
            cell.thumbnail.image = nil;
            cell.duration.text = @"00:00";
            cell.isChecked.hidden = YES;
        }
        self.currentPage++;
        self.nextPageButton.enabled = NO;
        self.prevPageButton.enabled = NO;
        
        [self.collectionView reloadData];
        [self.collectionView layoutIfNeeded];
        [self updatePageLabel];
    }
}

- (IBAction)didPrevPageButtonClick:(id)sender
{
    
    if(self.currentPage > 0) {
        for(FileCollectionViewCell *cell in self.collectionView.visibleCells) {
            cell.thumbnail.image = nil;
            cell.duration.text = @"00:00";
            cell.isChecked.hidden = YES;
        }
        self.currentPage--;
        self.nextPageButton.enabled = NO;
        self.prevPageButton.enabled = NO;
        
        [self.collectionView reloadData];
        [self.collectionView layoutIfNeeded];
        [self updatePageLabel];
    }
    
}
#pragma end

#pragma mark - Segue
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    NSArray *selectedIndexs = [self getSelectedCollectionViewIndexs];
    
    NSMutableArray *fileList = [[NSMutableArray alloc] init];
    NSMutableArray *avURLAssetList = [[NSMutableArray alloc] init];
    
    for(NSNumber *number in selectedIndexs) {
        [fileList addObject:((ListItem*)[[self.filePicker getVideoList] objectAtIndex:[number integerValue]]).avURLAsset.URL.path];
        [avURLAssetList addObject:((ListItem*)[[self.filePicker getVideoList] objectAtIndex:[number integerValue]]).avURLAsset];
    }
    
    if([[segue identifier] isEqualToString:@"seguePreview"]) {
        // go "PreviewViewController"
        [(PreviewViewController*)[segue destinationViewController] setProject4Preview:fileList avURLAssetList:avURLAssetList];
    }
}
#pragma end

#pragma mark - Private
- (int)getTotalPage
{
    int totalPage = (int)[[self.filePicker getVideoList] count] / MAX_DISPLAY_COUNT;
    
    if(totalPage == 0) {
        return 1;
    }
    
    if((int)[[self.filePicker getVideoList] count] % MAX_DISPLAY_COUNT > 0) {
        totalPage++;
    }
    
    return totalPage;
}


- (void)updatePageLabel
{
    self.pageLabel.text = [NSString stringWithFormat:@"%d/%d", self.currentPage + 1, [self getTotalPage]];
}

- (NSArray*)getSelectedCollectionViewIndexs
{
    NSMutableArray *selectedIndexs = [[NSMutableArray alloc] init];
    for(int i = 0; i < [[self.filePicker getVideoList] count]; i++) {
        ListItem *item = [[self.filePicker getVideoList] objectAtIndex:i];
        if(item.isChecked) {
            [selectedIndexs addObject:[NSNumber numberWithInteger:i]];
        }
    }
    
    return [NSArray arrayWithArray:selectedIndexs];
}
#pragma end
@end
