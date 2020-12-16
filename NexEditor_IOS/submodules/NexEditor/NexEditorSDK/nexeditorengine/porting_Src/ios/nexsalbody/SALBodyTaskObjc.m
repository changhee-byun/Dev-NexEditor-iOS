//
//  SALBodyTaskObjc.m
//  nexSalBody
//
//  Created by Simon Kim on 3/3/15.
//
//
#import "SALBodyTaskObjc.h"

NXINT32 SALBodyTaskMasterAutoreleasePool(NEXSALTaskFunc func, void *p)
{
	
	NXINT32 result;
	@autoreleasepool {
		result = func(p);
	}
	return result;
}