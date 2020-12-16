/******************************************************************************
 * File Name   :	IProperty.h
 * Description :	The interface of Get/SetProperty used in CALBody.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef IProperty_h
#define IProperty_h

class IProperty {
public:
    virtual ~IProperty() {};
    
    virtual bool getProperty( int propertyIndex, void** value ) = 0;
    virtual bool setProperty( int propertyIndex, void* value ) = 0;
};

#endif /* IProperty_h */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 15, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
