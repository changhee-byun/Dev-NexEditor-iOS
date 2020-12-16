/**
 * File Name   : BlockTimer.swift
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

import Foundation

class BlockTimer {
    var timer: Timer!
    var block: (BlockTimer) -> Void!
    
    init(withTimeInterval timeInterval: TimeInterval, repeats: Bool, block: @escaping (BlockTimer) -> Void) {
        
        self.block = block
        timer = Timer.scheduledTimer(timeInterval: timeInterval,
                                     target: self,
                                     selector: #selector(timerFired(_:)),
                                     userInfo: self,
                                     repeats: repeats)
    }
    
    @objc func timerFired(_ timer: Timer)
    {
        self.block(self)
    }
}
