// From http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/ (this is PUBLIC DOMAIN, so it's okay to use it) --Matthew 2012-07-23

// SimplexNoise1234
// Copyright 2003-2011, Stefan Gustavson
//
// Contact: stegu@itn.liu.se
//
// This library is public domain software, released by the author
// into the public domain in February 2011. You may do anything
// you like with it. You may even remove all attributions,
// but of course I'd appreciate it if you kept my name somewhere.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

/** \file
		\brief Declares the SimplexNoise1234 class for producing Perlin simplex noise.
		\author Stefan Gustavson (stegu@itn.liu.se)
*/

/*
 * This is a clean, fast, modern and free Perlin Simplex noise class in C++.
 * Being a stand-alone class with no external dependencies, it is
 * highly reusable without source code modifications.
 *
 *
 * Note:
 * Replacing the "float" type with "double" can actually make this run faster
 * on some platforms. A templatized version of SimplexNoise1234 could be useful.
 */

// Wrappers for C added July 23, 2012 by Matthew Feinberg
#ifdef __cplusplus
extern "C" {
#endif
    float RITM_simplex_noise1( float x );
    float RITM_simplex_noise2( float x, float y );
    float RITM_simplex_noise3( float x, float y, float z );
    float RITM_simplex_noise4( float x, float y, float z, float w );
    
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class RITM_SimplexNoise1234 {

  public:
    RITM_SimplexNoise1234() {}
    ~RITM_SimplexNoise1234() {}

    static float noise( float x );
    static float noise( float x, float y );
    static float noise( float x, float y, float z );
    static float noise( float x, float y, float z, float w );

  private:
    static unsigned char perm[];
    static float  grad( int hash, float x );
    static float  grad( int hash, float x, float y );
    static float  grad( int hash, float x, float y , float z );
    static float  grad( int hash, float x, float y, float z, float t );

};

#endif