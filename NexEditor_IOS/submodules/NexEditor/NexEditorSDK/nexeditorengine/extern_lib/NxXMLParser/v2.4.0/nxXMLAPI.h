/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |                            NexStreaming Corporation                      |
  |    	    Copyrightⓒ 2002-2013 All rights reserved to NexStreaming        |
  |                        http://www.nexstreaming.com                       |
  |                                                                          |
  |__________________________________________________________________________|

*******************************************************************************
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
_______________________________________________________________________________
*
*		NEXSALFileHandle Name			: nxXMLAPI.h
*		Author				: DongNam Kang /NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			: nxXMLAPI
*		Revision History	: 
*		Comment				: 
*		Version				: 2.4.0.OFFICIAL (2016.01.15)
******************************************************************************/

/** @NEXSALFileHandle nxXMLAPI.h
 */

#ifndef __NXXMLAPI_H__
#define __NXXMLAPI_H__
#include <NexTypeDef.h>
#include <NexMediaDef.h>
#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
#define pNXVOID NXVOID*
#ifndef POINTER_SIZE
	#define POINTER_SIZE sizeof(pNXVOID)
#endif //POINTER_SIZE

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

#define NXXML_VERSION_MAJOR		2
#define NXXML_VERSION_MINOR		4
#define NXXML_VERSION_PATCH		0
#define NXXML_VERSION_BRANCH	"OFFICIAL"

#define NXXML_COMPATIBILITY_NUM	3	



//#define NXXML_VERSION ( (NXXML_VERSION_MAJOR << 16) | (NXXML_VERSION_MINOR << 8) | (NXXML_VERSION_PATCH))

typedef pNXVOID NXXML_HANDLER;
typedef pNXVOID NXXML_NODE;
typedef pNXVOID NXXML_NODELIST;
typedef pNXVOID NXXML_ATTRIBUTE;

typedef enum _NXXML_RET{
	NXXMLRET_SUCCESS	= 0,
	NXXMLRET_INVALID_VERSION	= -1,
	NXXMLRET_INVALID_HANDLER,
	NXXMLRET_INVALID_NODE,
	NXXMLRET_INVALID_LISTNODE,
	NXXMLRET_INVALID_ATTRIBUTE,

	NXXMLRET_PARSE_ERROR	=   -101,
	NXXMLRET_MEMORY_ERROR,

	NXXMLRET_NO_ELEMENT	= -201,
	NXXMLRET_NOMORE_NEXTELEMENT,
	NXXMLRET_NOMORE_PREVELEMENT,

	NXXMLRET_NO_CHILDELEMENT	= -301,
	NXXMLRET_NOMORE_NEXTCHILD,
	NXXMLRET_NOMORE_PREVCHILD,
	
	NXXMLRET_ATTRIBUTE_NOT_FOUND	= -401,
	NXXMLRET_NO_ATTRIBUTE,
	NXXMLRET_NOMORE_NEXTATTRIBUTE,
	NXXMLRET_NOMORE_PREVATTRIBUTE
}NXXML_RET;

typedef enum _NXXML_NODETYPE{
	NXXML_ELEMENT_NODE		= 0,
	NXXML_PCDATA_NODE,
	NXXML_CDATA_NODE, 
	NXXML_INVALID_NODE		= -1
}NXXML_NODETYPE;

typedef struct {
	NXUINT16 nLen;
	NXUINT8 *pValue;
}nxXMLDuple, *pNxXMLDuple;

/******************************************************************************
 * Function Name			: 
 * Return Value Description : 
 * Comment					: nxXMLParser version function
******************************************************************************/
extern NXBOOL	NxXML_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);
extern NXBOOL	NxXML_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);
extern NXCHAR*	NxXML_GetVersionString();
extern NXINT32	NxXML_GetMajorVersion();
extern NXINT32	NxXML_GetMinorVersion();
extern NXINT32	NxXML_GetPatchVersion();
extern NXCHAR*	NxXML_GetBranchVersion();
extern NXCHAR*	NxXML_GetVersionInfo();

/******************************************************************************
 * Function Name			: 
 * Return Value Description : 
 * Comment					: XML create and destroy
******************************************************************************/
extern NXXML_HANDLER nxXML_Create(IN NXUINT32 nVersion, IN NXUINT32 nMaxHeapSize);
extern void nxXML_Destroy(IN NXXML_HANDLER hHandler);

// For Parse
//extern NXXML_RET nxXML_FileParse(NXXML_HANDLER hHandler, const char *pFileName);
extern NXXML_RET nxXML_StringParse(IN NXXML_HANDLER hHandler, IN const char *pBuffer, IN NXUSIZE nSize);
//extern NXXML_HANDLER nxXML_StringParse(const char *pBuffer, unsigned nSize);
extern NXXML_RET nxXML_Close(NXXML_HANDLER hHandler);

// for Validate 
extern NXINT32 nxXML_ValidateHandler(NXXML_HANDLER hHandler);
extern NXINT32 nxXML_ValidateNode(NXXML_NODE Node);
extern NXINT32 nxXML_ValidateNodeList(NXXML_NODELIST NodeList);
extern NXINT32 nxXML_ValidateAttribute(NXXML_ATTRIBUTE Attribute);

// for Element
extern NXXML_RET nxXML_GetRoot(IN NXXML_HANDLER hHandler, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_GetCurrentNode(IN NXXML_HANDLER hHandler, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_SetCurrentNode(IN NXXML_HANDLER hHandler, IN NXXML_NODE Node);

extern NXXML_RET nxXML_GetParent(IN NXXML_NODE Node, OUT NXXML_NODE *pParentNode);
extern NXXML_NODETYPE nxXML_GetElementType(IN NXXML_NODE Node);
extern pNxXMLDuple nxXML_GetNameSpace(IN NXXML_NODE Node);
extern pNxXMLDuple nxXML_GetElementName(IN NXXML_NODE Node);	// pcdata나 cdata의 경우 Element Name 이 값이다.

extern NXINT32 nxXML_FindElementByName(IN NXXML_HANDLER hHandler, IN const char *pNS, IN const char *pName, OUT NXXML_NODELIST *pNodeList);
extern NXINT32 nxXML_FindElementByXPath(IN NXXML_HANDLER hHandler, IN const char *pXPath, OUT NXXML_NODELIST *pNodeList);

extern NXXML_RET nxXML_GetNextElement(IN NXXML_HANDLER hHandler, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_GetPrevElement(IN NXXML_HANDLER hHandler, OUT NXXML_NODE *pNode);

extern NXINT32 nxXML_GetChildCount(IN NXXML_NODE Node);
extern NXXML_RET nxXML_SetCurrentChild(IN NXXML_NODE Node, IN NXXML_NODE ChildNode);
extern NXXML_RET nxXML_GetCurrentChild(IN NXXML_NODE Node, OUT NXXML_NODE *pChildNode);
extern NXXML_RET nxXML_GetFirstChild(IN NXXML_NODE Node, OUT NXXML_NODE *pChildNode);
extern NXXML_RET nxXML_GetLastChild(IN NXXML_NODE Node, OUT NXXML_NODE *pChildNode);
extern NXXML_RET nxXML_GetNextChild(IN NXXML_NODE Node, OUT NXXML_NODE *pChildNode);
extern NXXML_RET nxXML_GetPrevChild(IN NXXML_NODE Node, OUT NXXML_NODE *pChildNode);

// for NXXML_NODELIST
extern NXINT32 nxXML_NLCount(IN NXXML_NODELIST NodeList);
extern NXXML_RET nxXML_NLDestroy(IN NXXML_NODELIST NodeList);
extern NXXML_RET nxXML_NLGetCurrentNode(IN NXXML_NODELIST NodeList, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_NLGetFirstNode(IN NXXML_NODELIST NodeList, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_NLGetLastNode(IN NXXML_NODELIST NodeList, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_NLGetNextNode(IN NXXML_NODELIST NodeList, OUT NXXML_NODE *pNode);
extern NXXML_RET nxXML_NLGetPrevNode(IN NXXML_NODELIST NodeList, OUT NXXML_NODE *pNode);

// for Attribute
extern NXXML_RET nxXML_FindAttributeByName(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *pAttribute, IN const NXUINT8 *pName);
extern pNxXMLDuple nxXML_GetAttributeNameSpace(IN NXXML_ATTRIBUTE Attribute);
extern pNxXMLDuple nxXML_GetAttributeName(IN NXXML_ATTRIBUTE Attribute);
extern pNxXMLDuple nxXML_GetAttributeString(IN NXXML_ATTRIBUTE Attribute);


extern NXINT32 nxXML_GetAttributeCount(IN NXXML_NODE Node);
extern NXXML_RET nxXML_GetCurrentAttribute(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *Attribute);
extern NXXML_RET nxXML_GetFirstAttribute(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *Attribute);
extern NXXML_RET nxXML_GetLastAttribute(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *Attribute);
extern NXXML_RET nxXML_GetNextAttribute(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *Attribute);
extern NXXML_RET nxXML_GetPrevAttribute(IN NXXML_NODE Node, OUT NXXML_ATTRIBUTE *Attribute);

// for debug
extern void nxXML_PrintTree(IN NXXML_HANDLER hHandler);
extern void _printDuple(const pNxXMLDuple pDuple);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif //__NXXMLAPI_H__