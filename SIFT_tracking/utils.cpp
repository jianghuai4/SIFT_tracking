/*
Miscellaneous utility functions.

Copyright (C) 2006  Rob Hess <hess@eecs.oregonstate.edu>

@version 1.1.1-20070330
*/
#include "stdafx.h"
#include "utils.h"

#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include <errno.h>
#include <string.h>
#include <stdlib.h>


/*************************** Function Definitions ****************************/


/*
Prints an error message and aborts the program.  The error message is
of the form "Error: ...", where the ... is specified by the \a format
argument

@param format an error message format string (as with \c printf(3)).
*/
void fatal_error(char* format, ...)
{
	va_list ap;

	fprintf( stderr, "Error: ");

	va_start( ap, format );
	vfprintf( stderr, format, ap );
	va_end( ap );
	fprintf( stderr, "\n" );
	abort();
}



/*
Replaces a file's extension, which is assumed to be everything after the
last dot ('.') character.

@param file the name of a file

@param extn a new extension for \a file; should not include a dot (i.e.
\c "jpg", not \c ".jpg") unless the new file extension should contain
two dots.

@return Returns a new string formed as described above.  If \a file does
not have an extension, this function simply adds one.
*/
char* replace_extension( const char* file, const char* extn )
{
	char* new_file, * lastdot;

	new_file = (char*)calloc( strlen( file ) + strlen( extn ) + 2,  sizeof( char ) );
	strcpy( new_file, file );
	lastdot = strrchr( new_file, '.' );
	if( lastdot )
		*(lastdot + 1) = '\0';
	else
		strcat( new_file, "." );
	strcat( new_file, extn );

	return new_file;
}



/*
A function that removes the path from a filename.  Similar to the Unix
basename command.

@param pathname a (full) path name

@return Returns the basename of \a pathname.
*/
char* basename( const char* pathname )
{
	char* base, * last_slash;

	last_slash = (char *)strrchr( pathname, '/' );
	if( ! last_slash )
	{
		base = (char *)calloc( strlen( pathname ) + 1, sizeof( char ) );
		strcpy( base, pathname );
	}
	else
	{
		base = (char *)calloc( strlen( last_slash++ ), sizeof( char ) );
		strcpy( base, last_slash );
	}

	return base;
}



/*
Displays progress in the console with a spinning pinwheel.  Every time this
function is called, the state of the pinwheel is incremented.  The pinwheel
has four states that loop indefinitely: '|', '/', '-', '\'.

@param done if 0, this function simply increments the state of the pinwheel;
otherwise it prints "done"
*/
void progress( int done )
{
	char state[4] = { '|', '/', '-', '\\' };
	static int cur = -1;

	if( cur == -1 )
		fprintf( stderr, "  " );

	if( done )
	{
		fprintf( stderr, "\b\bdone\n");
		cur = -1;
	}
	else
	{
		cur = ( cur + 1 ) % 4;
		fprintf( stdout, "\b\b%c ", state[cur] );
		fflush(stderr);
	}
}



/*
Erases a specified number of characters from a stream.

@param stream the stream from which to erase characters
@param n the number of characters to erase
*/
void erase_from_stream( FILE* stream, int n )
{
	int j;
	for( j = 0; j < n; j++ )
		fprintf( stream, "\b" );
	for( j = 0; j < n; j++ )
		fprintf( stream, " " );
	for( j = 0; j < n; j++ )
		fprintf( stream, "\b" );
}



/*
Doubles the size of an array with error checking

@param array pointer to an array whose size is to be doubled
@param n number of elements allocated for \a array
@param size size in bytes of elements in \a array

@return Returns the new number of elements allocated for \a array.  If no
memory is available, returns 0 and frees array.
*/
int array_double( void** array, int n, int size )
{
	void* tmp;

	tmp = realloc( *array, 2 * n * size );
	if( ! tmp )
	{
		fprintf( stderr, "Warning: unable to allocate memory in array_double(),"
			" %s line %d\n", __FILE__, __LINE__ );
		if( *array )
			free( *array );
		*array = NULL;
		return 0;
	}
	*array = tmp;
	return n*2;
}



/*
Calculates the squared distance between two points.

@param p1 a point
@param p2 another point
*/
double dist_sq_2D( CvPoint2D64f p1, CvPoint2D64f p2 )
{
	double x_diff = p1.x - p2.x;
	double y_diff = p1.y - p2.y;

	return x_diff * x_diff + y_diff * y_diff;
}



/*
Draws an x on an image.

@param img an image
@param pt the center point of the x
@param r the x's radius
@param w the x's line weight
@param color the color of the x
*/
void draw_x( IplImage* img, CvPoint pt, int r, int w, CvScalar color )
{
	cvLine( img, pt, cvPoint( pt.x + r, pt.y + r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x - r, pt.y + r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x + r, pt.y - r), color, w, 8, 0 );
	cvLine( img, pt, cvPoint( pt.x - r, pt.y - r), color, w, 8, 0 );
}



/*
Combines two images by scacking one on top of the other

@param img1 top image
@param img2 bottom image

@return Returns the image resulting from stacking \a img1 on top if \a img2
*/
extern IplImage* stack_imgs( IplImage* img1, IplImage* img2 )
{
	IplImage* stacked = cvCreateImage( cvSize( MAX(img1->width, img2->width),
		img1->height + img2->height ),
		IPL_DEPTH_8U, 3 );

	cvZero( stacked );
	cvSetImageROI( stacked, cvRect( 0, 0, img1->width, img1->height ) );
	cvAdd( img1, stacked, stacked, NULL );
	cvSetImageROI( stacked, cvRect(0, img1->height, img2->width, img2->height) );
	cvAdd( img2, stacked, stacked, NULL );
	cvResetImageROI( stacked );

	return stacked;
}



/*
Allows user to view an array of images as a video.  Keyboard controls
are as follows:

<ul>
<li>Space - start and pause playback</li>
<li>Page Down - skip forward 10 frames</li>
<li>Page Up - jump back 10 frames</li>
<li>Right Arrow - skip forward 1 frame</li>
<li>Left Arrow - jump back 1 frame</li>
<li>Backspace - jump back to beginning</li>
<li>Esc - exit playback</li>
<li>Closing the window also exits playback</li>
</ul>

@param imgs an array of images
@param n number of images in \a imgs
@param win_name name of window in which images are displayed
*/
void vid_view( IplImage** imgs, int n, char* win_name )
{
	int k, i = 0, playing = 0;

	cvNamedWindow( win_name, 1 );
	cvShowImage( win_name, imgs[i] );
	while( ! win_closed( win_name ) )
	{
		/* if already playing, advance frame and check for pause */
		if( playing )
		{
			i = MIN( i + 1, n - 1 );
			cvNamedWindow( win_name, 1 );
			cvShowImage( win_name, imgs[i] );
			k = cvWaitKey( 33 );
			if( k == ' '  ||  i == n - 1 )
				playing = 0;
		}

		else
		{
			k = cvWaitKey( 0 );
			switch( k )
			{
				/* space */
			case ' ':
				playing = 1;
				break;

				/* esc */
			case 27:
			case 1048603:
				cvDestroyWindow( win_name );
				break;

				/* backspace */
			case '\b':
				i = 0;
				cvNamedWindow( win_name, 1 );
				cvShowImage( win_name, imgs[i] );
				break;

				/* left arrow */
			case 65288:
			case 1113937:
				i = MAX( i - 1, 0 );
				cvNamedWindow( win_name, 1 );
				cvShowImage( win_name, imgs[i] );
				break;

				/* right arrow */
			case 65363:
			case 1113939:
				i = MIN( i + 1, n - 1 );
				cvNamedWindow( win_name, 1 );
				cvShowImage( win_name, imgs[i] );
				break;

				/* page up */
			case 65365:
			case 1113941:
				i = MAX( i - 10, 0 );
				cvNamedWindow( win_name, 1 );
				cvShowImage( win_name, imgs[i] );
				break;

				/* page down */
			case 65366:
			case 1113942:
				i = MIN( i + 10, n - 1 );
				cvNamedWindow( win_name, 1 );
				cvShowImage( win_name, imgs[i] );
				break;
			}
		}
	}
}



/*
Checks if a HighGUI window is still open or not

@param name the name of the window we're checking

@return Returns 1 if the window named \a name has been closed or 0 otherwise
*/
int win_closed( char* win_name )
{
	if( ! cvGetWindowHandle(win_name) )
		return 1;
	return 0;
}

/*
Calculates the squared Euclidian distance between two feature descriptors.

@param f1 first feature
@param f2 second feature

@return Returns the squared Euclidian distance between the descriptors of
f1 and f2.
*/
double descr_dist_sq( struct SIFT_feature_unit* f1, struct SIFT_feature_unit* f2 )
{
	double diff, dsq = 0;
	double* descr1, * descr2;
	int i, d;

	d = f1->d;
	if( f2->d != d )
		return DBL_MAX;
	descr1 = f1->descr;
	descr2 = f2->descr;

	for( i = 0; i < d; i++ )
	{
		diff = descr1[i] - descr2[i];
		dsq += diff*diff;
	}
	return dsq;
}

void ModifyTrackingWindows(Rect &trackingRect ,Rect* TrackingWindow,Rect WholeImageSize)
{
	TrackingWindow->upper = trackingRect.upper-cvRound(TRACKING_WINDOW_SIZE*trackingRect.height);
	TrackingWindow->left = trackingRect.left-cvRound(TRACKING_WINDOW_SIZE*trackingRect.width);
	TrackingWindow->width = trackingRect.width+2*cvRound(TRACKING_WINDOW_SIZE*trackingRect.width);
	TrackingWindow->height = trackingRect.height+2*cvRound(TRACKING_WINDOW_SIZE*trackingRect.height);

	if (TrackingWindow->upper<0)
	{
		TrackingWindow->height -=abs(TrackingWindow->upper);
		TrackingWindow->upper = 0;
	}
	if ((TrackingWindow->upper+TrackingWindow->height)>WholeImageSize.height)
	{
		TrackingWindow->height -=abs((TrackingWindow->upper+TrackingWindow->height)-WholeImageSize.height);
	}
	if (TrackingWindow->left<0)
	{
		TrackingWindow->width -=abs(TrackingWindow->left);
		TrackingWindow->left = 0;
	}
	if((TrackingWindow->left+TrackingWindow->width)>WholeImageSize.width)
	{
		TrackingWindow->width-=abs(TrackingWindow->left+TrackingWindow->width-WholeImageSize.width);
	}
}

//////////////////////////////////////////////////////////////////////////
//For optical flow
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//caculate the partial of the point p in image ipim
//note that the return Point2D,Vx=v.col,Vy = v.row
//////////////////////////////////////////////////////////////////////////
Point2D partial(IplImage* ipIm,Point2D p)
{
	Point2D v;
	v.dcol = (pixval8U(ipIm,p.row,p.col+1)-pixval8U(ipIm,p.row,p.col-1))/2.0;
	v.drow = (pixval8U(ipIm,p.row+1,p.col)-pixval8U(ipIm,p.row-1,p.col))/2.0;
	return v;
}

//////////////////////////////////////////////////////////////////////////
//caculate A
//////////////////////////////////////////////////////////////////////////

Point2D getOptFlow(IplImage* currentFrame,Point2D p,IplImage* preFrame)
{
	Point2D temp;
	double b[2];
	b[0]=0;b[1]=0;
	
	double M11=0,M12=0,M22=0;
	for(int i = -OPTICAL_FLOW_POINT_AREA/2; i < OPTICAL_FLOW_POINT_AREA/2; i++)
	{
		for (int j = -OPTICAL_FLOW_POINT_AREA/2;j < OPTICAL_FLOW_POINT_AREA/2;j++)
		{
			temp = partial(currentFrame,Point2D(p.row+i,p.col+j));
			M11 += temp.dcol*temp.dcol;
			M12 += temp.dcol*temp.drow;
			M22 += temp.drow*temp.drow;
			b[0] += temp.dcol*(pixval8U(currentFrame,p.row+i,p.col+j)-pixval8U(preFrame,p.row+i,p.col+j));
			b[1] += temp.drow*(pixval8U(currentFrame,p.row+i,p.col+j)-pixval8U(preFrame,p.row+i,p.col+j));
		}
	}
	double a[] = {M11,M12,M12,M22};
	CvMat M=cvMat(2, 2, CV_64FC1, a);
	CvMat *Mi = cvCloneMat(&M);
	cvInvert(&M,Mi,CV_SVD);
	temp.col=0;
	temp.row=0;
	b[0] = -b[0];
	b[1] = -b[1];
	CvMat Mb = cvMat(2,1,CV_64FC1,b);
	CvMat *Mr = cvCloneMat(&Mb);
	cvMatMul( Mi, &Mb, Mr);
	double vy = (cvmGet(Mr,1,0));
	double vx = (cvmGet(Mr,0,0));
	
	return (Point2D(vy,vx));
}