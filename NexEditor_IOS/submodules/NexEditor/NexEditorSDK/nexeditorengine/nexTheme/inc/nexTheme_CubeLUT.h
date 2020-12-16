/******************************************************************************
* File Name   :	nexTheme_CubeLUT.h
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

#ifndef CubeLUT_H
#define CubeLUT_H

#include <string>
#include <vector>
#include <sstream>
using namespace std;

class CubeLUT {
    public:
        typedef vector <float> tableRow;
        typedef vector <tableRow> table1D;
        typedef vector <table1D> table2D;
        typedef vector <table2D> table3D;
        enum LUTState { OK = 0, NotInitialized = 1,
        ReadError = 10, WriteError, PrematureEndOfFile, LineError,
        UnknownOrRepeatedKeyword = 20, TitleMissingQuote, DomainBoundsReversed,
        LUTSizeOutOfRange, CouldNotParseTableData };
        LUTState status;
        string title;
        tableRow domainMin;
        tableRow domainMax;
        table1D LUT1D;
        table3D LUT3D;
        CubeLUT ( void ) { status = NotInitialized; };
        LUTState LoadCubeFile ( istringstream & infile );
        int* createLUT();
    private:
        string ReadLine ( istringstream & infile, char lineSeparator);
        tableRow ParseTableRow ( const string & lineOfText );
};

#endif