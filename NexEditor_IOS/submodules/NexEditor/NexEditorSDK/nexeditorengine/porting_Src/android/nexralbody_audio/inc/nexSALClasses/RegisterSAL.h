/******************************************************************************
* File Name        : RegisterSAL.h
* Description      : Convenience class for registering functions with nexSAL
* Revision History : Located at the bottom of this file
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nextreaming Confidential Proprietary
*	Copyright (C) 2006~2012 Nextreaming Corporation
*	All rights are reserved by Nextreaming Corporation
*
******************************************************************************/

#ifndef __NEX_REGISTERSAL_H__
#define __NEX_REGISTERSAL_H__

namespace nextreaming
{
	
// Expected to be implemented by the porter
class RegisterSAL
{
public:
	#ifdef SI_BUG
	int SI_BUG1;
	#endif
	
	// registers all nexSAL functions, along with system-dependent initialization
	RegisterSAL();
	
	// performs system-dependent deinitialization
	// may or may not unregister nexSAL functions
	~RegisterSAL();
};

};

#endif

/*=============================================================================
                                Revision History
===============================================================================
    Author         Date          API Version
===============================================================================
    benson         2012/03/06    1.0
-------------------------------------------------------------------------------
 Initial Draft
=============================================================================*/
