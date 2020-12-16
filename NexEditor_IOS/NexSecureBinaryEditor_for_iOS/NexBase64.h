//=============================================================================
// Nextreaming Confidential Proprietary
// Copyright (C) 2005-2008 Nextreaming Corporation
// All rights are reserved by Nextreaming Corporation
//
// File       : NexSecureWare.h
// Description: 
//
// Revision History:
// Version   Date         Author			Description of Changes
//-----------------------------------------------------------------------------
// 1.0.0     2008/09/11   Kye-Beom Song		Draft
//=============================================================================

#ifndef _NEXBASE64_H_
#define _NEXBASE64_H_

int BitsToBase64(unsigned char* pOut, int nOutLen, unsigned char* pIn, int nInLen);
int Base64ToBits(unsigned char* pOut, int nOutLen, unsigned char* pIn, int nInLen);

#endif // _NEXBASE64_H_