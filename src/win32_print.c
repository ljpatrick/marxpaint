/* win32_print.c */
/* printing support for Tux Paint */

/* John Popplewell <john@johnnypops.demon.co.uk> */

/* Sept. 30, 2002 - Oct. 17, 2002 */
/* Oct.  07, 2003 - added banding support */
/*                - prints using 24-bit (not 32-bit) bitmap */

#include "SDL_syswm.h"
#include "win32_print.h"
#include "resource.h"


#define NOREF(x)        ((x)=(x))
#define GETHINST(hWnd)	((HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE )) 
#define MIR( id )	    (MAKEINTRESOURCE( id ))


static PRINTDLG	global_pd = {
  sizeof(PRINTDLG),
  NULL, NULL, NULL, NULL,
  PD_RETURNDC,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  1,
  NULL,
};


static SDL_Surface *make24bitDIB( SDL_Surface *surf )
{
  SDL_PixelFormat     pixfmt;
  SDL_Surface         *surf24;
  SDL_Surface         *surfDIB;
  Uint8               *src,*dst;
  Uint32              linesize;
  int                 i;

  memset( &pixfmt, 0, sizeof(pixfmt) );
  pixfmt.palette      = NULL;
  pixfmt.BitsPerPixel = 24;
  pixfmt.BytesPerPixel= 3;
  pixfmt.Rmask        = 0x00FF0000;
  pixfmt.Gmask        = 0x0000FF00;
  pixfmt.Bmask        = 0x000000FF;
  pixfmt.Amask        = 0;
  pixfmt.Rshift       = 16;
  pixfmt.Gshift       = 8;
  pixfmt.Bshift       = 0;
  pixfmt.Ashift       = 0;
  pixfmt.Rloss        = 0;
  pixfmt.Gloss        = 0;
  pixfmt.Bloss        = 0;
  pixfmt.Aloss        = 0;
  pixfmt.colorkey     = 0;
  pixfmt.alpha        = 0;

  surf24 = SDL_ConvertSurface( surf, &pixfmt, SDL_SWSURFACE );
  surfDIB = SDL_CreateRGBSurface( SDL_SWSURFACE, surf24->w, surf24->h, 24, 
                    pixfmt.Rmask, pixfmt.Gmask, pixfmt.Bmask, pixfmt.Amask );

  linesize = surf24->w * 3;    // Flip top2bottom
  dst = surfDIB->pixels;
  src = ((Uint8*)surf24->pixels)+((surf24->h-1)*surf24->pitch);
  for ( i = 0; i < surf24->h; ++i )
  {
      memcpy( dst, src, linesize );
      src -= surf24->pitch;
      dst += surfDIB->pitch;
  }

  SDL_FreeSurface( surf24 );   // Free temp surface

  return surfDIB;
}

/* returns 0 if failed */
static int GetDefaultPrinterStrings( char *device, char *driver, char *output )
{
  char	buff[MAX_PATH];
  char  *section = "windows";
  char  *key = "device";
  char  *def = "NODEFAULTPRINTER";
  char	*dev,*drv,*out;
     
  if ( !GetProfileString( section, key, def, buff, sizeof(buff) ) )
    return 0;
 
  if ( strcmp( buff, def ) == 0 )
    return 0;

  if ( ((dev = strtok( buff, "," )) != NULL) &&
       ((drv = strtok( NULL, ", ")) != NULL) &&
       ((out = strtok( NULL, ", ")) != NULL) )
  {
    if ( device ) strcpy( device, dev );
    if ( driver ) strcpy( driver, drv );
    if ( output ) strcpy( output, out );
    return 1;
  }
  return 0;
}

#define dmDeviceNameSize    32

static HANDLE LoadCustomPrinterHDEVMODE( HWND hWnd, const char *filepath )
{
  char	  device[MAX_PATH];
  HANDLE  hPrinter = NULL;
  int	  sizeof_devmode;
  HGLOBAL hDevMode = NULL;
  DEVMODE *devmode = NULL;
  int	  res;
  FILE	  *fp = NULL;
  int	  block_size;
  int	  block_read;

  if ( (fp = fopen( filepath, "rb" )) == NULL )
    return NULL;

  if ( fread( device, 1, dmDeviceNameSize, fp ) != dmDeviceNameSize )
    goto err_exit;

  if (!OpenPrinter( device, &hPrinter, NULL ))
    return NULL;

  sizeof_devmode = (int)DocumentProperties( hWnd, hPrinter, device, 
					    NULL, NULL, 0 );

  if ( !sizeof_devmode )
    goto err_exit;

  hDevMode = GlobalAlloc( GHND, sizeof_devmode );
  if ( !hDevMode )
    goto err_exit;

  devmode = (DEVMODE*)GlobalLock( hDevMode );
  if ( !devmode )
    goto err_exit;

  res = DocumentProperties( hWnd, hPrinter, device, devmode, NULL, 
			    DM_OUT_BUFFER);
  if ( res != IDOK )
    goto err_exit;

  block_size = devmode->dmSize + devmode->dmDriverExtra;
  block_read = fread( devmode, 1, block_size, fp );
  if ( block_size != block_read )
    goto err_exit;
  fclose(fp);

  res = DocumentProperties( hWnd, hPrinter, device, devmode, devmode, 
			    DM_IN_BUFFER|DM_OUT_BUFFER);
  if ( res != IDOK )
    goto err_exit;

  GlobalUnlock( hDevMode );
  ClosePrinter( hPrinter );
  return hDevMode;

err_exit:
  if ( fp ) fclose( fp );
  if ( devmode ) GlobalUnlock( hDevMode );
  if ( hDevMode ) GlobalFree( hDevMode );
  if ( hPrinter ) ClosePrinter( hPrinter );
  return NULL;
}


static int SaveCustomPrinterHDEVMODE( HWND hWnd, const char *filepath, HANDLE hDevMode )
{
  FILE	  *fp = NULL;

  NOREF(hWnd);
  if ( (fp = fopen( filepath, "wb" )) != NULL )
  {
    DEVMODE *devmode = (DEVMODE*)GlobalLock( hDevMode );
    int	     block_size = devmode->dmSize + devmode->dmDriverExtra;
    int	     block_written;
    char     devname[dmDeviceNameSize];

    strcpy( devname, (const char*)devmode->dmDeviceName );
    fwrite( devname, 1, sizeof(devname), fp );
    block_written = fwrite( devmode, 1, block_size, fp );
    GlobalUnlock( hDevMode );
    fclose( fp );
    return block_size == block_written;
  }
  return 0;
}


static HDC GetCustomPrinterDC( HWND hWnd, const char *printcfg, int show )
{
  global_pd.hwndOwner = hWnd;
  global_pd.hDC       = NULL;
  global_pd.hDevNames = NULL;

  if ( global_pd.hDevMode == NULL )
  {
    global_pd.hDevMode  = LoadCustomPrinterHDEVMODE( hWnd, printcfg );
  }

  if ( show )
  {
    if ( PrintDlg( &global_pd ) )
    {
      SaveCustomPrinterHDEVMODE( hWnd, printcfg, global_pd.hDevMode );
      return global_pd.hDC;
    }
    return NULL;
  }

  {
    DEVMODE *devmode = (DEVMODE*)GlobalLock( global_pd.hDevMode );

    global_pd.hDC = CreateDC( NULL, (const char*)devmode->dmDeviceName, NULL, devmode );
    GlobalUnlock( global_pd.hDevMode );
  }
  return global_pd.hDC;
}


static HDC GetDefaultPrinterDC( void )
{
  char	device[MAX_PATH],driver[MAX_PATH],output[MAX_PATH];

  if ( GetDefaultPrinterStrings( device, driver, output ) )
    return CreateDC( driver, device, output, NULL );

  return NULL;
}


static HDC GetPrinterDC( HWND hWnd, const char *printcfg, int show )
{
  if ( !printcfg ) return GetDefaultPrinterDC();
  return GetCustomPrinterDC( hWnd, printcfg, show );
}


static int IsBandingRequired( HDC hPrinter )
{
  OSVERSIONINFO osvi;
  int           indata = NEXTBAND;

  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if ( GetVersionEx( &osvi ) && (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) )
    return Escape( hPrinter, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&indata, NULL );
  return 0;
}


int IsPrinterAvailable( void )
{
  return (GetDefaultPrinterStrings( NULL, NULL, NULL ) != 0);
}


const char *SurfacePrint( SDL_Surface *surf, const char *printcfg, int showdialog )
{
  char                *res = NULL;
  HWND                hWnd;
  DOCINFO             di;
  int                 nError;
  SDL_SysWMinfo       wminfo;
  HDC                 hDCwindow;
  HDC                 hDCprinter;
  BITMAPINFOHEADER    bmih;
  SDL_Surface         *surf24 = NULL;
  RECT                rc;
  float               fLogPelsX1, fLogPelsY1, fLogPelsX2, fLogPelsY2;
  float               fScaleX, fScaleY;
  int                 cWidthPels, xLeft, yTop;
  float               subscaler,subscalerx,subscalery;
  int                 hDCCaps;
  HBITMAP             hbm = NULL;
  HDC                 hdcMem = NULL;

  SDL_VERSION(&wminfo.version);
  if ( !SDL_GetWMInfo( &wminfo ) )
    return "win32_print: SDL_GetWMInfo() failed.";

  hWnd = wminfo.window;
  hDCprinter = GetPrinterDC( hWnd, printcfg, showdialog );
  if ( !hDCprinter )
    return "win32_print: GetPrinterDC() failed.";

  EnableWindow( hWnd, FALSE );

  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = "Tux Paint";
  di.lpszOutput = (LPTSTR)NULL;
  di.lpszDatatype = (LPTSTR)NULL;
  di.fwType = 0;

  nError = StartDoc( hDCprinter, &di );
  if ( nError == SP_ERROR )
  {
    res = "win32_print: StartDoc() failed.";
    goto error;
  }

  nError = StartPage( hDCprinter );
  if (nError <= 0)
  {
    res = "win32_print: StartPage() failed.";
    goto error;
  }

//////////////////////////////////////////////////////////////////////////////////////

  surf24 = make24bitDIB( surf );
  if ( !surf24 )
  {
    res = "win32_print: make24bitDIB() failed.";
    goto error;
  }

  memset( &bmih,0, sizeof(bmih) );
  bmih.biSize         = sizeof(bmih);
  bmih.biPlanes       = 1;
  bmih.biCompression  = BI_RGB;
  bmih.biBitCount     = 24;
  bmih.biWidth        = surf24->w;
  bmih.biHeight       = surf24->h;

  GetClientRect( hWnd, &rc );
  subscalerx = (float)rc.right/surf24->w;
  subscalery = (float)rc.bottom/surf24->h;
  subscaler = subscalery;
  if ( subscalerx < subscalery )
    subscaler = subscalerx;

  hDCwindow = GetDC( hWnd );
  if ( !hDCwindow )
  {
    res = "win32_print: failed to get window DC.";
    goto error;
  }
  fLogPelsX1 = (float)GetDeviceCaps(hDCwindow, LOGPIXELSX);
  fLogPelsY1 = (float)GetDeviceCaps(hDCwindow, LOGPIXELSY);
  ReleaseDC( hWnd, hDCwindow );

  fLogPelsX2 = (float)GetDeviceCaps(hDCprinter, LOGPIXELSX);
  fLogPelsY2 = (float)GetDeviceCaps(hDCprinter, LOGPIXELSY);

  if (fLogPelsX1 > fLogPelsX2)
    fScaleX = (fLogPelsX1/fLogPelsX2);
  else
    fScaleX = (fLogPelsX2/fLogPelsX1);

  if (fLogPelsY1 > fLogPelsY2)
    fScaleY = (fLogPelsY1/fLogPelsY2);
  else
    fScaleY = (fLogPelsY2/fLogPelsY1);

  fScaleX *= subscaler;
  fScaleY *= subscaler;

  yTop = 0;
  cWidthPels = GetDeviceCaps(hDCprinter, PHYSICALWIDTH);
  xLeft = ((cWidthPels - ((int)(fScaleX*bmih.biWidth)))/2)-
                          GetDeviceCaps(hDCprinter, PHYSICALOFFSETX);

  hDCCaps = GetDeviceCaps(hDCprinter, RASTERCAPS);

  if (hDCCaps & RC_PALETTE)
  {
	  res = "win32_print: printer context requires palette.";
	  goto error;
  }

  if ( IsBandingRequired( hDCprinter ) )
  {
    RECT  rcBand = { 0 };
    RECT  rcPrinter;
    RECT  rcImage;

    SetRect( &rcPrinter, xLeft, yTop, (int)(fScaleX*bmih.biWidth), (int)(fScaleY*bmih.biHeight) );
    SetRect( &rcImage, 0, 0, bmih.biWidth, bmih.biHeight );

    while ( Escape(hDCprinter, NEXTBAND, 0, NULL, &rcBand) )
    {
      if ( IsRectEmpty( &rcBand) ) break;
      if ( IntersectRect( &rcBand, &rcBand, &rcPrinter ) )
      {
        rcImage.top = (int)(0.5f+(float)rcBand.top/fScaleX);
        rcImage.bottom = (int)(0.5f+(float)rcBand.bottom/fScaleX);

        SetStretchBltMode( hDCprinter, COLORONCOLOR );
        nError = StretchDIBits(hDCprinter, rcBand.left, rcBand.top,
                      rcBand.right - rcBand.left,
                      rcBand.bottom - rcBand.top,
                      rcImage.left, rcImage.top,
                      rcImage.right - rcImage.left,
                      rcImage.bottom - rcImage.top,
                      surf24->pixels, (BITMAPINFO*)&bmih,
                      DIB_RGB_COLORS, SRCCOPY);
        if ( nError == GDI_ERROR )
        {
	        res = "win32_print: StretchDIBits() failed.";
	        goto error;
        }
      }
    }
  }
  else
  {
    if ( hDCCaps & RC_STRETCHDIB )
    {
      SetStretchBltMode(hDCprinter, COLORONCOLOR);

      nError = StretchDIBits(hDCprinter, xLeft, yTop,
                    (int)(fScaleX*bmih.biWidth),
                    (int)(fScaleY*bmih.biHeight),
                    0, 0, bmih.biWidth, bmih.biHeight,
                    surf24->pixels, (BITMAPINFO*)&bmih,
                    DIB_RGB_COLORS, SRCCOPY);
      if ( nError == GDI_ERROR )
      {
	      res = "win32_print: StretchDIBits() failed.";
	      goto error;
      }
    }
    else
    {
      res = "win32_print: StretchDIBits() not available.";
      goto error;
    }
  }

//////////////////////////////////////////////////////////////////////////////////////

  nError = EndPage( hDCprinter );
  if ( nError <= 0 )
  {
    res = "win32_print: EndPage() failed.";
    goto error;
  }

  EndDoc( hDCprinter );

error: 
  if ( hdcMem ) DeleteDC( hdcMem );
  if ( hbm ) DeleteObject( hbm );
  if ( surf24 ) SDL_FreeSurface( surf24 );

  EnableWindow( hWnd, TRUE );
  DeleteDC( hDCprinter );

  return res;
}
