/******************************************************************************
 * File Name   :	NXEProject.h
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

#import <Foundation/Foundation.h>

@class NXEClip;

/**
 * \brief This class manages clips to be edited with NexEditor&trade;SDK as projects. <p>To edit with the NexEditor&trade;SDK, a project is created, 
 * and then clips(video, image, or audio) are added to the project and further edited with the options available. 
 * <p>A finished project is then exported to become the final version of a NexEditor&trade; video. The following sample code demonstrates how to create a project 
 * and add clips to the project
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [engine setProject:project];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5 
 */
@interface NXEProject : NSObject <NSCopying>
/**
 * \brief This method accesses the instance of a video and image clip, among other clips added to a project.
 * \see <b>NexVisualClip</b>
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray* visualItems;
/**
 * \brief This method accesses the instance of an audio clip, among other clips added to a project.
 * \see <b>NexAudioClip</b>
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableArray* audioItems;
/**
 * \brief This method gets the total playtime information of a project
 * \return The Total playtime information of a project in <tt>msec</tt> (milliseconds).
 * \since version 1.0.5
 */
- (int)getTotalTime;

/**
 * \brief This method gets the total number of clips saved a specific track.
 * \param primary Set to <tt>TRUE</tt> if the track is composed of video clips or image clips; Set to <tt>FALSE</tt> if the track is composed audio clips only.
 * \since version 1.0.5
 */
- (int)getTotalClipCount:(BOOL)primary;

/**
 * \brief This method is called to get an update of the total playtime of a project when changes.
 * \since version 1.0.5
 */
- (void)updateProject;

/**
 * \brief This method clears all of the clips in a track in a project.
 * \since version 1.0.5
 */
- (void)allClear:(BOOL)primary;

/**
 * \brief This method adds a clip to a project.
 * \param <b>NXECLip</b> instance.
 * \see <b>NXECLip</b>
 * \since version 1.0.5
 */
- (void)addClip:(NXEClip*)clip;

/**
 * \brief This method sets the background music of a project.
 * \param path The path of music contents.
 * \param volumeSacle 배경음악에 대한 volumeScale값을 조정해준다. 기본값으로 0.5이고, 0~1.0의 값을 입력해 주면 된다.
 * \since version 1.0.5
 */
- (BOOL)setBGM:(NSString *)path volumeScale:(float)volumeScale;

/**
 * \brief BGM 설정 된 클립에 대한 NXEClip 정보를 리턴해준다.
 * \return NXECLip
 * \since version 1.12.0
 */
- (NXEClip *)getBGMClip;

@end
