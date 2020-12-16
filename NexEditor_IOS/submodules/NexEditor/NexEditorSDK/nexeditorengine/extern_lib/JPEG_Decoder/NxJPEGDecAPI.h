#ifndef NX_IMG_IO_STREAM_TYPE
#define NX_IMG_IO_STREAM_TYPE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int (* file_read_t) (unsigned char *, int, int, void *);
typedef int (* file_write_t)(unsigned char *, int, int, void *);
typedef int (* file_seek_t) ( void *, long, int);

typedef struct {
	file_read_t   file_read;
	file_seek_t   file_seek;
	void		 *file_ptr;
	unsigned int  file_size;
} NxImgInputStream;

typedef struct {
	file_write_t  file_write;
	file_seek_t   file_seek;
	void	     *file_ptr;
	int           file_size_limit;

	int           output_file_size;
} NxImgOutputStream;

#endif

#ifndef NX_JPEG_DEC_API_H
#define NX_JPEG_DEC_API_H

#define SECTION_DECODING_NONE 10000

/* image color format */
#define FORMAT_YUV_400						0
#define FORMAT_YUV_420						1
#define FORMAT_YUV_422						2
#define FORMAT_YUV_444						3
#define FORMAT_YUV_411						4
#define FORMAT_YUV_422V						5
#define FORMAT_RGB_888						7
#define FORMAT_RGB_565						8
#define FORMAT_RGB_8888						9

/* same width and height */
#define	DIRECTION_ORIGINAL					0
#define	DIRECTION_FLIP_HORIZONTAL			1
#define	DIRECTION_FLIP_VERTICAL				2
#define	DIRECTION_ROTATE_180				3
/* swapped width and height */
#define	DIRECTION_FLIP_DIAGONAL1			4
#define	DIRECTION_ROTATE_90_CW				5
#define	DIRECTION_ROTATE_90_CCW				6
#define	DIRECTION_FLIP_DIAGONAL2			7

//unsigned char g_uDirection[8] = {0, 1, 3, 2, 4, 5, 7, 6};
/*
 1) transform="";;
 2) transform="-flip horizontal";;
 3) transform="-rotate 180";;
 4) transform="-flip vertical";;
 5) transform="-transpose";;
 6) transform="-rotate 90";;
 7) transform="-transverse";;
 8) transform="-rotate 270";;
*/

typedef struct {
	/*************************************************************************
	      input parameters about image
	 *************************************************************************/

	/** image decoding parameters **/

	//unsigned char	decoded_image_format;			// image format of output
	unsigned char	output_image_format;
	unsigned char	input_image_format;
	unsigned char	downsize_coeff;					// image downsize coefficient: 1, 2, 4, 8
	unsigned char	direction;						// eight types
	// to disable section decoding : set section_x1 to SECTION_DECODING_NONE
	unsigned short	section_x1;						// horizontal start point(unit of pixel)
	unsigned short	section_y1;						// vertical start point(unit of pixel)
	unsigned short	section_x2;						// horizontal end point(unit of pixel)
	unsigned short	section_y2;						// vertical end point(unit of pixel)
	unsigned char	*alpha_map;						// alpha map of image : set 0 to disable

	/*************************************************************************
	      buffer
	 *************************************************************************/

	unsigned char	*buffer_read;					// buffer for the file read chunk
	unsigned int	length_buffer_read;				// length of buffer_read

	/** output buffer : You have to prepare these buffers before decoding. **/
	unsigned char	*buffer_y;						// y value pointer of output image
	unsigned int	length_buffer_y;				// length of buffer_y
	unsigned char	*buffer_u;						// u value pointer of output image
	unsigned char	*buffer_v;						// v value pointer of output image
	unsigned int	length_buffer_uv;				// length of buffer_uv
	unsigned short	*buffer_temp;					// temp buffer for progressive jpeg
	unsigned int	length_buffer_temp;				// length of buffer_temp

	
	/*************************************************************************
	      header output
	 *************************************************************************/

	/** output about image **/
	signed int		output_errno;						// error number : SUCCESS=0
	unsigned short	output_width;						// image width of output image
	unsigned short	output_height;						// image height of output image
	unsigned short	output_y_pitch;						// y pitch of output image
	unsigned short	output_uv_pitch;					// uv pitch of output image
	unsigned short	output_y_offset;					// y offset of output image
	unsigned short	output_uv_offset;					// uv offset of output image
	unsigned short	output_original_width;				// image width of output image
	unsigned short	output_original_height;				// image height of output image
	//unsigned short	output_original_format;				// image format of output image
	unsigned int	output_buffer_y_length_needed;		// required length of buffer_y
	unsigned int	output_buffer_uv_length_needed;		// required length of buffer_uv
	unsigned int	output_buffer_temp_length_needed;	// required length of buffer_temp
	unsigned int	output_valid_lines;
	void *jd_struct;

	unsigned int	bRotate;							// default 1
	unsigned int	uNumComponents;
	unsigned int	uSamplingFactor[3];					// possible down-sampling factor
	unsigned int	uMaxSamplingFactor;					// maximum down-sampling factor, downsize_coeff must be lower than uMaxSamplingFactor
} NxJPEGDecParam;

typedef struct {
	/*** Exif decoding error number ***/
	int				output_errno;						// error number : SUCCESS=0

	/*** image info ***/
	
	/* if NULL, not exists */
	char			*image_description;		// image title
	char			*make;					// image input equipment manufacturer
	char			*model;					// image input equipment model
	char			*date_time;				// file change date and time
	char			*date_time_original;	// date and time of original data generation
	char			*date_time_digitized;	// date and time of digital data generation
	/* if length == 0, not exists */
	unsigned short	user_comment_length;	// user comment length (includes charactor code)
	unsigned char	*user_comment;			// user comment (not ends with 0)

	unsigned short orientation;

	/*** GPS info ***/
	
	unsigned char	GPS_exists;				// flag of GPS IFD existance

	/* if GPS exists, always exists */
	char			GPS_latitude_ref;		// north or south latitude : 'N' or 'S'
	unsigned int	GPS_lat_degrees_numer;	// latitude (degrees numerator)
	unsigned int	GPS_lat_degrees_denom;	// latitude (degrees denominator)
	unsigned int	GPS_lat_minutes_numer;	// latitude (minutes numerator)
	unsigned int	GPS_lat_minutes_denom;	// latitude (minutes denominator)
	unsigned int	GPS_lat_seconds_numer;	// latitude (seconds numerator)
	unsigned int	GPS_lat_seconds_denom;	// latitude (seconds denominator)
	char			GPS_longitude_ref;		// east or west longitude : 'E' or 'W'
	unsigned int	GPS_lon_degrees_numer;	// longitude (degrees numerator)
	unsigned int	GPS_lon_degrees_denom;	// longitude (degrees denominator)
	unsigned int	GPS_lon_minutes_numer;	// longitude (minutes numerator)
	unsigned int	GPS_lon_minutes_denom;	// longitude (minutes denominator)
	unsigned int	GPS_lon_seconds_numer;	// longitude (seconds numerator)
	unsigned int	GPS_lon_seconds_denom;	// longitude (seconds denominator)
	char			*GPS_map_datum;			// geodetic survey data used : "SEOUL", "TOKYO"

	/* if length == 0, not exists */
	unsigned short	GPS_processing_method_length;	// length of GPS processing method
	unsigned char	*GPS_processing_method;	// name of GPS processing method

	/***** user have to set character buffer *****/
	unsigned int	str_buf_length;
	char			*str_buf;
} NxJPEGDecExif;

#ifndef JPEGDECAPI
	#define JPEGDECAPI
#endif

int JPEGDECAPI NxJPEGDecGetVersionNum(int mode);
const char* JPEGDECAPI NxJPEGDecGetVersionInfo(int mode);

int  NxJPEGDec_initParam(NxJPEGDecParam *param);
void  NxJPEGDec_initExif(NxJPEGDecExif *exif);
int   NxJPEGDec_getHeader(NxImgInputStream *in, NxJPEGDecParam *jpeg,
						  NxJPEGDecParam *thumb, NxJPEGDecExif *exif);
int   NxJPEGDec_decode(NxImgInputStream *in, NxJPEGDecParam *jpeg,
					   NxJPEGDecParam *thumb, NxJPEGDecExif *exif);
unsigned short NxJPEGDec_getThumbLength(unsigned char *thumb_input);
unsigned int  NxJPEGDec_searchComment(NxImgInputStream *in,
									  unsigned int start_buf_len, const unsigned char *starts_with,
									  unsigned int comment_buf_len, unsigned char *comment_buf);
void NxJPEGDecClose(NxJPEGDecParam *param);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// NX_IMG_IO_STREAM_TYPE
