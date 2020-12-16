/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ClipList.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_ClipList.h"
#include "NEXVIDEOEDITOR_VideoEditor.h" //yoon for LG
#include "NexSAL_Internal.h"
#include<map>
#include<set>
#include <algorithm>

bool compareClipStartTime(CClipItem *p1, CClipItem *p2) 
{ 
    return (p1->getStartTime() < p2->getStartTime()); 
} 

//---------------------------------------------------------------------------
CClipList::CClipList()
{
	m_uiTotalTime = 0;

	m_ClipItemVec.clear();
	m_AudioClipItemVec.clear();
	
	m_LayerItemVec.clear();

	m_pEffectItemVec = new CVideoEffectItemVec;

	m_pBackGround = NULL;

	m_iBGMFadeInTime	= 0;
	m_iBGMFadeOutTime	= 0;

	m_bSeparateEffect		= FALSE;
	m_isCheckDirectExport	= FALSE;

	m_iDirectExportIndex	= 0;
	
	m_isCheckingDirectExport = FALSE; //yoon
	m_isCancelDirectExport = FALSE; //yoon

	memset(m_pVisualIDs, 0x00, sizeof(unsigned int)*50);
	m_uVisualIndex = 0; 	
}

CClipList::~CClipList()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ClipList.cpp %d] ~~~~~~CClipList In", __LINE__);
	clearClipList();
	
	if( m_pEffectItemVec )
	{
		SAFE_RELEASE(m_pEffectItemVec);
	}

	SAFE_RELEASE(m_pBackGround);
	m_iDirectExportIndex = 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ClipList.cpp %d] ~~~~~~CClipList Out", __LINE__);
}

int CClipList::getClipCount()
{
	return (int)m_ClipItemVec.size();
}

IClipItem* CClipList::getClip(int nIndex)
{
	if( nIndex < 0 ||  nIndex > (int)m_ClipItemVec.size() )
		return NULL;

	m_ClipItemVec[nIndex]->AddRef();
	return m_ClipItemVec[nIndex];
}

IClipItem* CClipList::getClipUsingID(unsigned int uiID)
{
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->getClipID() == uiID )
		{
			m_ClipItemVec[i]->AddRef();
			return m_ClipItemVec[i];
		}
		CClipItem* pAudioClip = m_ClipItemVec[i]->getAudioClipUsingClipID(uiID);
		if( pAudioClip )
		{
			return pAudioClip;
		}
	}
	return NULL;
}

IClipItem* CClipList::createClipUsingID(unsigned int uiID, int isAudio)
{
	CClipItem* pItem	= new CClipItem(uiID);
	if( pItem == NULL )
		return NULL;

	if( isAudio )
	{
#if 0		
		if( m_ClipItemVec.size() <= 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] Do not add audio clip: visual clip is not exist", __LINE__);
			pItem->Release();
			return NULL;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] createClipUsingID(Add audio clip done)", __LINE__);
		m_ClipItemVec[0]->addAudioClip(pItem);
		pItem->AddRef();
#endif
	}
	else
	{
		m_ClipItemVec.insert(m_ClipItemVec.end(), pItem);
		pItem->AddRef();
	}
	return (IClipItem*)pItem;
}

IClipItem* CClipList::createEmptyClipUsingID(unsigned int uiID)
{
	CClipItem* pItem	= new CClipItem(uiID);
	if( pItem == NULL )
		return NULL;

	return (IClipItem*)pItem;
}

ILayerItem* CClipList::createEmptyLayerUsingID(unsigned int uiID)
{
	CLayerItem* pItem	= new CLayerItem(uiID);
	if( pItem == NULL )
		return NULL;

	return (ILayerItem*)pItem;
}

int CClipList::addClipItem(IClipItem* pItem)
{
	if( pItem == NULL ) 
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

#ifdef FOR_VIDEO_LAYER_FEATURE
	if( pItem->getClipType() == CLIPTYPE_IMAGE || pItem->getClipType() == CLIPTYPE_VIDEO || pItem->getClipType() == CLIPTYPE_VIDEO_LAYER )
#else
	if( pItem->getClipType() == CLIPTYPE_IMAGE || pItem->getClipType() == CLIPTYPE_VIDEO )
#endif
	{
		SAFE_ADDREF(pItem);
		m_ClipItemVec.insert(m_ClipItemVec.end(), (CClipItem*)pItem);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	else if( pItem->getClipType() == CLIPTYPE_AUDIO )
	{
		SAFE_ADDREF(pItem);
		m_AudioClipItemVec.insert(m_AudioClipItemVec.end(), (CClipItem*)pItem);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addClipItem failed because item has wrong clip type", __LINE__);
	return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
}

int CClipList::addLayerItem(ILayerItem* pItem)
{
	if( pItem == NULL ) 
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( pItem->getType() != CLIPTYPE_LAYER )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addClipItem failed because item has wrong clip type", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	SAFE_ADDREF(pItem);
	m_LayerItemVec.insert(m_LayerItemVec.end(), (CLayerItem*)pItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipList::deleteLayerItem(unsigned int uiID)
{
	if( m_LayerItemVec.size() <= 0 )
		return NEXVIDEOEDITOR_ERROR_NONE; 

	CLayerItemVectorIterator i;
	for( i = m_LayerItemVec.begin(); i != m_LayerItemVec.end(); i++)
	{
		CLayerItem* pItem = (CLayerItem*)(*i);
		if( pItem != NULL && pItem->getID() == uiID )
		{
 			m_LayerItemVec.erase(i);
			pItem->Release();
			break;
 		}		
	}
	return NEXVIDEOEDITOR_ERROR_NONE; 
 }

int CClipList::clearLayerItem()
{
	for( int i = 0; i < m_LayerItemVec.size(); i++)
	{
		m_LayerItemVec[i]->Release();
	}
	m_LayerItemVec.clear();	
	return NEXVIDEOEDITOR_ERROR_NONE; 
}

int CClipList::addEffectItem(IEffectItem* pItem)
{
	if( pItem == NULL ) 
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	m_pEffectItemVec->lock();
	m_pEffectItemVec->addEffectItem((CVideoEffectItem*)pItem);
	m_pEffectItemVec->unlock();
	SAFE_RELEASE(pItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CClipList::lockClipList()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] lockClipList() In", __LINE__);
	m_ClipListLock.Lock();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] lockClipList() Out", __LINE__);
}

void CClipList::unlockClipList()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] unlockClipList() In", __LINE__);
	m_ClipListLock.Unlock();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] unlockClipList() Out", __LINE__);
}

int CClipList::clearClipList()
{
	stopPlay();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ClipList.cpp %d] clearClipList()", __LINE__);
	for( int i = 0; i < m_ClipItemVec.size(); i++)
	{
		CClipItem* pItem = (CClipItem*)m_ClipItemVec[i];
		SAFE_RELEASE(pItem);
	}
	m_ClipItemVec.clear();
	m_uiTotalTime = 0;

#ifdef FOR_LOADLIST_FEATURE
	for( int i = 0; i < m_AudioClipItemVec.size(); i++)
	{
		CClipItem* pItem = (CClipItem*)m_AudioClipItemVec[i];
		SAFE_RELEASE(pItem);
	}
	m_AudioClipItemVec.clear();
#endif	

#ifdef FOR_LAYER_FEATURE
	for( int i = 0; i < m_LayerItemVec.size(); i++)
	{
		CClipItem* pItem = (CClipItem*)m_LayerItemVec[i];
		SAFE_RELEASE(pItem);
	}
	m_LayerItemVec.clear();
#endif
	return TRUE;
}

/* iRet 
	0 : support direct encode
	1 : Invalid editor handle
	2 : Invalid clip list handle
	3 : Clip count is 0 or clip error
	4 : Video information was different.
	5 : Video codec unmatch
	6 : Video not start idr frame
	7 : Has image clip.
	8 : Has video layer.
	9 : Enable transition effect
	10 : Enable title effect
	11 : Encoder dsi unmatch with dsi of clip
	12 : applied speed control
	13 : Unsupport codec.
	14 : Rotate invalid.
	15 : Multi track content.
	16 : Has audio tracks.
	17 : Audio Codec unmatch
*/
struct _const_char_point_compare
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) < 0;
    }
};

typedef std::map<const char *, int>::iterator master_map_it;
typedef std::multimap<int, int>::iterator sub_mp_it;
typedef std::pair<sub_mp_it, sub_mp_it> sub_range_pair;
    
int CClipList::checkDirectExport(int option)
{
	CClipItem*	pFirst				= NULL;
	CClipItem*	pSecond				= NULL;
	CClipItem*	pPrev				= NULL; //yoon
	
	int			iRet					= 0;
    int isSameContent = 0; //yoon
	NXBOOL		bNeedCheckEncoder	= FALSE;

	if( m_isCheckDirectExport ){
		return 0;
	}
	
	if( m_isCancelDirectExport ){
	    m_isCancelDirectExport = FALSE;
	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  cancel", __LINE__);
	    return -1;
	}
	
	m_isCheckingDirectExport = TRUE; //yoon
	
	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 )
	{
	    m_isCheckingDirectExport = FALSE; //yoon
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  cliplist is 0", __LINE__);
		return 3;
	}

	if( m_pBackGround != NULL )
	{
	    m_isCheckingDirectExport = FALSE; //yoon
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  has audio BGM", __LINE__);
		return 16;
	}

	if( m_AudioClipItemVec.size() > 0 )
	{
	    m_isCheckingDirectExport = FALSE; //yoon
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  has audio clips(%zu)", __LINE__, m_AudioClipItemVec.size());
		return 16;
	}

    //yoon
    CClipItemVectorIterator i, j;
	int nRotateDegree = -1;
	std:map<const char *,int, _const_char_point_compare> clipmap;
	std::multimap<int, int> clips;
	int count = 0;
	for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
	{
		pFirst = ((CClipItem*)(*i));
		
		if( m_isCancelDirectExport ){
		    m_isCancelDirectExport = FALSE;
		    m_isCheckingDirectExport = FALSE;
	        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  cancel", __LINE__);
	        return -1;
	    }
		
		if( pFirst == NULL )
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport cliplist is 0", __LINE__);
			return 3;
		}
		
		if( pFirst->getAudioClipCount() > 0 )
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport has audio clip", __LINE__);
			return 16;
		}


		if( pFirst->getClipEffectEnable() )
		{
			if( strcmp("none", pFirst->getClipEffectID()) != 0 )
			{
			    m_isCheckingDirectExport = FALSE; //yoon
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check transition effect(I:%d)", __LINE__,
					pFirst->getClipID());			
				return 9;
			}
		}

		if( pFirst->getTitleEffectID() != NULL )
		{
			if( strcmp("", pFirst->getTitleEffectID()) != 0 )
			{
			    m_isCheckingDirectExport = FALSE; //yoon
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check title effect(I:%d)", __LINE__,
					pFirst->getClipID());			
				return 10;
			}
		}

		if(pFirst->getBrightness() 	|| 
			pFirst->getContrast() 	||
			pFirst->getSaturation() ||
			pFirst->getTintcolor() 	||
			pFirst->getLUT() 		||
			pFirst->getCustomLUTA() ||
			pFirst->getCustomLUTB() 
			){

			m_isCheckingDirectExport = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check Filter(I:%d)", __LINE__,
					pFirst->getClipID());			
			return 15;
		}

		if( pFirst->getClipType() == CLIPTYPE_IMAGE )
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check type(I:%d Type:%d)", __LINE__,
				pFirst->getClipID(), pFirst->getClipType());			
			return 7;
		}

		if( pFirst->getClipType() == CLIPTYPE_VIDEO_LAYER )
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check type(I:%d Type:%d)", __LINE__,
				pFirst->getClipID(), pFirst->getClipType());			
			return 8;
		}

		if( pFirst->getAudioVolume() != 100)
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  check getAudioVolume(I:%d VOL:%d)", __LINE__,
				pFirst->getClipID(), pFirst->getAudioVolume());			
			return 12;
		}
		if( (option & DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL) == DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL )
		{

		}
		else
		{
			for(int k=0; k<pFirst->m_ClipVideoRenderInfoVec.size();k++)
			{
				if(pFirst->m_ClipVideoRenderInfoVec[k]->m_iSpeedCtlFactor != 100)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport use speed control", __LINE__);
					m_isCheckingDirectExport = FALSE; //yoon
					return 12;
				}
			}
		}

        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] insert map(%s,%d)", __LINE__, pFirst->getClipPath(), pFirst->getStartTrimTime());
		master_map_it mi;
		mi=clipmap.find(pFirst->getClipPath());
		int index = 0;
		if( mi ==  clipmap.end() ){
		    clipmap.insert(std::pair<const char *, int>(pFirst->getClipPath(),count ));
		    index = count;
		}else{
		    index = mi->second;
		}
		clips.insert(std::pair<int, int>(index,count));
		count++;
    }
    
    sub_mp_it m_it, s_it;
    for (m_it = clips.begin();  m_it != clips.end();  m_it = s_it)
    {
        
        int index = m_it->first;
        
        if( m_isCancelDirectExport ){
		    m_isCancelDirectExport = FALSE;
		    m_isCheckingDirectExport = FALSE;
	        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  cancel", __LINE__);
	        return -1;
	    }
        
        pFirst = m_ClipItemVec[index];
        if( pFirst == NULL )
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport cliplist is index=%d", __LINE__,index);
			return 3;
		}
		
        pFirst->parseDSIofClip();

		if( pFirst->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO) != eNEX_CODEC_V_H264 && pFirst->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO) != eNEX_CODEC_V_HEVC && pFirst->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO) != eNEX_CODEC_V_MPEG4V)
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] Check codec OTI(0x%x)", __LINE__, pFirst->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO));
			return 13;
		}		

		if( pFirst->getAudioTrackCount() >= 2 ||  pFirst->getVideoTrackCount()  >= 2)
		{
		    m_isCheckingDirectExport = FALSE; //yoon
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  Track Count Check(%d %d)", 
				__LINE__, pFirst->getAudioTrackCount(), pFirst->getVideoTrackCount());
			return 15;
		}
		
		if( (option & DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL) == DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL ){
		    ;
		}
		else
		{ 
		    if( pFirst->isAudioExist() ){

			    if( pFirst->getAudioOnOff() == 0 )
			    {
				    iRet = 17;
				    break;
			    }

    		    if( !(pFirst->getCodecType(NXFF_MEDIA_TYPE_AUDIO) == eNEX_CODEC_A_AAC ||
    			  pFirst->getCodecType(NXFF_MEDIA_TYPE_AUDIO) == eNEX_CODEC_A_MP3) )
    		    {
    			    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport audio OTI(0x%x) not support", __LINE__, pFirst->getCodecType(NXFF_MEDIA_TYPE_AUDIO));
    			    iRet =  13;
    			    break;
    		    }
    		
    			#if 0
    				if( /*pFirst->getAudioChannels() < 1 ||*/ pFirst->getAudioChannels() > 2 )
    			{
    				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  Audio Channel Count Check(%d)",
    							__LINE__, pFirst->getAudioChannels());
    				iRet =  15;
    				break;
    			}
    			#endif
	        }
	    }

   		if( nRotateDegree == -1 )
	    {
		    nRotateDegree = pFirst->getContentOrientation();
	    }
	    else
	    {
		    if( nRotateDegree != pFirst->getContentOrientation() )
		    {
		        m_isCheckingDirectExport = FALSE; //yoon
			    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport diffrent orient (%d,%d)", __LINE__,nRotateDegree, pFirst->getContentOrientation());
			    return 14;
		    }
	    }

        if( pPrev != NULL ){
		    if( (option & DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL) == DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL )
		    {
                ;
		    }
		    else if( pFirst->compareAudioDSIofClip(pPrev) == FALSE )
    		{
    			iRet = 17;
    			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport compareAudioDSIofClip mismatch", __LINE__);
    			break;
    		}
    				
    		if( pFirst->compareVideoDSIofClip(pPrev) == FALSE )
    		{
    		    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport compareVideoDSIofClip mismatch", __LINE__);
    			iRet = 5;
    			break;
    		}
        }
        pPrev = pFirst;
        
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport clips.count(%d)=%d", __LINE__,index,clips.count(index));
		if( clips.count(index) == 1 ){
		    if( bNeedCheckEncoder == FALSE ){
        		if( pFirst->isStartIDRFrame() == FALSE )
        		{
        			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  IDR Check(%d)", __LINE__, pFirst->isStartIDRFrame());
        			bNeedCheckEncoder = TRUE;
        		}
    	    }   
    	    
            sub_range_pair keyRange = clips.equal_range(index);
            
            for (s_it = keyRange.first;  s_it != keyRange.second;  ++s_it)
            {
                int value = s_it->second;
            }
    	         
        }else{
				
            //open
            CClipItem *pIDRCheckItem = new CClipItem(INVALID_CLIP_ID);
            pIDRCheckItem = new CClipItem(INVALID_CLIP_ID);
            int OpenOk = 0;
            if( pIDRCheckItem != NULL)
    	    {
                NEXVIDEOEDITOR_ERROR eRet = (NEXVIDEOEDITOR_ERROR)pIDRCheckItem->parseClipFile(pFirst->getClipPath(), GET_CLIPINFO_CHECK_IDR_MODE);
                if( eRet == NEXVIDEOEDITOR_ERROR_NONE ){
                    OpenOk = 1;
                }
            }
            
            sub_range_pair keyRange = clips.equal_range(index);
            
            for (s_it = keyRange.first;  s_it != keyRange.second;  ++s_it)
            {
                int value = s_it->second;
                
           		if( m_isCancelDirectExport ){
		            m_isCancelDirectExport = FALSE;
		            m_isCheckingDirectExport = FALSE;
	                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  cancel", __LINE__);
	                return -1;
	            }
                
                
                
                if( OpenOk ){
                	pSecond = m_ClipItemVec[value];
                	if( pSecond == NULL )
									{
										m_isCancelDirectExport = FALSE;
		    						m_isCheckingDirectExport = FALSE; 
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport clip(%d) is 0", __LINE__,value);
										return 3;
									}
                	
                    if( pIDRCheckItem->isIDRFrame(pSecond->getStartTrimTime()) ){
               		    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  clip(%d) is IDRFrame(CTS:%u)", __LINE__
               		    ,value, pSecond->getStartTrimTime());			
											pSecond->setStartIDRFrame(TRUE);
                    }else{
               		    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  clip(%d) is not IDRFrame(CTS:%u)", __LINE__
               		    , value, pSecond->getStartTrimTime());			
											pSecond->setStartIDRFrame(FALSE);
                        bNeedCheckEncoder = TRUE;
                    }
                }
            }
            SAFE_RELEASE(pIDRCheckItem);
        }
    }  

	if( iRet == 0 && bNeedCheckEncoder )
	{
		if( checkDirectExportForEncoder() == FALSE )
			iRet = 11;
	}
	
	if( iRet == 0 )
	{
		m_isCheckDirectExport = TRUE;
	}
	
	m_isCheckingDirectExport = FALSE; //yoon 
	m_isCancelDirectExport = FALSE; //yoon
    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  iRet(%d) checked(%d)", __LINE__, iRet, m_isCheckDirectExport);
	return iRet;
}

void CClipList::cancelcheckDirectExport()
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] cancelcheckDirectExport(%d)", __LINE__,m_isCheckingDirectExport);

    if( m_isCheckingDirectExport ){
        m_isCancelDirectExport = TRUE;
    }
    
}

/*
	return val
		0 : Did not need audio Encode
		1 : Need audio Encode
		2 : Audio not exist
*/
		
int CClipList::checkAudioDirectExport()
{
	CClipItem*	pFirst				= NULL;
	CClipItem*	pSecond				= NULL;
	int			iRet					= 0;

	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  cliplist is 0", __LINE__);
		return 1;
	}

	if( m_pBackGround != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  has audio BGM", __LINE__);
		return 1;
	}

	if( m_AudioClipItemVec.size() > 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  has audio clips(%zu)", __LINE__, m_AudioClipItemVec.size());
		return 1;
	}
    
    std:map<const char *,int, _const_char_point_compare> clipmap;
    CClipItemVectorIterator i;
    int count = 0;
    std::set<const char *,_const_char_point_compare> setsAudioOff;
    std::set<const char *,_const_char_point_compare> setsSpeed;
	for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
	{
		pFirst = ((CClipItem*)(*i));
		if( pFirst == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport cliplist is 0", __LINE__);
			return 1;
		}

		if( pFirst->getAudioClipCount() > 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport has audio clip", __LINE__);
			return 1;
		}

		if( pFirst->getAudioOnOff() == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport Audio Off", __LINE__);
			setsAudioOff.insert(pFirst->getClipPath());
		}
		
		for(int j=0; j<pFirst->m_ClipAudioRenderInfoVec.size();j++)
		{
			if(pFirst->m_ClipAudioRenderInfoVec[j]->m_iSpeedCtlFactor != 100)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport use speed control", __LINE__);
				setsSpeed.insert(pFirst->getClipPath());
			}
		}
        
        if( clipmap.count(pFirst->getClipPath()) == 0 )
        {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport insert map(%s,%d)", __LINE__, pFirst->getClipPath(), count);
		    clipmap.insert(std::pair<const char *, int>(pFirst->getClipPath(),count ));
		}
		count++;
	}
    
    master_map_it mi;
    int lastAudioChannels = -1;
    for( mi = clipmap.begin() ; mi  != clipmap.end() ; mi++ ){
        int index = mi->second;
        pFirst = m_ClipItemVec[index];
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport check (%s,%d)", __LINE__,mi->first,mi->second );
        if( pFirst->isAudioExist() == 0 ){
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport not exist audio", __LINE__);
			return 2;
        }
        int audioChannels = pFirst->getAudioChannels();
        if( lastAudioChannels == -1 )
        {
        	lastAudioChannels = audioChannels;
        }
        else
        {
        	if( lastAudioChannels != audioChannels )
        	{
   					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  Audio Channel diff (%d,%d)",
							__LINE__, audioChannels, lastAudioChannels);

        		return 1;	
        	}
        }
        
        if( /*(audioChannels > 2) &&*/ (audioChannels< 1) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  Audio Channel Count Check(%d)",
							__LINE__, audioChannels);
			return 1;
    	}
        
        if( setsAudioOff.count(pFirst->getClipPath()) > 0 ){
            return 2;
        }
        
        if( setsSpeed.count(pFirst->getClipPath()) > 0 ){
            return 1;
        }
        
        if( pSecond != NULL ){
            if( pFirst->compareAudioDSIofClip(pSecond) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport DSI unmatch", __LINE__);
				iRet = 1;
				break;
			}
        }
        pSecond = pFirst;
    }

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioDirectExport  Match(%d)", __LINE__, iRet);
	return iRet;
}

static unsigned char g_p1s_dsiInfo_3840_2160[] = {0xE1, 0x00, 0x0F, 0x67, 0x64, 0x00, 0x33, 0xAC, 0xD2, 0x00, 0xF0, 0x01, 0x0F, 0xA0, 0x1B, 0x42, 0x84, 0xD4, 0x01, 0x00, 0x05, 0x68, 0xEE, 0x06, 0xE2, 0xC0  };
static unsigned char g_p1s_dsiInfo_1920_1080[] = {0xE1, 0x00, 0x10, 0x67, 0x64, 0x00, 0x28, 0xAC, 0xD2, 0x01, 0xE0, 0x08, 0x9F, 0x96, 0x01, 0xB4, 0x28, 0x4D, 0x40, 0x01, 0x00, 0x05, 0x68, 0xEE, 0x06, 0xE2, 0xC0};
static unsigned char g_p1s_dsiInfo_1280_720[] = {0xE1, 0x00, 0x0D, 0x67, 0x42, 0x80, 0x1F, 0xDA, 0x01, 0x40, 0x16, 0xE8, 0x06, 0xD0, 0xA1, 0x35, 0x01, 0x00, 0x04, 0x68, 0xCE, 0x06, 0xE2};
static unsigned char g_p1s_dsiInfo_1280_720_120[] = {0xE1, 0x00, 0x0D, 0x67, 0x42, 0x80, 0x2A, 0xDA, 0x01, 0x40, 0x16, 0xE8, 0x06, 0xD0, 0xA1, 0x35, 0x01, 0x00, 0x04, 0x68, 0xCE, 0x06, 0xE2};

NXBOOL CClipList::checkDirectExportForEncoder()
{
        //yoon for LG
    if( CNexVideoEditor::m_bForceDirectExport )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExportForEncoder  ForceDirectExport is true", __LINE__);
        return TRUE;
    }
    

	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExportForEncoder  cliplist is 0", __LINE__);
		return FALSE;
	}

	CClipItem*	pItem = ((CClipItem*)(*m_ClipItemVec.begin()));
	if( pItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExportForEncoder  first clip is null", __LINE__);
		return FALSE;
	}

	// need compare 

	NXBOOL bRet = pItem->compareVideoDSIWithEncoder();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkDirectExport  bRet(%d)", __LINE__, bRet);
	return bRet;
}

int CClipList::clearIDRFlagForDirectExport()
{
	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] setIDRFlagForDirectExport  cliplist is 0", __LINE__);
		return 1;
	}

	for(int i = 0; i < iClipVecSize; i++)
	{
		m_ClipItemVec[i]->setStartIDRFrame(FALSE);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] setIDRFlagForDirectExport", __LINE__);
	return 0;	
}

int CClipList::setIDRFlagForDirectExport()
{
	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] setIDRFlagForDirectExport  cliplist is 0", __LINE__);
		return 1;
	}

	for(int i = 0; i < iClipVecSize; i++)
	{
		m_ClipItemVec[i]->setStartIDRFrame(TRUE);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] setIDRFlagForDirectExport", __LINE__);
	return 0;	
}

void CClipList::setSeparateEffect(NXBOOL bSeparate)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setSeparateEffect(%d)", __LINE__, bSeparate);
	m_bSeparateEffect = bSeparate;
}

CVideoEffectItemVec* CClipList::getEffectItemVec()
{
	SAFE_ADDREF(m_pEffectItemVec);
	return m_pEffectItemVec;
}

CClipItem* CClipList::getVisualClipUsingID(unsigned int uiClipID)
{
	CClipItem* pItem	= NULL;
	CClipItem* pItemTmp	= NULL;
	CClipItemVectorIterator i;
	for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
	{
		pItem = (CClipItem*)(*i);
		if( pItem )
		{
			if( pItem->getClipID() == uiClipID )
			{
				pItem->AddRef();
				return pItem;
			}
		}
	}
	return NULL;
}

CClipItem* CClipList::getAudioClipUsingID(unsigned int uiClipID)
{
	CClipItem* pItemTmp	= NULL;
	for(int i = 0; i < m_ClipItemVec.size(); i++)
	{
		pItemTmp = m_ClipItemVec[i]->getAudioClipUsingClipID(uiClipID);
		if( pItemTmp )
		{
			return pItemTmp;
		}
	}
	return NULL;
}

CClipItem* CClipList::getVisualClipUsingTime(unsigned int uiTime)
{
	for(int i = 0; i < m_ClipItemVec.size(); i++)
	{
		if( uiTime >= m_ClipItemVec[i]->getStartTime() && uiTime < m_ClipItemVec[i]->getEndTime() )
		{
			m_ClipItemVec[i]->AddRef();
			return m_ClipItemVec[i];
		}

		if( i == (m_ClipItemVec.size() -1) )
		{
			m_ClipItemVec[i]->AddRef();
			return m_ClipItemVec[i];
		}
	}
	return NULL;
}

CClipItem* CClipList::getVisualClipUsingTimeAppliedEffect(unsigned int uiTime)
{
	for(int i = 0; i < m_ClipItemVec.size(); i++)
	{
		unsigned int uiStartTime	= m_ClipItemVec[i]->getStartTime();
		unsigned int uiEndTime	= m_ClipItemVec[i]->getEndTime() - (m_ClipItemVec[i]->getClipEffectDuration() * m_ClipItemVec[i]->getClipEffectOverlap() / 100);

		if( uiTime >= uiStartTime && uiTime < uiEndTime )
		{
			m_ClipItemVec[i]->AddRef();
			return m_ClipItemVec[i];
		}

		if( i == (m_ClipItemVec.size() -1) )
		{
			m_ClipItemVec[i]->AddRef();
			return m_ClipItemVec[i];
		}
	}
	return NULL;
}

int CClipList::getBGMVolumeOfVisualClipAtCurrentTime(unsigned int uiTime)
{
	int iVolume = 100;
    int i, j;
	for(i = 0; i < m_ClipItemVec.size(); i++)
	{
		unsigned int uiStartTime	= m_ClipItemVec[i]->getStartTime();
//		unsigned int uiEndTime	= m_ClipItemVec[i]->getEndTime() - (m_ClipItemVec[i]->getClipEffectDuration() * m_ClipItemVec[i]->getClipEffectOverlap() / 100);
                unsigned int uiEndTime	= m_ClipItemVec[i]->getEndTime();

		if( uiTime >= uiStartTime && uiTime < uiEndTime )
		{
			CClipItem* pClipItem = m_ClipItemVec[i];
			for(j = 0; j < pClipItem->m_ClipAudioRenderInfoVec.size(); j++)
			{
				if( pClipItem->m_ClipAudioRenderInfoVec[j]->mStartTime <= uiTime && pClipItem->m_ClipAudioRenderInfoVec[j]->m_uiBGMEndTIme > uiTime )
				{
					iVolume = pClipItem->m_ClipAudioRenderInfoVec[j]->m_iBackGroundVolume;
					return iVolume;
				}
			}
		}

		if( i == (m_ClipItemVec.size()-1 ))
		{
			int index = m_ClipItemVec[i]->m_ClipAudioRenderInfoVec.size();
			if( index > 0 )
				iVolume = m_ClipItemVec[i]->m_ClipAudioRenderInfoVec[index -1]->m_iBackGroundVolume;
		}
	}
	return iVolume;
}

NXBOOL CClipList::checkAudioClipCount(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int* pEndTime)
{
	int iAudioClipCount = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioClipCount (S:%d E:%d)", __LINE__, uiStartTime, uiEndTime);
	for( ; uiStartTime < uiEndTime; uiStartTime += 100 )
	{
		for( int i = 0; i < m_ClipItemVec.size(); i++)
		{
			iAudioClipCount += m_ClipItemVec[i]->getAudioClipCountAtTime(uiStartTime);
		}
		if( iAudioClipCount >= 3 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioClipCount return FALSE(StartTime : %d AudioCount:%d)", __LINE__, uiStartTime, iAudioClipCount);
			*pEndTime = uiStartTime;
			return FALSE;
		}
		iAudioClipCount = 0;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] checkAudioClipCount return TRUE(%d)", __LINE__, iAudioClipCount);
	return TRUE;
}

int	CClipList::addVisualClip(unsigned int uiNextToClipID, CClipItem* pItem)
{
	m_isCheckDirectExport = FALSE;
	
	if( pItem == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	
	if( uiNextToClipID == 0 )
	{
		pItem->AddRef();
		m_ClipItemVec.insert(m_ClipItemVec.begin(), pItem);
	}
	else
	{
		CClipItemVectorIterator i;
		for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
		{
			if( uiNextToClipID == ((CClipItem*)(*i))->getClipID() )
			{
				i++;
				break;
			}
		}
		pItem->AddRef();
		m_ClipItemVec.insert(i, pItem);
	}
	reCalcTime();
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int	CClipList::addAudioClip(unsigned int uiStartTime, CClipItem* pItem)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;

	if( (uiStartTime + 500 ) > m_uiTotalTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip failed because do not exist audio clip slot", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	unsigned int uiEndTime;
	if( checkAudioClipCount(uiStartTime, uiStartTime + pItem->getTotalTime(), &uiEndTime) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip Slot is not available.", __LINE__);
		uiEndTime -= 200;

		uiEndTime = pItem->getTotalTime() - (uiEndTime - uiStartTime);
		pItem->setEndTrimTime(uiEndTime);

		int iPlayTime =  pItem->getTotalTime() - ( pItem->getStartTrimTime() + pItem->getEndTrimTime() );
		if( iPlayTime < 500 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip failed because do not exist audio clip slot and playTime(%d)", __LINE__, iPlayTime);
			return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip forced setting end trim time(%d).", __LINE__, uiEndTime);
	}
	
	CClipItem* pVisualItem = getVisualClipUsingTime(uiStartTime);
	if( pVisualItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip failed because did not find visual clip(%d)", __LINE__, uiStartTime);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	unsigned int uiVisualStartTime = pVisualItem->getStartTime();

	uiStartTime = uiStartTime - uiVisualStartTime;
	pItem->setStartTime(uiStartTime);
	pItem->setEndTime(uiStartTime + pItem->getTotalTime());

	eRet = (NEXVIDEOEDITOR_ERROR)pVisualItem->addAudioClip(pItem);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] addAudioClip is failed(0x%x)", __LINE__, eRet);
		SAFE_RELEASE(pVisualItem);
		return eRet;
	}
	SAFE_RELEASE(pVisualItem);
	reCalcTime();
	return eRet;
}

int	CClipList::deleteClipItem(unsigned int uiClipID)
{
	CClipItemVectorIterator i;
	CClipItem* pVisualItem = NULL;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] deleteClipItem ClipID(%d)", __LINE__, uiClipID);
	for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
	{
		pVisualItem = (CClipItem*)(*i);
		if( pVisualItem == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] deleteClipItem Visual Clip is null", __LINE__);
			continue;
		}

		if( pVisualItem->deleteAudioClip(uiClipID) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] This clip is audio and delete done", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		
		if( uiClipID == pVisualItem->getClipID() )
		{
			m_ClipItemVec.erase(i);
			pVisualItem->Release();
			reCalcTime();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] This clip is video and delete done", __LINE__);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
}

int CClipList::setBackGroundMusic(CClipItem* pItem)
{
	CAutoLock m(m_ClipListLock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setBackGroundMusic In(%p)", __LINE__, pItem);
	SAFE_RELEASE(m_pBackGround);
	m_pBackGround = pItem;
	if( m_pBackGround )
	{
		m_pBackGround->setAudioOnOff(TRUE);
		m_pBackGround->setStartTime(0);
		m_pBackGround->setEndTime(m_uiTotalTime);

		unsigned int tmpArrayTime[4];
		unsigned int tmpArrayVol[4] = {0, 100, 100, 0};

		tmpArrayTime[0] = 0;
		tmpArrayTime[1] = m_iBGMFadeInTime;
		tmpArrayTime[2] = m_uiTotalTime - m_iBGMFadeOutTime;
		tmpArrayTime[3] = m_uiTotalTime;

		m_pBackGround->setAudioEnvelop(4, tmpArrayTime, tmpArrayVol);
		
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipList::setBackGroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime)
{
	CAutoLock m(m_ClipListLock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setBackGroundMusicVolume In(%d %d %d)", __LINE__, iVolume, iFadeInTime, iFadeOutTime);
	if( m_pBackGround )
	{
		m_pBackGround->setAudioVolume(iVolume);

		unsigned int tmpArrayTime[4];
		unsigned int tmpArrayVol[4] = {0, 100, 100, 0};

		m_iBGMFadeInTime	= iFadeInTime;
		m_iBGMFadeOutTime	= iFadeOutTime;
		
		tmpArrayTime[0] = 0;
		tmpArrayTime[1] = m_iBGMFadeInTime;
		tmpArrayTime[2] = m_uiTotalTime - m_iBGMFadeOutTime;
		tmpArrayTime[3] = m_uiTotalTime;
		m_pBackGround->setAudioEnvelop(4, tmpArrayTime, tmpArrayVol);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setBackGroundMusicVolume Out(%d)", __LINE__, iVolume);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setBackGroundMusic did not set", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipList::moveVisualClip(unsigned int uiNextToClipID, unsigned int uimovedClipID)
{
	m_isCheckDirectExport = FALSE;
	
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	CClipItem* pItem = getVisualClipUsingID(uimovedClipID);
	if( pItem == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	eRet = (NEXVIDEOEDITOR_ERROR)deleteClipItem(uimovedClipID);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(pItem);
		return eRet;
	}

	eRet = (NEXVIDEOEDITOR_ERROR)addVisualClip(uiNextToClipID, pItem);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(pItem);
		return eRet;
	}
	pItem->printClipInfo();
	SAFE_RELEASE(pItem);
	return eRet;
}

int CClipList::moveAudioClip(unsigned int uiMoveTime, unsigned int uimovedClipID)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	CClipItem* pItem	= NULL;
	CClipItem* pItemTmp	= NULL;
	pItem = getVisualClipUsingTime(uiMoveTime);
	if( pItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] moveAudioClip getVisual Clip Err(Clip Time:%d)", __LINE__, uiMoveTime);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	pItemTmp = getAudioClipUsingID(uimovedClipID);
	if( pItemTmp == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] moveAudioClip getAudio Clip Err(Clip ID:%d)", __LINE__, uimovedClipID);
		SAFE_RELEASE(pItem);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	eRet = (NEXVIDEOEDITOR_ERROR)deleteClipItem(uimovedClipID);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] moveAudioClip Delete Audio Clip Err(Clip ID:%d)", __LINE__, uimovedClipID);
		SAFE_RELEASE(pItem);
		SAFE_RELEASE(pItemTmp);
		return eRet;
	}

	unsigned int uiVisualStartTime = pItem->getStartTime();

	uiMoveTime = uiMoveTime - uiVisualStartTime;
	
	unsigned int uiStartTime = pItemTmp->getStartTime();
	unsigned int uiEndTime = pItemTmp->getEndTime();
	uiEndTime = uiEndTime - uiStartTime;

	pItemTmp->setStartTime(uiMoveTime);
	pItemTmp->setEndTime(uiMoveTime + uiEndTime);
	
	eRet = (NEXVIDEOEDITOR_ERROR)pItem->addAudioClip(pItemTmp);
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] moveAudioClip Add Audio Clip Err(Clip ID:%d)", __LINE__, uimovedClipID);
		SAFE_RELEASE(pItem);
		SAFE_RELEASE(pItemTmp);
		return eRet;
	}

	pItemTmp->printClipInfo();

	SAFE_RELEASE(pItemTmp);
	SAFE_RELEASE(pItem);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] moveAudioClip Add Audio Clip Done(Clip ID:%d)", __LINE__, uimovedClipID);
	return eRet;
}

void CClipList::resetClipReader()
{
	for(int i = 0; i < m_ClipItemVec.size(); i++)
	{
		m_ClipItemVec[i]->deinitSource();
	}
#ifdef FOR_LOADLIST_FEATURE
	for(int i=0; i < m_AudioClipItemVec.size(); i++)
	{
		m_AudioClipItemVec[i]->deinitSource();
	}
#endif	
#ifdef FOR_LAYER_FEATURE
	for(int i=0; i < m_LayerItemVec.size(); i++)
	{
		m_LayerItemVec[i]->deinitSource();
	}
#endif
}

unsigned int CClipList::getClipTotalTime()
{
	return m_uiTotalTime;
}

NXBOOL CClipList::updatePlay(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	NXBOOL isFail = TRUE;
	if( uiTime >= m_uiTotalTime )
	{
		return stopPlay();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] updatePlay(Clip Count : %zu, Time : %d %p %p) BGM(%p)", 
			__LINE__, m_ClipItemVec.size(), uiTime, pVideoRender, pAudioRender, m_pBackGround);

	if( m_pBackGround )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] background music info(%p) startTime(%d) endTime(%d) CurrentTime(%d)", 
				__LINE__, m_pBackGround, m_pBackGround->getStartTime(), m_pBackGround->getEndTime(), uiTime);
	
		if( m_pBackGround->isPlayTime(uiTime, false) )
		{
			m_pBackGround->startPlayAudio(uiTime, pAudioRender, TRUE);
		}
		else
		{
			m_pBackGround->stopPlay();
		}
	}
	
// KM-3414 do audio first
#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		m_AudioClipItemVec[i]->startPlayAudio(uiTime, pAudioRender, FALSE);
	}
#endif	

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->updatePlayTime(uiTime, pVideoRender, pAudioRender) == FALSE )
			isFail = FALSE;
	}

#ifdef FOR_LAYER_FEATURE
	for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
	{
		m_LayerItemVec[i]->updatePlayTime(uiTime, pVideoRender, pAudioRender);
	}
#endif	
	
	return isFail;
}

NXBOOL CClipList::updatePlay(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData, int option)
{
	NXBOOL isFail = TRUE;
	if( uiTime >= m_uiTotalTime )
	{
		return stopPlay();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] updatePlay with FileWriter(Clip Count : %zu, Time : %d %p %p) BGM(%p)", 
			__LINE__, m_ClipItemVec.size(), uiTime, pAudioRender, pFileWriter, m_pBackGround);

	if( m_pBackGround )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] background music info(%p) startTime(%d) endTime(%d) CurrentTime(%d)", 
				__LINE__, m_pBackGround, m_pBackGround->getStartTime(), m_pBackGround->getEndTime(), uiTime);
	
		if( m_pBackGround->isPlayTime(uiTime, false) )
		{
			m_pBackGround->startPlayAudio(uiTime, pAudioRender, TRUE);
		}
		else
		{
			m_pBackGround->stopPlay();
		}
	}

	if( (option & DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL) == DIRECTEXPORT_FLAG_SUPPORT_SPEED_CTL )
	{
		for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
		{
			if( m_ClipItemVec[i]->updatePlayTime(uiTime, pAudioRender, pFileWriter, pUserData) == FALSE )
				isFail = FALSE;
		}
	}
	else
	{
		if( m_iDirectExportIndex < (int)m_ClipItemVec.size() )
		{
			int iRet = m_ClipItemVec[m_iDirectExportIndex]->playVideoForDirectExport(pFileWriter, pUserData);
			if( iRet == 0 )
			{
			}
			else if( iRet == 1 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] One clip item end(Index:%d)", __LINE__, m_iDirectExportIndex);
				m_iDirectExportIndex++;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] Clip de failed(%d)  (Index:%d)", __LINE__, iRet, m_iDirectExportIndex);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] Eirect export End(Index:%d)", __LINE__, m_iDirectExportIndex);
		}
	}

#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		m_AudioClipItemVec[i]->startPlayAudio(uiTime, pAudioRender, FALSE);
	}
#endif	

#ifdef FOR_LAYER_FEATURE
	for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
	{
		m_LayerItemVec[i]->updatePlayTime(uiTime, pVideoRender, pAudioRender);
	}
#endif	
	
	return isFail;
}

NXBOOL CClipList::updatePlay4Export(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	NXBOOL isFail = TRUE;
	if( uiTime >= m_uiTotalTime )
	{
		return stopPlay();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] updatePlay4Export(Clip Count : %zu, Time : %d %p %p) BGM(%p)", 
			__LINE__, m_ClipItemVec.size(), uiTime, pVideoRender, pAudioRender, m_pBackGround);

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->updatePlay4Export(uiTime, pVideoRender, pAudioRender) == FALSE )
			isFail = FALSE;
	}

	return isFail;
}

NXBOOL CClipList::stopPlay()
{
	if( m_pBackGround )
	{
		m_pBackGround->stopPlay();
	}
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		m_ClipItemVec[i]->stopPlay();
	}
#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		m_AudioClipItemVec[i]->stopPlay();
	}
#endif
#ifdef FOR_LAYER_FEATURE
	for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
	{
		m_LayerItemVec[i]->stopPlay();
	}
#endif
	m_iDirectExportIndex = 0;
	m_uVisualIndex = 0;
	return TRUE;
}

NXBOOL CClipList::pausePlay()
{
	NXBOOL isFail = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->pausePlayTime() == FALSE )
			isFail = FALSE;
	}

#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		if( m_AudioClipItemVec[i]->pausePlayTime() == FALSE )
			isFail = FALSE;
	}
#endif	

	return isFail;
}

NXBOOL CClipList::resumePlay(CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	NXBOOL isFail = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->resumePlayTime(pVideoRender, pAudioRender) == FALSE )
			isFail = FALSE;
	}

#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		if( m_AudioClipItemVec[i]->resumePlayTime(pVideoRender, pAudioRender) == FALSE )
			isFail = FALSE;
	}
#endif	

	return TRUE;
}

NXBOOL CClipList::setPreviewTime(unsigned int uiTime, int iIDRFrame, CNEXThreadBase* pVideoRander)
{
	NXBOOL bRet = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->isPlayTime(uiTime) )
		{
			if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_IMAGE )
			{
				if( FALSE == m_ClipItemVec[i]->setPreviewImage(uiTime, (CNEXThreadBase*)pVideoRander))
				{
					bRet = FALSE;
				}
			}
			else if( (m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER ))
			{
				if( m_ClipItemVec[i]->setPreviewVideo(uiTime, iIDRFrame, (CNEXThreadBase*)pVideoRander) == FALSE ) {
					bRet = FALSE;
				}
			}
			m_uVisualIndex = i;
		}
		else
		{
			m_ClipItemVec[i]->unsetPreview();
		}
	}
#ifdef FOR_LAYER_FEATURE
	for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
	{
		if( m_LayerItemVec[i]->isPlayTime(uiTime) )
		{
			m_LayerItemVec[i]->setPreview(uiTime, (CNEXThreadBase*)pVideoRander);
		}
		else
		{
			m_LayerItemVec[i]->unsetPreview();
		}
	}
#endif

	return bRet;
}
NXBOOL CClipList::clearImageTask()
{
	NXBOOL bRet = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_IMAGE )
		{
			if (FALSE == m_ClipItemVec[i]->clearImageTask())
			{
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

unsigned char* CClipList::getPreviewThumbData(unsigned int uiTime)
{
	unsigned char* pData = NULL;
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->isPlayTime(uiTime) )
		{
			if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER )
			{
				pData = m_ClipItemVec[i]->getPreviewThumbData(uiTime);
				if(pData)
					return pData;
			}
		}
	}
	return pData;

}

NXBOOL CClipList::setPreviewThumb(unsigned int uiTime, void* pThumbRender, void* pOutputSurface, CNEXThreadBase* pVideoRander)
{
	NXBOOL bRet = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->isPlayTime(uiTime) )
		{
			if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER )
			{
				m_ClipItemVec[i]->setPreviewThumb(uiTime, pThumbRender, pOutputSurface, pVideoRander);
			}
		}
		else
		{
			m_ClipItemVec[i]->unsetPreviewThumb();
		}
	}
	return bRet;
}

NXBOOL CClipList::unsetPreviewThumb()
{
	NXBOOL bRet = TRUE;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER )
		{
			m_ClipItemVec[i]->unsetPreviewThumb();
		}
	}
	return bRet;
}

NXBOOL CClipList::isAudioClipTime(unsigned int uiClipID, unsigned int uiTime)
{
	CClipItem* pVisualClip = NULL;
	CClipItem* pAudioClip = NULL;

	if( m_pBackGround && m_pBackGround->getClipID() == uiClipID )
		return TRUE;
	
#ifdef FOR_LOADLIST_FEATURE
	for(int i = 0; i < (int)m_AudioClipItemVec.size(); i++)
	{
		if( m_AudioClipItemVec[i]->getClipID() == uiClipID )
		{
			NXBOOL bPlayTime = m_AudioClipItemVec[i]->isPlayTimeAudio(0, uiTime);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] find audioClip(id:%d, isplay:%d)", __LINE__, uiClipID, bPlayTime);
			return bPlayTime;
		}
	}
#endif
	
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		for( int j = 0; j < m_ClipItemVec[i]->m_AudioClipVec.size(); j++)
		{
			if( m_ClipItemVec[i]->m_AudioClipVec[j]->getClipID() == uiClipID )
			{
				pVisualClip = m_ClipItemVec[i];
				pAudioClip = m_ClipItemVec[i]->m_AudioClipVec[j];
				break;
			}
		}
	}
	if( pVisualClip && pAudioClip )
	{
		NXBOOL bPlayTime = pAudioClip->isPlayTimeAudio(pVisualClip->getStartTime(), uiTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] find audioClip(id:%d, isplay:%d)", __LINE__, uiClipID, bPlayTime);
		return bPlayTime;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] Did not find audioClip(id:%d)", __LINE__, uiClipID);
	}
	return FALSE;
}

void CClipList::updateClipInfo(IClipItem* pItem)
{

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] updateClipInfo In(%p)", __LINE__, pItem);	
	
	if( pItem == NULL ) return;

	IClipItem* pClip = getClipUsingID((unsigned int)pItem->getClipID());
	if( pClip == NULL ) return;

	pClip->printClipInfo();

	switch(pClip->getClipType())
	{
		case CLIPTYPE_IMAGE:
			pClip->setTotalTime(pItem->getTotalTime());
			pClip->setClipPath(pItem->getClipPath());
			break;
		case CLIPTYPE_VIDEO:
			pClip->setStartTrimTime(pItem->getStartTrimTime());
			pClip->setEndTrimTime(pItem->getEndTrimTime());
			break;
		case CLIPTYPE_VIDEO_LAYER:
			pClip->setStartTrimTime(pItem->getStartTrimTime());
			pClip->setEndTrimTime(pItem->getEndTrimTime());
			break;
		case CLIPTYPE_AUDIO:
			pClip->setStartTime(pItem->getStartTime());
			pClip->setEndTime(pItem->getEndTime());
			pClip->setStartTrimTime(pItem->getStartTrimTime());
			pClip->setEndTrimTime(pItem->getEndTrimTime());
			pClip->setAudioOnOff(pItem->getAudioOnOff());
			pClip->setAudioVolume(pItem->getAudioVolume());
			pClip->setPanLeftFactor(pItem->getPanLeftFactor());
			pClip->setPanRightFactor(pItem->getPanRightFactor());
			pClip->setEnhancedAudioFilter(pItem->getEnhancedAudioFilter());
			pClip->setEqualizer(pItem->getEqualizer());
#ifdef FOR_TEST_AUDIOENVELOP
			// RYU 20130604 add AudioEnvelop.
			{
				unsigned int* puiTime = NULL;
				unsigned int* puiVolume = NULL;
				int			nSize = 0;
				pItem->getAudioEnvelop(&nSize, &puiTime, &puiVolume);
				pClip->setAudioEnvelop(nSize, puiTime, puiVolume);
				pClip->setAudioEnvelopOnOff(pItem->isAudioEnvelopEnabled());
			}
#endif
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] updateClipInfo Out(%p)", __LINE__, pItem);	
			return;
		default:
			break;
	};

	if( pClip->isAudioExist() )
	{
		pClip->setAudioOnOff(pItem->getAudioOnOff());
		pClip->setAudioVolume(pItem->getAudioVolume());
		pClip->setBGMVolume(pItem->getBGMVolume());
		pClip->setPanLeftFactor(pItem->getPanLeftFactor());
		pClip->setPanRightFactor(pItem->getPanRightFactor());
#ifdef FOR_TEST_AUDIOENVELOP
		// RYU 20130604 add AudioEnvelop.
		{
			unsigned int* puiTime = NULL;
			unsigned int* puiVolume = NULL;
			int			nSize = 0;
			pItem->getAudioEnvelop(&nSize, &puiTime, &puiVolume);
			pClip->setAudioEnvelop(nSize, puiTime, puiVolume);
			pClip->setAudioEnvelopOnOff(pItem->isAudioEnvelopEnabled());
		}
#endif
	}

	pClip->setClipEffectDuration(pItem->getClipEffectDuration());
	pClip->setClipEffectOffset(pItem->getClipEffectOffset());
	pClip->setClipEffectOverlap(pItem->getClipEffectOverlap());
	
	pClip->setClipEffectID(pItem->getClipEffectID());
	pClip->setTitleEffectID(pItem->getTitleEffectID());
	pClip->setFilterID(pItem->getFilterID());

	pClip->setRotateState(pItem->getRotateState());
	pClip->setBrightness(pItem->getBrightness());
	pClip->setContrast(pItem->getContrast());
	pClip->setSaturation(pItem->getSaturation());
	pClip->setTintcolor(pItem->getTintcolor());
	pClip->setLUT(pItem->getLUT());
	pClip->setCustomLUTA(pItem->getCustomLUTA());
	pClip->setCustomLUTB(pItem->getCustomLUTB());
	pClip->setCustomLUTPower(pItem->getCustomLUTPower());
	pClip->setVignette(pItem->getVignette());
	pClip->setSpeedCtlFactor(pItem->getSpeedCtlFactor());
	pClip->setVoiceChangerFactor(pItem->getVoiceChangerFactor());
	pClip->setCompressorFactor(pItem->getCompressorFactor());
	pClip->setPitchFactor(pItem->getPitchFactor());
	pClip->setKeepPitch(pItem->getKeepPitch());
	pClip->setMusicEffector(pItem->getMusicEffector());
	pClip->setProcessorStrength(pItem->getProcessorStrength());
	pClip->setBassStrength(pItem->getBassStrength());
	pClip->setSlowMotion(pItem->getSlowMotion());

	pClip->setEnhancedAudioFilter(pItem->getEnhancedAudioFilter());
	pClip->setEqualizer(pItem->getEqualizer());

	pClip->setTitleStyle(pItem->getTitleStyle());
	pClip->setTitleStartTime(pItem->getTitleStartTime());
	pClip->setTitleEndTime(pItem->getTitleEndTime());
	pClip->setTitle(pItem->getTitle());

	IRectangle* pSrcStartRect = pItem->getStartPosition();	
	IRectangle* pSrcEndtRect = pItem->getEndPosition();	
	IRectangle* pSrcDstRect = pItem->getDstPosition();	
	
	IRectangle* pDstStartRect = pClip->getStartPosition();	
	IRectangle* pDstEndtRect = pClip->getEndPosition();	
	IRectangle* pDstDstRect = pClip->getDstPosition();

	if( pSrcStartRect && pDstStartRect )
	{
		pDstStartRect->setRect(pSrcStartRect->getLeft(), pSrcStartRect->getTop(), pSrcStartRect->getRight(), pSrcStartRect->getBottom());
	}

	if( pSrcEndtRect && pDstEndtRect )
	{
		pDstEndtRect->setRect(pSrcEndtRect->getLeft(), pSrcEndtRect->getTop(), pSrcEndtRect->getRight(), pSrcEndtRect->getBottom());
	}

	if( pSrcDstRect && pDstDstRect )
	{
		pDstDstRect->setRect(pSrcDstRect->getLeft(), pSrcDstRect->getTop(), pSrcDstRect->getRight(), pSrcDstRect->getBottom());
	}

	SAFE_RELEASE(pSrcStartRect);
	SAFE_RELEASE(pSrcEndtRect);
	SAFE_RELEASE(pSrcDstRect);
	SAFE_RELEASE(pDstStartRect);
	SAFE_RELEASE(pDstEndtRect);
	SAFE_RELEASE(pDstDstRect);

	pClip->printClipInfo();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] updateClipInfo Out(%p)", __LINE__, pItem);	
}

void CClipList::reCalcTime()
{
	unsigned int	uiCurrentStartTime	= 0;
	unsigned int	uiCurrentEndTime	= 0;
	unsigned int	uiCurrentItemTime	= 0;
	unsigned int	uiStartTrimTime		= 0;
	unsigned int	uiEndTrimTime		= 0;
	CClipItem*	pItem				= NULL;
	unsigned int	uiTotalPlayTime		= 0;
	unsigned int	uiEffectMargin		= 0;
	unsigned int	uiTitleTime			= 0;
	unsigned int	uiClipIndex			= 0;

	m_uiTotalTime					= 0;

	m_isCheckDirectExport				= FALSE;

	CAutoLock m(m_ClipListLock);
	//pEffectItemVec->clearEffectItem();
	
	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 ) return;

	CClipItemVectorIterator i;
	for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++, uiClipIndex++)
	{
		pItem = ((CClipItem*)(*i));
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] reCalcTime(pItem:%p)", __LINE__, pItem);
		if( pItem && (pItem->getClipType() == CLIPTYPE_IMAGE || pItem->getClipType() == CLIPTYPE_VIDEO) )
		{
			int iEffectDuration		= pItem->getClipEffectDuration();
			int iEffectOverlap		= pItem->getClipEffectOverlap();
			int iEffectOffset		= pItem->getClipEffectOffset();
			
			uiCurrentItemTime		= pItem->getTotalTime();
			uiStartTrimTime			= pItem->getStartTrimTime();
			uiEndTrimTime			= pItem->getEndTrimTime();

			uiCurrentItemTime -= (uiStartTrimTime + uiEndTrimTime);
			
			int iSpeedFactor = pItem->getSpeedCtlFactor();
			if( iSpeedFactor != 100 )
			{
				uiCurrentItemTime = uiCurrentItemTime * 100 / iSpeedFactor;
			}

			uiCurrentEndTime	= uiCurrentStartTime + uiCurrentItemTime;

			pItem->setStartTime(uiCurrentStartTime);
			pItem->setEndTime(uiCurrentEndTime);
			pItem->setClipEffectEnable(TRUE);

			pItem->setTitleStartTime(uiTitleTime);
			uiEffectMargin		= iEffectDuration * iEffectOverlap / 100;
			uiTitleTime		= iEffectDuration * iEffectOffset / 100;
			
			pItem->setTitleEndTime(uiCurrentEndTime - uiTitleTime);
			uiTitleTime = uiCurrentEndTime - uiTitleTime + iEffectDuration;
			
			m_uiTotalTime			= uiCurrentEndTime;
			uiCurrentStartTime		= m_uiTotalTime - uiEffectMargin;


			if( m_bSeparateEffect && ((uiClipIndex + 1 ) < iClipVecSize) )
			{
				CVideoEffectItem* pEffectItem = new CVideoEffectItem();
				if( pEffectItem )
				{
					unsigned int uiEffectStartTime = uiCurrentEndTime - (iEffectDuration * iEffectOffset / 100);

					pEffectItem->setEffectInfo(pItem->getStartTime(), pItem->getEndTime(), uiEffectStartTime, iEffectDuration, iEffectOffset, iEffectOverlap, pItem->getClipEffectID(), pItem->getTitle(), uiClipIndex, iClipVecSize);
					m_pEffectItemVec->lock();
					m_pEffectItemVec->addEffectItem(pEffectItem);
					m_pEffectItemVec->unlock();
					SAFE_RELEASE(pEffectItem);
				}
			}

			pItem->setClipIndexInfo(uiClipIndex);
			pItem->setClipTotalCount(iClipVecSize);
			pItem->printClipTime();
		}
		else if( pItem && (pItem->getClipType() == CLIPTYPE_VIDEO_LAYER) )
		{
			pItem->setClipIndexInfo(uiClipIndex);
		}
	}

	// ------- DISABLE THIS TO PREVENT CUTTING AUDIO CLIPS TO FIT TIMELINE -----------------
	/*for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
	{
		pItem = ((CClipItem*)(*i));
		if( pItem && (pItem->getClipType() == CLIPTYPE_IMAGE || pItem->getClipType() == CLIPTYPE_VIDEO) )
		{
			pItem->setAudioClipEndTime(getClipTotalTime());
		}
		else
		{
			char* p = NULL;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] This case is wrong so program currupt hang up", __LINE__);
			*p  = 100;
		}
	}	*/
	// --------------------------------------------------------------------------------------
 
	m_ClipItemVec[iClipVecSize -1]->setClipEffectEnable(FALSE);
	m_ClipItemVec[iClipVecSize -1]->setTitleEndTime(m_uiTotalTime);

	if( m_pBackGround )
	{
		m_pBackGround->setEndTime(m_uiTotalTime);

		unsigned int tmpArrayTime[4];
		unsigned int tmpArrayVol[4] = {0, 100, 100, 0};

		tmpArrayTime[0] = 0;
		tmpArrayTime[1] = m_iBGMFadeInTime;
		tmpArrayTime[2] = m_uiTotalTime - m_iBGMFadeOutTime;
		tmpArrayTime[3] = m_uiTotalTime;

		m_pBackGround->setAudioEnvelop(4, tmpArrayTime, tmpArrayVol);
	}

	m_pEffectItemVec->printEffectInfo();
	m_iDirectExportIndex = 0;
}

void CClipList::updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face){

	CAutoLock m(m_ClipListLock);
	
	for(CClipItemVectorIterator itor = m_ClipItemVec.begin(); itor != m_ClipItemVec.end(); ++itor){

		CClipItem* pitem = *itor;

		if(TRUE == pitem->updateRenderInfo(clipid, face_detected, start, end, face))
			return;
	}
}

void CClipList::resetFaceDetectInfo(unsigned int clipid){

	CAutoLock m(m_ClipListLock);
	
	for(CClipItemVectorIterator itor = m_ClipItemVec.begin(); itor != m_ClipItemVec.end(); ++itor){

		CClipItem* pitem = *itor;

		if(TRUE == pitem->resetFaceDetectInfo(clipid))
			return;
	}
}

bool CClipList::checkFaceDetectProcessedAtTime(unsigned int uiTime){

	CAutoLock m(m_ClipListLock);

	for(CClipItemVectorIterator itor = m_ClipItemVec.begin(); itor != m_ClipItemVec.end(); ++itor){

		CClipItem* pitem = *itor;

		if(false == pitem->checkFaceDetectProcessedAtTime(uiTime))
			return false;
	}

	return true;
}
void CClipList::reCalcTimeForLoadList(NXBOOL bOnlyTotalTime, std::map<int, int>* pmapper)
{
	CClipItem*		pItem				= NULL;
	unsigned int	uiClipIndex			= 0;

	m_uiTotalTime						= 0;

	m_isCheckDirectExport				= FALSE;

	CAutoLock m(m_ClipListLock);
	m_pEffectItemVec->lock();
	m_pEffectItemVec->clearEffectItem();
	m_pEffectItemVec->unlock();
	
	int iClipVecSize = m_ClipItemVec.size();
	if( iClipVecSize <= 0 ) return;

	if( bOnlyTotalTime )
	{
		CClipItemVectorIterator i;
		for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++, uiClipIndex++)
		{
			pItem = ((CClipItem*)(*i));
			if( pItem == NULL ) continue;

			m_uiTotalTime = m_uiTotalTime < pItem->getEndTime() ? pItem->getEndTime() : m_uiTotalTime;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] reCalcTimeForLoadList m_ClipItemVec size(%zu), (%zu)", __LINE__, m_ClipItemVec.size(), m_AudioClipItemVec.size());
		if( m_ClipItemVec.size() < 20 )
			printClipInfo();

		{
			CClipItemVectorIterator i;
			for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
			{
				pItem = ((CClipItem*)(*i));
				if( pItem == NULL ) continue;

				CClipVideoRenderInfo* pVideoRenderInfo = getVideoRenderInfoByItem(pItem);
				SAFE_ADDREF(pVideoRenderInfo);
				pItem->m_ClipVideoRenderInfoVec.insert(pItem->m_ClipVideoRenderInfoVec.end(), pVideoRenderInfo);
				SAFE_RELEASE(pVideoRenderInfo); // to set ref count to 1

				CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pItem);
				SAFE_ADDREF(pAudioRenderInfo);
				pItem->m_ClipAudioRenderInfoVec.insert(pItem->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
				SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1
			}

			for(i = m_AudioClipItemVec.begin(); i != m_AudioClipItemVec.end(); i++)
			{
				pItem = ((CClipItem*)(*i));
				if( pItem == NULL ) continue;

				CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pItem);
				SAFE_ADDREF(pAudioRenderInfo);
				pItem->m_ClipAudioRenderInfoVec.insert(pItem->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
				SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1
			}
		}
		return;
	}

	for(int i = 0; i < iClipVecSize; i++, uiClipIndex++)
	{
		pItem = (CClipItem*)m_ClipItemVec[i];
		if( pItem && (pItem->getClipType() == CLIPTYPE_IMAGE || pItem->getClipType() == CLIPTYPE_VIDEO) )
		{
			int iEffectDuration		= pItem->getClipEffectDuration();
			int iEffectOverlap		= pItem->getClipEffectOverlap();
			int iEffectOffset		= pItem->getClipEffectOffset();

			m_uiTotalTime			= pItem->getEndTime();
			
			if( m_bSeparateEffect )
			{
				if( pItem->getClipEffectID() != NULL && strcmp(pItem->getClipEffectID(), "none") != 0 ) {
					for(int j = i+1; j < iClipVecSize; j++ )
					{
						if( m_ClipItemVec[j]->getClipType() == CLIPTYPE_IMAGE || m_ClipItemVec[j]->getClipType() == CLIPTYPE_VIDEO ) {
							CVideoEffectItem* pEffectItem = new CVideoEffectItem();
							if( pEffectItem )
							{
								unsigned int uiEffectStartTime = m_uiTotalTime - (iEffectDuration * iEffectOffset / 100);
								pEffectItem->setEffectInfo(pItem->getStartTime(), pItem->getEndTime(), uiEffectStartTime, iEffectDuration, iEffectOffset, iEffectOverlap, pItem->getClipEffectID(), pItem->getTitle(), uiClipIndex, iClipVecSize);
								m_pEffectItemVec->lock();
								m_pEffectItemVec->addEffectItem(pEffectItem);
								m_pEffectItemVec->unlock();
								SAFE_RELEASE(pEffectItem);
							}
							break;
						}
					}
				}

				if( pItem->getTitleEffectID() != NULL && strcmp(pItem->getTitleEffectID(), "none") != 0 ) {

					CVideoEffectItem* pEffectItem2 = new CVideoEffectItem();
					if( pEffectItem2 )
					{
						pEffectItem2->setTitleEffectInfo(pItem->getStartTime(), pItem->getEndTime(), pItem->getTitleStartTime(), pItem->getTitleEndTime(), pItem->getTitleEffectID(), pItem->getTitle(), uiClipIndex, iClipVecSize);
						m_pEffectItemVec->lock();
						m_pEffectItemVec->addEffectItem(pEffectItem2);
						m_pEffectItemVec->unlock();
						SAFE_RELEASE(pEffectItem2);
					}
				}
			}

			pItem->setClipIndexInfo(uiClipIndex);
			pItem->setClipTotalCount(iClipVecSize);
		}
		else if( pItem && (pItem->getClipType() == CLIPTYPE_VIDEO_LAYER) )
		{
			pItem->setClipIndexInfo(uiClipIndex);
			pItem->setClipTotalCount(iClipVecSize);
		}
	}

	m_ClipItemVec[iClipVecSize -1]->setClipEffectEnable(FALSE);
	m_ClipItemVec[iClipVecSize -1]->setTitleEndTime(m_uiTotalTime);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] m_ClipItemVec size(%d) (%zu)", __LINE__, iClipVecSize, m_AudioClipItemVec.size());
	// printClipInfo(); // LG-H-779 issue

	m_pEffectItemVec->printEffectInfo();
	m_iDirectExportIndex = 0;
	CClipItemVectorIterator i;

	//if(iClipVecSize > 1)
	{
		CClipItem*	pFirst				= NULL;
		CClipItem*	pSecond				= NULL;

		IRectangle*	pRec						= NULL;
		float* pMatrix = NULL;
		int iMatrixSize = 0;
									

		int iFirst = 0, iSecond = 0;
		CClipItemVectorIterator j;			

		if(iClipVecSize == 1)
		{
			pFirst = m_ClipItemVec[0];
			if( pFirst )
			{
				CClipVideoRenderInfo* pVideoRenderInfo = getVideoRenderInfoByItem(pFirst);
				SAFE_ADDREF(pVideoRenderInfo);
				pFirst->m_ClipVideoRenderInfoVec.insert(pFirst->m_ClipVideoRenderInfoVec.end(), pVideoRenderInfo);
				SAFE_RELEASE(pVideoRenderInfo); // to set ref count to 1

				CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pFirst);
				SAFE_ADDREF(pAudioRenderInfo);
				pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
				SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1
			}
		}
		else
		{

            if(pmapper)
            {
                for(const auto &e : m_ClipItemVec) 
                {
                    (*pmapper)[e->getClipID()] = e->getClipID();
                }
            }

			for(iFirst = 0; iFirst < iClipVecSize; iFirst++)
			{
				pFirst = m_ClipItemVec[iFirst];
				if( pFirst )
				{
					CClipVideoRenderInfo* pVideoRenderInfo = getVideoRenderInfoByItem(pFirst);
					SAFE_ADDREF(pVideoRenderInfo);
					pFirst->m_ClipVideoRenderInfoVec.insert(pFirst->m_ClipVideoRenderInfoVec.end(), pVideoRenderInfo);
					SAFE_RELEASE(pVideoRenderInfo); // to set ref count to 1

					CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pFirst);
					SAFE_ADDREF(pAudioRenderInfo);
					pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
					SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1

					if(iFirst == iClipVecSize - 1)
						break;
					if(pFirst->getClipType() != CLIPTYPE_IMAGE)
					{
						for(iSecond = iFirst + 1; iSecond < iClipVecSize; iSecond++)
						{
							pSecond = m_ClipItemVec[iSecond];
							if( pSecond && pSecond->getClipType() == pFirst->getClipType())
							{
								if(checkVideoItems(pFirst, pSecond) == 0 && checkAudioItems(pFirst, pSecond) == 0)
								{
                                    if(pmapper)
                                    {
                                        (*pmapper)[pSecond->getClipID()] = pFirst->getClipID();
                                    }
                                    
									pFirst->setEndTime(pSecond->getEndTime());
									pFirst->setEndTrimTime(pSecond->getEndTrimTime());

									pVideoRenderInfo = getVideoRenderInfoByItem(pSecond);
									SAFE_ADDREF(pVideoRenderInfo);
									pFirst->m_ClipVideoRenderInfoVec.insert(pFirst->m_ClipVideoRenderInfoVec.end(), pVideoRenderInfo);
									SAFE_RELEASE(pVideoRenderInfo); // to set ref count to 1

									pAudioRenderInfo = getAudioRenderInfoByItem(pSecond);
									SAFE_ADDREF(pAudioRenderInfo);
									pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
									SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1

									for(i = m_ClipItemVec.begin(); i != m_ClipItemVec.end(); i++)
									{
										pItem = ((CClipItem*)(*i));
										if( pSecond->getClipID() == pItem->getClipID())
										{
											m_ClipItemVec.erase(i);
											SAFE_RELEASE(pItem);
											break;
										}
									}
									iSecond--;
									iClipVecSize = m_ClipItemVec.size();
								}
							}
						}
					}
				}
			}
		}
		//m_ClipItemVec[iClipVecSize -1]->setClipEffectEnable(FALSE);
		//m_ClipItemVec[iClipVecSize -1]->setTitleEndTime(m_uiTotalTime);

		std::sort(m_ClipItemVec.begin(), m_ClipItemVec.end(), compareClipStartTime);

		iClipVecSize = m_AudioClipItemVec.size();
		if(iClipVecSize == 1)
		{
			pFirst = m_AudioClipItemVec[0];
			if( pFirst )
			{
				CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pFirst);
				SAFE_ADDREF(pAudioRenderInfo);
				pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
				SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1
			}
		}
		else
		{
			for(iFirst = 0; iFirst < iClipVecSize; iFirst++)
			{
				pFirst = m_AudioClipItemVec[iFirst];
				if( pFirst && pFirst->getClipType() == CLIPTYPE_AUDIO)
				{
					CClipAudioRenderInfo* pAudioRenderInfo = getAudioRenderInfoByItem(pFirst);
					SAFE_ADDREF(pAudioRenderInfo);
					pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
					SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1

					if(iFirst == iClipVecSize - 1)
						break;
					for(iSecond = iFirst + 1; iSecond < iClipVecSize; iSecond++)
					{
						pSecond = m_AudioClipItemVec[iSecond];
						if( pSecond && pSecond->getClipType() == CLIPTYPE_AUDIO)
						{
							if(checkAudioItems(pFirst, pSecond) == 0)
							{
								pFirst->setEndTime(pSecond->getEndTime());
								pFirst->setEndTrimTime(pSecond->getEndTrimTime());

								pAudioRenderInfo = getAudioRenderInfoByItem(pSecond);
								SAFE_ADDREF(pAudioRenderInfo);
								pFirst->m_ClipAudioRenderInfoVec.insert(pFirst->m_ClipAudioRenderInfoVec.end(), pAudioRenderInfo);
								SAFE_RELEASE(pAudioRenderInfo); // to set ref count to 1

								for(i = m_AudioClipItemVec.begin(); i != m_AudioClipItemVec.end(); i++)
								{
									pItem = ((CClipItem*)(*i));
									if( pSecond->getClipID() == pItem->getClipID())
									{
										m_AudioClipItemVec.erase(i);
										SAFE_RELEASE(pItem);
										break;
									}
								}
								iSecond--;
								iClipVecSize = m_AudioClipItemVec.size();
							}
						}
					}
				}		
			}
		}
	}

	iClipVecSize = m_ClipItemVec.size();
	for(i = m_ClipItemVec.begin(), uiClipIndex = 0; i != m_ClipItemVec.end(); i++, uiClipIndex++)
	{
		pItem = ((CClipItem*)(*i));
		pItem->setClipIndexInfo(uiClipIndex);
		pItem->setClipTotalCount(iClipVecSize);
	}

	iClipVecSize = m_AudioClipItemVec.size();
	for(i = m_AudioClipItemVec.begin(), uiClipIndex = 0; i != m_AudioClipItemVec.end(); i++, uiClipIndex++)
	{
		pItem = ((CClipItem*)(*i));
		pItem->setClipIndexInfo(uiClipIndex);
		pItem->setClipTotalCount(iClipVecSize);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] reCalcTimeForLoadList m_ClipItemVec size(%zu), (%zu)", __LINE__, m_ClipItemVec.size(), m_AudioClipItemVec.size());
	if( m_ClipItemVec.size() < 20 )
		printClipInfo();
}

void CClipList::printClipInfo()
{
	for(int i = 0; i < m_ClipItemVec.size(); i++)
		m_ClipItemVec[i]->printClipInfo();
	
	for(int i = 0; i < m_AudioClipItemVec.size(); i++)
		m_AudioClipItemVec[i]->printClipInfo();
}

int CClipList::getFirstClipCodecType(NXFF_MEDIA_TYPE eType)
{
	if( m_ClipItemVec.size() <=0 ) 
		return (int)eNEX_CODEC_UNKNOWN;

	return m_ClipItemVec[0]->getCodecType(eType);
}

int CClipList::getFirstClipDSIofClip(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize )
{
	if( m_ClipItemVec.size() <=0 ) 
		return (int)NEXVIDEOEDITOR_ERROR_UNKNOWN;

	return m_ClipItemVec[0]->getDSIofClip(eType, ppDSI, pDSISize);
}

int CClipList::getFirstClipRotateInfo(int* pRotate)
{
	if( m_ClipItemVec.size() <=0 || pRotate == NULL ) 
		return (int)NEXVIDEOEDITOR_ERROR_UNKNOWN;

	*pRotate = m_ClipItemVec[0]->getContentOrientation();
	return (int)NEXVIDEOEDITOR_ERROR_NONE;
}

CClipItem* CClipList::getVideoClipItem(unsigned int uiStartTime, unsigned int uiEndTime)
{
	CClipItem* pItem = NULL;
	if( m_ClipItemVec.size() <= 0 )
		return pItem;

	for( int i = 0; i < m_ClipItemVec.size(); i++ )
	{
		if( m_ClipItemVec[i]->getStartTime() <= uiStartTime && m_ClipItemVec[i]->getEndTime() >= uiEndTime )
		{
			pItem = m_ClipItemVec[i];
			break;
		}
	}

	SAFE_ADDREF(pItem);
	return pItem;
}

unsigned int CClipList::getMaxHWUseSize(unsigned int uiTime)
{
	unsigned int uiMaxSize = 0;
	for(unsigned int uiCur = uiTime; uiCur < m_uiTotalTime; uiCur += 1000 )
	{
		unsigned int uTotalSize = 0;
		for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
		{
			int size = 0;
			if( m_ClipItemVec[i]->isPlayTime(uiCur) )
			{
				if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER )
				{
					size = m_ClipItemVec[i]->getWidth()*m_ClipItemVec[i]->getHeight();
					uTotalSize += size;
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getMaxHWUseSize Item(%d, %d, %d)", __LINE__, i, size, uTotalSize);

				}
			}
		}
#ifdef FOR_LAYER_FEATURE
		for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
		{
			int size = 0;
			if( m_LayerItemVec[i]->isPlayTime(uiCur) )
			{
				size = m_ClipItemVec[i]->getWidth()*m_ClipItemVec[i]->getHeight();
				uTotalSize += size;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getMaxHWUseSize Layer(%d, %d, %d)", __LINE__, i, size, uTotalSize);			
			}
		}
#endif	
		uiMaxSize = uiMaxSize > uTotalSize ? uiMaxSize : uTotalSize;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getMaxHWUseSize(%d, %d)", __LINE__, uiMaxSize, uTotalSize);	
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getMaxHWUseSize(%d)", __LINE__, uiMaxSize);	
	return uiMaxSize;
}

unsigned int CClipList::getPreviewHWSize(unsigned int uiTime)
{
	unsigned int uTotalSize = 0;
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		uTotalSize += m_ClipItemVec[i]->checkResourceSize(uiTime);
	}
	
#ifdef FOR_LAYER_FEATURE
	for(int i = 0; i < (int)m_LayerItemVec.size(); i++)
	{
		int size = 0;
		if( m_LayerItemVec[i]->isPlayTime(uiTime) )
		{
			size = m_ClipItemVec[i]->getWidth()*m_ClipItemVec[i]->getHeight();
			uTotalSize += size;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getPreviewHWSize Layer(%d, %d, %d)", __LINE__, i, size, uTotalSize);			
		}
	}
#endif	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getPreviewHWSize(%d)", __LINE__, uTotalSize);		
	return uTotalSize;
}

int CClipList::getVisualClipCountAtTime(unsigned int uiTime)
{
    int iCount = 0;
    for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
    {
    	// uiTime + 5 for KM-4067 case1 and case3
        if( m_ClipItemVec[i]->isPlayTime(uiTime+5, FALSE) && m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO && m_ClipItemVec[i]->getVideoDecodingEnd() == FALSE)
            iCount++;
    }
    return iCount;
}

int CClipList::getAllVisualClipCountAtTime(unsigned int uiTime)
{
    int iCount = 0;
    for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
    {
    	// uiTime + 5 for KM-4067 case1 and case3
        if( m_ClipItemVec[i]->isPlayTime(uiTime+5, FALSE) && (m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_IMAGE) )
            iCount++;
    }
    return iCount;
}

int CClipList::getVisualLayerCountAtTime(unsigned int uiTime)
{
    int iCount = 0;
    for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
    {
    	// uiTime + 5 for KM-4067 case1 and case3
        if( m_ClipItemVec[i]->isPlayTime(uiTime+5, FALSE) && m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER && m_ClipItemVec[i]->getVideoDecodingEnd() == FALSE)
            iCount++;
    }
    return iCount;
}

unsigned int CClipList::getVisualIndex()
{
    return m_uVisualIndex;
}

void CClipList::setVisualIndex(unsigned int uiIndex)
{
    m_uVisualIndex = uiIndex;
}

unsigned int* CClipList::getVisualClipIDsAtTime(unsigned int uiTime, unsigned int* pCount)
{
    int iCount = 0;
    for(int i = m_uVisualIndex; i < (int)m_ClipItemVec.size(); i++)
    {
    	// uiTime + 5 for KM-4067 case1 and case3
        if(m_ClipItemVec[i]->getStartTime() > uiTime + 5)
            break;
        if( m_ClipItemVec[i]->isPlayTime(uiTime+5, FALSE) && m_ClipItemVec[i]->getClipType() == CLIPTYPE_IMAGE )
        {
            m_pVisualIDs[iCount] = m_ClipItemVec[i]->getClipID();
            iCount++;
            m_uVisualIndex = i;
        }
    }

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ClipList.cpp %d] getVisualClipIDsAtTime(%d, %d, %d, %d)", __LINE__, m_uVisualIndex, uiTime, iCount, m_ClipItemVec[m_uVisualIndex]->getStartTime());		
    
    *pCount = iCount;
    return m_pVisualIDs;
}

int CClipList::getVideoClipCount()
{
    int iCount = 0;
    for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
    {
        if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER)
            iCount++;
    }

    return iCount;
}

int CClipList::setVideoDecodingEnd(int iEnd)
{
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		if( m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO || m_ClipItemVec[i]->getClipType() == CLIPTYPE_VIDEO_LAYER )
			m_ClipItemVec[i]->setVideoDecodingEnd(iEnd);
	}
	
	return true;
}

int CClipList::checkVideoItems(CClipItem* pFirst, CClipItem* pSecond)
{
	if(pFirst == NULL || pSecond == NULL)
		return 1;
	
	if( strcmp(pFirst->getClipPath(), pSecond->getClipPath()) != 0 )
		return 1;

	
	if(pFirst->getEndTime() != pSecond->getStartTime())
		return 1;

	int iVecSize = pFirst->m_ClipVideoRenderInfoVec.size();

	if((pFirst->getTotalTime() - pFirst->m_ClipVideoRenderInfoVec[iVecSize-1]->mEndTrimTime) != pSecond->getStartTrimTime())
		return 1;
	
//	if(pFirst->getSpeedCtlFactor() != pSecond->getSpeedCtlFactor())
//		return 1;
	
	return 0;
}

int CClipList::checkAudioItems(CClipItem* pFirst, CClipItem* pSecond)
{
	if(pFirst == NULL || pSecond == NULL)
		return 1;

	if( strcmp(pFirst->getClipPath(), pSecond->getClipPath()) != 0 )
		return 1;

	if( pFirst->getAudioOnOff() == 0 && pSecond->getAudioOnOff() == 0 )
		return 0;

	if(pFirst->getEndTime() != pSecond->getStartTime())
		return 1;

	int iVecSize = pFirst->m_ClipAudioRenderInfoVec.size();
	if((pFirst->getTotalTime() - pFirst->m_ClipAudioRenderInfoVec[iVecSize-1]->mEndTrimTime) != pSecond->getStartTrimTime())
		return 1;

	unsigned int uiSecondDur = 0;	
	if(pSecond->getSpeedCtlFactor() == 13)
	{
		uiSecondDur = (pSecond->getEndTime() - pSecond->getStartTime())/100*25/2;
	}
	else if( pSecond->getSpeedCtlFactor() == 6 )
	{
		uiSecondDur = (pSecond->getEndTime() - pSecond->getStartTime())/100*25/4;
	}	
	else if( pSecond->getSpeedCtlFactor() == 3 )
	{
		uiSecondDur = (pSecond->getEndTime() - pSecond->getStartTime())/100*25/8;
	}	
	else
	{
		uiSecondDur = (pSecond->getEndTime() - pSecond->getStartTime())/100*pSecond->getSpeedCtlFactor();
	}


	if(uiSecondDur > (pSecond->getTotalTime() - pSecond->getEndTrimTime() - pSecond->getStartTrimTime()))
	{
		if(uiSecondDur - (pSecond->getTotalTime() - pSecond->getEndTrimTime() - pSecond->getStartTrimTime()) > 10)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] checkAudioItems %d, %d, %d, %d", __LINE__, uiSecondDur, pSecond->getTotalTime(), pSecond->getEndTrimTime(), pSecond->getStartTrimTime());
			return 1;
		}
	}
//	if(pFirst->getSpeedCtlFactor() != pSecond->getSpeedCtlFactor())
//		return 1;

	if(pFirst->getAudioOnOff() != pSecond->getAudioOnOff())
		return 1;
 
	if(pFirst->getAutoEnvelop() != pSecond->getAutoEnvelop())
		return 1;

	return 0;
}

CClipVideoRenderInfo* CClipList::getVideoRenderInfoByItem(CClipItem* pItem)
{
	CClipVideoRenderInfo* pRenderInfo = new CClipVideoRenderInfo;
	IRectangle*	pRec						= NULL;
	float* pMatrix = NULL;
	int iMatrixSize = 0;

	if(pRenderInfo == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getVideoRenderInfoByItem mem alloc failed", __LINE__);
		return NULL;
	}
	//memset(pRenderInfo, 0x00, sizeof(CClipVideoRenderInfo));

	pRenderInfo->mFaceDetectProcessed = 1;

	pRenderInfo->muiTrackID = pItem->getClipID();
	pRenderInfo->mClipType = pItem->getClipType();

	pRenderInfo->mStartTime = pItem->getStartTime();
	pRenderInfo->mEndTime = pItem->getEndTime();

	pRenderInfo->mStartTrimTime = pItem->getStartTrimTime();
	pRenderInfo->mEndTrimTime = pItem->getEndTrimTime();

	pRenderInfo->mRotateState = pItem->getRotateState();
	pRenderInfo->mBrightness = pItem->getBrightness();
	pRenderInfo->mContrast = pItem->getContrast();
	pRenderInfo->mSaturation = pItem->getSaturation();
    pRenderInfo->mHue = pItem->getHue();
	pRenderInfo->mTintcolor = pItem->getTintcolor();
	pRenderInfo->mLUT = pItem->getLUT();
	pRenderInfo->mCustomLUT_A = pItem->getCustomLUTA();
	pRenderInfo->mCustomLUT_B = pItem->getCustomLUTB();
	pRenderInfo->mCustomLUT_Power = pItem->getCustomLUTPower();
    pRenderInfo->mVignette = pItem->getVignette();
	//pRenderInfo->mVideoRenderMode =  pFirst->getVideoRenderMode();
	pRec =  pItem->getStartPosition();
	if( pRec != NULL )
	{
		pRenderInfo->mStart.left		= pRec->getLeft();
		pRenderInfo->mStart.top		= pRec->getTop();
		pRenderInfo->mStart.right		= pRec->getRight();
		pRenderInfo->mStart.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pRec =  pItem->getEndPosition();
	if( pRec != NULL )
	{
		pRenderInfo->mEnd.left			= pRec->getLeft();
		pRenderInfo->mEnd.top			= pRec->getTop();
		pRenderInfo->mEnd.right			= pRec->getRight();
		pRenderInfo->mEnd.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pMatrix = pItem->getStartMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(pRenderInfo->mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pMatrix = pItem->getEndMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(pRenderInfo->mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pRenderInfo->m_iSpeedCtlFactor = pItem->getSpeedCtlFactor();

	pRenderInfo->mFreezeDuration = pItem->getFreezeDuration();
	pRenderInfo->mIframePlay = pItem->getIframePlay();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getVideoRenderInfoByItem ID(%d) FreezeDuration(%d)", __LINE__, pRenderInfo->muiTrackID, pRenderInfo->mFreezeDuration);
	return pRenderInfo;
}

CClipAudioRenderInfo* CClipList::getAudioRenderInfoByItem(CClipItem* pItem)
{
	CClipAudioRenderInfo* pRenderInfo = new CClipAudioRenderInfo;

	if(pRenderInfo == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getAudioRenderInfoByItem mem alloc failed", __LINE__);
		return NULL;
	}
	//memset(pRenderInfo, 0x00, sizeof(CLIP_AUDIO_RENDER_INFO));

	pRenderInfo->muiTrackID = pItem->getClipID();
	pRenderInfo->mClipType = pItem->getClipType();

	pRenderInfo->mStartTime = pItem->getStartTime();
	pRenderInfo->mEndTime = pItem->getEndTime();

	pRenderInfo->mStartTrimTime = pItem->getStartTrimTime();
	pRenderInfo->mEndTrimTime = pItem->getEndTrimTime();

	pRenderInfo->m_iAudioOnOff = pItem->getAudioOnOff();
	pRenderInfo->m_iClipVolume = pItem->getAudioVolume();

	pRenderInfo->m_iVoiceChangerFactor = pItem->getVoiceChangerFactor();	
	pRenderInfo->m_iCompressorFactor = pItem->getCompressorFactor();	
	pRenderInfo->m_iPitchIndex = pItem->getPitchFactor();
	pRenderInfo->m_iMusicEffector = pItem->getMusicEffector();	
	pRenderInfo->m_iProcessorStrength = pItem->getProcessorStrength();	
	pRenderInfo->m_iBassStrength = pItem->getBassStrength();	
	pRenderInfo->m_iPanFactor[0] = pItem->getPanLeftFactor();	
	pRenderInfo->m_iPanFactor[1] = pItem->getPanRightFactor();	
	pRenderInfo->m_uiBGMEndTIme = pItem->getEndTime() - (pItem->getClipEffectDuration() * pItem->getClipEffectOverlap() / 100);    
	pRenderInfo->m_iBackGroundVolume = pItem->getBGMVolume();	
	pRenderInfo->m_bEnableEnvelopAudio = TRUE;		
	pRenderInfo->m_iEnvelopAudioSize = pItem->getAudioEnvelopSize();		

	if(pItem->getEnhancedAudioFilter())
	{
		pRenderInfo->m_pEnhancedAudioFilter = (char*)nexSAL_MemAlloc(strlen(pItem->getEnhancedAudioFilter())+1);
		strcpy(pRenderInfo->m_pEnhancedAudioFilter, pItem->getEnhancedAudioFilter());
	}
	if(pItem->getEqualizer())
	{
		pRenderInfo->m_pEqualizer = (char*)nexSAL_MemAlloc(strlen(pItem->getEqualizer())+1);
		strcpy(pRenderInfo->m_pEqualizer, pItem->getEqualizer());
	}
	if(pRenderInfo->m_iEnvelopAudioSize)
	{
		pRenderInfo->m_puiEnvelopAudioVolume = (unsigned int*)nexSAL_MemAlloc(sizeof(int)*pRenderInfo->m_iEnvelopAudioSize);
		pRenderInfo->m_puiEnvelopAudioTime = (unsigned int*)nexSAL_MemAlloc(sizeof(int)*pRenderInfo->m_iEnvelopAudioSize);

		for( int i = 0 ; i < pRenderInfo->m_iEnvelopAudioSize; i++)
		{
			unsigned int uiTime = 0, uiVolume = 0;
			pItem->getAudioEnvelop(i, &uiTime, &uiVolume);
			pRenderInfo->m_puiEnvelopAudioVolume[i] = uiVolume;
			pRenderInfo->m_puiEnvelopAudioTime[i] = uiTime;
		}
	}
	pRenderInfo->m_iSpeedCtlFactor = pItem->getSpeedCtlFactor();
	pRenderInfo->m_bKeepPitch = pItem->getKeepPitch();

	return pRenderInfo;
}

void CClipList::setDrawInfos(IDrawInfoList* pDrawInfos)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setDrawInfos(%p)", __LINE__, pDrawInfos);

	CNexDrawInfoVec* pDrawVec = (CNexDrawInfoVec*)pDrawInfos;
	if( pDrawVec == NULL ) return;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		CNexDrawInfoVec* newInfos = pDrawVec->getDrawInfoList(m_ClipItemVec[i]->getClipID());
		m_ClipItemVec[i]->setDrawInfos(newInfos);
		SAFE_RELEASE(newInfos);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setDrawInfos End", __LINE__);
}

void CClipList::updateDrawInfo(IDrawInfo* pDrawInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setDrawInfo(%p)", __LINE__, pDrawInfo);

	CNexDrawInfo* pDraw = (CNexDrawInfo*)pDrawInfo;
	if( pDraw == NULL ) return;

	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		m_ClipItemVec[i]->updateDrawInfo(pDraw);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] setDrawInfo End", __LINE__);
}

CNexDrawInfo* CClipList::getDrawInfo(int id)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getDrawInfo(%d)", __LINE__, id);

	CNexDrawInfo* pInfo = NULL;
	for(int i = 0; i < (int)m_ClipItemVec.size(); i++)
	{
		pInfo = m_ClipItemVec[i]->getDrawInfo(id);
		if( pInfo != NULL )
			break;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ClipList.cpp %d] getDrawInfo End(0x%p)", __LINE__, pInfo);
	return pInfo;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
