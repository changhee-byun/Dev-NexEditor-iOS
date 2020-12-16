/******************************************************************************
* File Name   :	NexTheme_CubeLUT.cpp
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

#include "nexTheme_CubeLUT.h"
#include <sstream>
#include <cmath>

string CubeLUT:: ReadLine ( istringstream & infile, char lineSeparator )
{
    // Skip empty lines and comments
    const char CommentMarker = '#';
    string textLine("");
    while ( textLine.size() == 0 || textLine[0] == CommentMarker ) {
        if ( infile.eof() ) { status = PrematureEndOfFile; break; }
        getline ( infile, textLine, lineSeparator );
        if ( infile.fail() ) { status = ReadError; break; }
    }
    return textLine;
} // ReadLine

vector <float> CubeLUT:: ParseTableRow ( const string & lineOfText )
{
    int N = 3;
    tableRow f ( N );
    istringstream line ( lineOfText );
    for (int i = 0; i < N; i++) {
        line >> f[i];
        if ( line.fail() ) { status = CouldNotParseTableData; break; };
    }
    return f;
} // ParseTableRow
CubeLUT:: LUTState CubeLUT:: LoadCubeFile ( istringstream & infile )
{
    // Set defaults
    status = OK;
    title.clear();
    domainMin = tableRow ( 3, 0.0 );
    domainMax = tableRow ( 3, 1.0 );
    LUT1D.clear();
    LUT3D.clear();
    // Read file data line by line
    const char NewlineCharacter = '\n';
    char lineSeparator = NewlineCharacter;
    // sniff use of legacy lineSeparator
    const char CarriageReturnCharacter = '\r';
    for (int i = 0; i < 255; i++) {
        char inc = infile.get();
        if ( inc == NewlineCharacter ) break;
        if ( inc == CarriageReturnCharacter ) {
            if ( infile.get() == NewlineCharacter ) break;
            lineSeparator = CarriageReturnCharacter;
            break;
        }
        if ( i > 250 ) { status = LineError; break; }
    }
    infile.seekg ( 0 );
    infile.clear();
    // read keywords
    int N, CntTitle, CntSize, CntMin, CntMax;
    // each keyword to occur zero or one time
    N = CntTitle = CntSize = CntMin = CntMax = 0;
    while ( status == OK ) {
        long linePos = infile.tellg();
        string lineOfText = ReadLine ( infile, lineSeparator );

        if(lineOfText.size() > 0){

            if(lineOfText.at(lineOfText.size() - 1) == CarriageReturnCharacter)
                lineOfText.pop_back();

            if(lineOfText.size() <= 0)
                continue;            
        }
        
        if ( status != OK ) break;
        // Parse keywords and parameters
        istringstream line ( lineOfText );
        string keyword;
        line >> keyword;
        if ( "+" < keyword && keyword < ":" ) {
            // lines of table data come after keywords
            // restore stream pos to re-read line of data
            infile.seekg ( linePos );
            break;
        } else if ( keyword == "TITLE" && CntTitle++ == 0 ) {
            const char QUOTE = '"';
            char startOfTitle;
            line >> startOfTitle;
            if ( startOfTitle != QUOTE ) { status = TitleMissingQuote; break; }
            getline ( line, title, QUOTE ); // read to "
        } else if ( keyword == "DOMAIN_MIN" && CntMin++ == 0 ) {
            line >> domainMin[0] >> domainMin[1] >> domainMin[2];
        } else if ( keyword == "DOMAIN_MAX" && CntMax++ == 0 ) {
            line >> domainMax[0] >> domainMax[1] >> domainMax[2];
        } else if ( keyword == "LUT_1D_SIZE" && CntSize++ == 0 ) {
            line >> N;
            if ( N < 2 || N > 65536 ) { status = LUTSizeOutOfRange; break; }
            LUT1D = table1D ( N, tableRow ( 3 ) );
        } else if ( keyword == "LUT_3D_SIZE" && CntSize++ == 0 ) {
            line >> N;
            if ( N < 2 || N > 256 ) { status = LUTSizeOutOfRange; break; }
            LUT3D = table3D ( N, table2D ( N, table1D ( N, tableRow ( 3 ) ) ) );
        } else { status = UnknownOrRepeatedKeyword; break; }
        if ( line.fail() ) { status = ReadError; break; }
    } // read keywords
    if ( status == OK && CntSize == 0 ) status = LUTSizeOutOfRange;
    if ( status == OK && ( domainMin[0] >= domainMax[0] || domainMin[1] >= domainMax[1]
        || domainMin[2] >= domainMax[2] ) )
        status = DomainBoundsReversed;
// read lines of table data
    if ( LUT1D.size() > 0 ) {
    N = LUT1D.size();
    for ( int i = 0; i < N && status == OK; i++ ) {
    LUT1D [i] = ParseTableRow ( ReadLine ( infile, lineSeparator ) );
    }
    } else {
        N = LUT3D.size();
        // NOTE that r loops fastest
        for ( int b = 0; b < N && status == OK; b++ ) {

            for ( int g = 0; g < N && status == OK; g++ ) {

                for ( int r = 0; r < N && status == OK; r++ ) {
                    LUT3D[r][g][b] = ParseTableRow( ReadLine ( infile, lineSeparator ) );
                }
            }
        }
    } // read 3D LUT
    return status;
} // LoadCubeFile

int* CubeLUT:: createLUT(){

    int N = LUT3D.size();
    if(N <= 0)
        return NULL;

    int* lut = new int[64 *4096];

    for(int b = 0; b < 64; ++b){

        int* pblock = lut + (64*64*63) - (64*64) * b;

        for(int r = 0; r < 64; ++r){

            for(int g = 0; g < 64; ++g){

                float value_x = (float)N * (float)r / 64.0f;
                float value_y = (float)N * (float)g / 64.0f;
                float value_z = (float)N * (float)b / 64.0f;

                int min_x = max(int(floor(value_x)), 0);
                int max_x = min(int(ceil(value_x)), N - 1);
                int min_y = max(int(floor(value_y)), 0);
                int max_y = min(int(ceil(value_y)), N - 1);
                int min_z = max(int(floor(value_z)), 0);
                int max_z = min(int(ceil(value_z)), N - 1);

                float c000 = LUT3D[min_x][min_y][min_z][0];
                float c010 = LUT3D[max_x][min_y][min_z][0];
                float c020 = LUT3D[min_x][max_y][min_z][0];
                float c030 = LUT3D[max_x][max_y][min_z][0];

                float c001 = LUT3D[min_x][min_y][min_z][1];
                float c011 = LUT3D[max_x][min_y][min_z][1];
                float c021 = LUT3D[min_x][max_y][min_z][1];
                float c031 = LUT3D[max_x][max_y][min_z][1];

                float c002 = LUT3D[min_x][min_y][min_z][2];
                float c012 = LUT3D[max_x][min_y][min_z][2];
                float c022 = LUT3D[min_x][max_y][min_z][2];
                float c032 = LUT3D[max_x][max_y][min_z][2];

                float c100 = LUT3D[min_x][min_y][max_z][0];
                float c110 = LUT3D[max_x][min_y][max_z][0];
                float c120 = LUT3D[min_x][max_y][max_z][0];
                float c130 = LUT3D[max_x][max_y][max_z][0];

                float c101 = LUT3D[min_x][min_y][max_z][1];
                float c111 = LUT3D[max_x][min_y][max_z][1];
                float c121 = LUT3D[min_x][max_y][max_z][1];
                float c131 = LUT3D[max_x][max_y][max_z][1];

                float c102 = LUT3D[min_x][min_y][max_z][2];
                float c112 = LUT3D[max_x][min_y][max_z][2];
                float c122 = LUT3D[min_x][max_y][max_z][2];
                float c132 = LUT3D[max_x][max_y][max_z][2];

                float evalx = floor(value_x) + 1.0f - value_x;
                float evaly = floor(value_y) + 1.0f - value_y;
                float evalz = floor(value_z) + 1.0f - value_z;

                float lower_final_color_0 = c000 * evalx * evaly + c010 * (1.0f - evalx) * evaly + c020 * evalx * (1.0f - evaly) + c030 * (1.0f - evalx) * (1.0f - evaly);
                float lower_final_color_1 = c001 * evalx * evaly + c011 * (1.0f - evalx) * evaly + c021 * evalx * (1.0f - evaly) + c031 * (1.0f - evalx) * (1.0f - evaly);
                float lower_final_color_2 = c002 * evalx * evaly + c012 * (1.0f - evalx) * evaly + c022 * evalx * (1.0f - evaly) + c032 * (1.0f - evalx) * (1.0f - evaly);
                float upper_final_color_0 = c100 * evalx * evaly + c110 * (1.0f - evalx) * evaly + c120 * evalx * (1.0f - evaly) + c130 * (1.0f - evalx) * (1.0f - evaly);
                float upper_final_color_1 = c101 * evalx * evaly + c111 * (1.0f - evalx) * evaly + c121 * evalx * (1.0f - evaly) + c131 * (1.0f - evalx) * (1.0f - evaly);
                float upper_final_color_2 = c102 * evalx * evaly + c112 * (1.0f - evalx) * evaly + c122 * evalx * (1.0f - evaly) + c132 * (1.0f - evalx) * (1.0f - evaly);

                float final_color_0 = lower_final_color_0 * evalz + upper_final_color_0 * (1.0f - evalz);
                float final_color_1 = lower_final_color_1 * evalz + upper_final_color_1 * (1.0f - evalz);
                float final_color_2 = lower_final_color_2 * evalz + upper_final_color_2 * (1.0f - evalz);

                int red = (int)(final_color_0 * 255.0f) << 16;
                int green = (int)(final_color_1 * 255.0f) << 8;
                int blue = (int)(final_color_2 * 255.0f);
                int alpha = 255 << 24;

                pblock[64 *63 - (r * 64) + g] = red|green|blue|alpha;
            }
        }
    }

    return lut;    
}