/******************************************************************************
 * File Name        : nexChecker.java
 * Description      :
 *******************************************************************************
 * Copyright (c) 2002-2017 NexStreaming Corp. All rights reserved.
 * http://www.nexstreaming.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

package com.nexstreaming.nexeditorsdk;

import com.nexstreaming.checkcaps.Checker;

/**
 * This class is used to test a device's hardware encoder performance in runtime. 
 * The hardware codec resources is used for the encoder test.
 * The test result will not return instantly, therefore the encoder test result should be received asynchronously. 
 * 
 * @since version 1.3.43
 */
public final class nexChecker {

    final static int UHD_WIDTH = 3840;
    final static int UHD_HEIGHT = 2160;
    
    private static Checker mChecker;

    /**
     * This listener receives the device's hardware encoder test result from the <tt>nexChecker</tt>.  
     * 
     * @see #checkUHD(nexCheckerListener)
     * @since version 1.3.43
     */
    public interface nexCheckerListener {
        
          
        /**
         * This method checks whether the hardware encoder of a device is currently available. 
         * 
         * @param result  0 if currently available, otherwise 1. 
         * @since version 1.3.43
         */
        public void onCheckerCapsResult(int result);
    }
  
    /**
     * This method checks whether a device can encode UHD content.
     * 
     * <p>Example code:</p>
     *     <b>class sample code</b>
     *     {@code     nexChecker.checkUHD(new nexChecker.nexCheckerListener() {
                    @Override
                    public void onCheckerCapsResult(int result) {

                    }
                });
            }
     * @param checkerListener The listener to receive the result.
     * @since version 1.3.43
     */
    public static void checkUHD(final nexCheckerListener checkerListener) {
        mChecker = new Checker(false);
        mChecker.setCheckerEventListener(new Checker.CheckerEventListener() {
            
              
            /**
             * This method gets the information when checking the current availability of the hardware encoder.  
             *
             * @param checker The checker class.
             * @param msg The message about the encoder availability as a <tt>string</tt>.
             * @since version 1.3.43
             */
            @Override
            public void onCheckerCapsInfoAvailable(Checker checker, String msg) {

            }

   
            /**
             * This method gets the result when checking the current availability of the hardware encoder. 
             * 
             * @param checker The checker class. 
             * @param nResult 0 if the encoder is available, otherwise 1.
             * @since version 1.3.43
             */
            @Override
            public void onCheckerCapsResult(Checker checker, int nResult) {
//                if(mChecker.equals(checker)) {
//                    checkerListener.onCheckerCapsResult(nResult);
//                }
                checkerListener.onCheckerCapsResult(nResult);
            }
        });
        mChecker.CheckEncoderAvailability(UHD_WIDTH, UHD_HEIGHT);
    }
}
