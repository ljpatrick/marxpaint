/* win32_print.c */

/* printing support for Tux Paint */
/* John Popplewell <john@johnnypops.demon.co.uk> */

/* Sept. 30, 2002 - Oct. 17, 2002 */


#include "SDL_syswm.h"
#include "win32_print.h"
#include "resource.h"


#define NOREF(x)        ((x)=(x))
#define GETHINST(hWnd)	((HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE )) 
#define MIR( id )	(MAKEINTRESOURCE( id ))


static int	bPrint = FALSE;
static HWND	hDlgCancel = NULL;

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

//static DEVMODE	*devmode = NULL;

BOOL CALLBACK AbortProc( HDC hDC, int nCode )
{
  MSG msg;

  NOREF(nCode);
  NOREF(hDC);
  while ( PeekMessage( (LPMSG)&msg, (HWND)NULL, 0, 0, PM_REMOVE) )
  { 
    if ( !IsDialogMessage( hDlgCancel, (LPMSG)&msg ) )
    {
      TranslateMessage( (LPMSG)&msg );
      DispatchMessage( (LPMSG)&msg );
    }
  }

  return bPrint;
}


LRESULT CALLBACK AbortPrintJob( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
  NOREF(hDlg);
  NOREF(lParam);
  NOREF(wParam);
  NOREF(message);
  switch ( message )
  { 
      case WM_INITDIALOG  :
	return TRUE;
      case WM_COMMAND	    :
	bPrint = FALSE;
	return TRUE;
      default		    :
	return FALSE;
  }
}


static SDL_Surface *make32bitDIB( SDL_Surface *surf )
{
  SDL_PixelFormat     pixfmt;
  SDL_Surface         *surf32;
  SDL_Surface         *surfDIB;
  Uint8               *src,*dst;
  Uint32              linesize;
  int                 i;

  memset( &pixfmt, 0, sizeof(pixfmt) );
  pixfmt.palette      = NULL;
  pixfmt.BitsPerPixel = 32;
  pixfmt.BytesPerPixel= 4;
  pixfmt.Rmask        = 0x00FF0000;
  pixfmt.Gmask        = 0x0000FF00;
  pixfmt.Bmask        = 0x000000FF;
  pixfmt.Amask        = 0xFF000000;
  pixfmt.Rshift       = 16;
  pixfmt.Gshift       = 8;
  pixfmt.Bshift       = 0;
  pixfmt.Ashift       = 24;
  pixfmt.Rloss        = 0;
  pixfmt.Gloss        = 0;
  pixfmt.Bloss        = 0;
  pixfmt.Aloss        = 0;
  pixfmt.colorkey     = 0;
  pixfmt.alpha        = 0;

  surf32 = SDL_ConvertSurface( surf, &pixfmt, SDL_SWSURFACE );
  surfDIB = SDL_CreateRGBSurface( SDL_SWSURFACE, surf32->w, surf32->h, 32, 
                    pixfmt.Rmask, pixfmt.Gmask, pixfmt.Bmask, pixfmt.Amask );

  linesize = surf32->w*sizeof(Uint32);    /* Flip top2bottom */
  dst = surfDIB->pixels;
  src = ((Uint8*)surf32->pixels)+((surf32->h-1)*surf32->pitch);
  for ( i = 0; i < surf32->h; ++i )
  {
      memcpy( dst, src, linesize );
      src -= surf32->pitch;
      dst += surfDIB->pitch;
  }

  SDL_FreeSurface( surf32 );              /* Free temp surface */

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


static HDC GetDefaultPrinterDC( void )
{
  char	device[MAX_PATH],driver[MAX_PATH],output[MAX_PATH];

  if ( GetDefaultPrinterStrings( device, driver, output ) )
    return CreateDC( driver, device, output, NULL );

  return NULL;
}


static HANDLE LoadCustomPrinterHDEVMODE( HWND hWnd, const char *filepath )
{
  char	  device[MAX_PATH];
  HANDLE  hPrinter = NULL;
  int	  sizeof_devmode;
  HGLOBAL hDevMode = NULL;
  DEVMODE *devmode = NULL;
  int	  res;
  FILE	  *fp = NULL;

  if ( !GetDefaultPrinterStrings( device, NULL, NULL ) )
    return NULL;

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

  if ( (fp = fopen( filepath, "rb" )) != NULL )
  {
    int	  block_size = devmode->dmSize + devmode->dmDriverExtra;
    int	  block_read = fread( devmode, 1, block_size, fp );

    fclose( fp );
    if ( block_size != block_read )
      goto err_exit;
  }

  res = DocumentProperties( hWnd, hPrinter, device, devmode, devmode, 
			    DM_IN_BUFFER|DM_OUT_BUFFER);
  if ( res != IDOK )
    goto err_exit;

  GlobalUnlock( hDevMode );
  ClosePrinter( hPrinter );
  return hDevMode;

err_exit:
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
    int	     block_writ = fwrite( devmode, 1, block_size, fp );

    GlobalUnlock( hDevMode );
    fclose( fp );
    return block_size == block_writ;
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


int IsPrinterAvailable( void )
{
  return (GetDefaultPrinterStrings( NULL, NULL, NULL ) != 0);
}


int SurfacePrint( SDL_Surface *surf, const char *printcfg, int showdialog )
{
  int                 res = 0;
  HWND                hWnd;
  DOCINFO	      di;
  int		      nError;
  SDL_SysWMinfo       wminfo;
  HDC                 hDCwindow;
  HDC                 hDCprinter;
  BITMAPINFOHEADER    bmih;
  SDL_Surface         *surf32 = NULL;
  RECT                rc;
  float               fLogPelsX1, fLogPelsY1, fLogPelsX2, fLogPelsY2;
  float               fScaleX, fScaleY;
  int                 cWidthPels, xLeft, yTop;
  float               subscaler,subscalerx,subscalery;
  int                 hDCCaps;
  HANDLE              hOldObject = NULL;
  HBITMAP             hbm = NULL;
  HDC                 hdcMem = NULL;

  SDL_VERSION(&wminfo.version);
  if ( !SDL_GetWMInfo( &wminfo ) )
    return -1;
  hWnd = wminfo.window;

  if ( !printcfg )
    hDCprinter = GetDefaultPrinterDC();
  else
    hDCprinter = GetCustomPrinterDC( hWnd, printcfg, showdialog );

  if ( !hDCprinter )
    return -1;

  bPrint = TRUE;
  SetAbortProc( hDCprinter, AbortProc );
  hDlgCancel = CreateDialog( GETHINST(hWnd), MIR(IDD_ABORTDLG), hWnd, (DLGPROC)AbortPrintJob );
  EnableWindow( hWnd, FALSE );

  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = "Tux Paint";
  di.lpszOutput = (LPTSTR)NULL;
  di.lpszDatatype = (LPTSTR)NULL;
  di.fwType = 0;

  nError = StartDoc( hDCprinter, &di );
  if ( nError == SP_ERROR )
  {
    res = -2;
    goto error;
  }

  nError = StartPage(hDCprinter);
  if (nError <= 0)
  {
    res = -3;
    goto error;
  }

//////////////////////////////////////////////////////////////////////////////////////

  surf32 = make32bitDIB( surf );
  if ( !surf32 )
  {
    res = -4;
    goto error;
  }

  memset( &bmih,0, sizeof(bmih) );
  bmih.biSize         = sizeof(bmih);
  bmih.biPlanes       = 1;
  bmih.biCompression  = BI_RGB;
  bmih.biBitCount     = 32;
  bmih.biWidth        = surf32->w;
  bmih.biHeight       = surf32->h;

  GetClientRect( hWnd, &rc );
  subscalerx = (float)rc.right/surf32->w;
  subscalery = (float)rc.bottom/surf32->h;
  subscaler = subscalery;
  if ( subscalerx < subscalery )
    subscaler = subscalerx;

  hDCwindow = GetDC( hWnd );
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

  if ( hDCCaps & RC_STRETCHDIB )
  {
    StretchDIBits(hDCprinter, xLeft, yTop,
                  (int)(fScaleX*bmih.biWidth),
                  (int)(fScaleY*bmih.biHeight),
                  0, 0, bmih.biWidth, bmih.biHeight,
                  surf32->pixels, (BITMAPINFO*)&bmih,
                  DIB_RGB_COLORS, SRCCOPY);
  }
  else
  if ( hDCCaps & RC_STRETCHBLT )
  {
    hbm = CreateDIBitmap(hDCprinter, &bmih, CBM_INIT, 
                         surf32->pixels, (const BITMAPINFO*)&bmih, 0);
    if ( hbm )
    {
      hdcMem = CreateCompatibleDC( hDCprinter );
      if ( hdcMem )
      {
        hOldObject = SelectObject(hdcMem, hbm);
        if ( hOldObject ) 
        {
          StretchBlt(hDCprinter, xLeft, yTop,
                     (int)(fScaleX*bmih.biWidth),
                     (int)(fScaleY*bmih.biHeight),
                     hdcMem, 0, 0, bmih.biWidth, bmih.biHeight, SRCCOPY);
          SelectObject(hdcMem, hOldObject);
        }
      }
    }
  }
  else
  {
    res = -10;
    goto error;
  }

//////////////////////////////////////////////////////////////////////////////////////

  nError = EndPage( hDCprinter );
  if ( nError <= 0 )
  {
    res = -9;
    goto error;
  }

  EndDoc( hDCprinter );

error: 
  if ( hdcMem ) DeleteDC( hdcMem );
  if ( hbm ) DeleteObject( hbm );
  if ( surf32 ) SDL_FreeSurface( surf32 );

  EnableWindow( hWnd, TRUE );
  DestroyWindow( hDlgCancel );
  DeleteDC( hDCprinter );

  return res;
}


