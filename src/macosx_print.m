//
//  macosx_print.m
//  Tux Paint
//
//  Created by Darrell Walisser on Sat Mar 15 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//  $Id$
//

#import "macosx_print.h"
#import <Cocoa/Cocoa.h>

// this object presents the image to the printing layer
@interface ImageView : NSView
{
    NSImage* _image;
}
- (void) setImage:(NSImage*)image;
@end

@implementation ImageView

- (void) setImage:(NSImage*)image
{
    _image = [ image retain ];
}

- (void) drawRect:(NSRect)rect
{
    [ _image compositeToPoint: NSMakePoint (0, 0) operation: NSCompositeCopy ];
}
@end

// this object waits for the print dialog to go away
@interface ModalDelegate : NSObject
{
    BOOL	_complete;
    BOOL    _wasOK;
}
- (id)   init;
- (BOOL) wait;
- (void) reset;
- (BOOL) wasOK;
@end

@implementation ModalDelegate

- (id) init
{
    self = [ super init ];
    _complete = NO;
    _wasOK = NO;
    return self;
}

- (BOOL) wait
{
    while (!_complete) {
    
        NSEvent *event;
         
        event = [ NSApp nextEventMatchingMask:NSAnyEventMask
                        untilDate:[ NSDate distantFuture ]
                        inMode: NSDefaultRunLoopMode dequeue:YES ];
        
        [ NSApp sendEvent:event ];
    }
    
    return [ self wasOK ];
}

- (void) reset
{
    _complete = NO;
    _wasOK = NO;
}

- (BOOL) wasOK
{
    return _wasOK;
}

- (void)printDidRun:(NSPrintOperation *)printOperation 
            success:(BOOL)success contextInfo:(void *)contextInfo
{
    _complete = YES;
    _wasOK = success;
}

- (void)pageLayoutEnded:(NSPageLayout *)pageLayout 
             returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    _complete = YES;
    _wasOK = returnCode == NSOKButton;
}

@end

static NSImage* CreateImage (SDL_Surface *surface)
{
    NSBitmapImageRep* imageRep;
    NSSize            imageSize;
    NSImage*          image;
    SDL_Surface*      surface32RGBA;
    
    
    // convert surface to 32bit RGBA
    surface32RGBA = SDL_CreateRGBSurface (SDL_SWSURFACE, surface->w, surface->h,
                                          32, 0xff<<24, 0xff<<16, 0xff<<8, 0xff<<0);
    if (surface32RGBA == NULL) {
        NSLog (@"CreateImage: Cannot allocate conversion surface");
        return nil;
    }
    
    SDL_BlitSurface (surface, NULL, surface32RGBA, NULL);
    
    // convert surface to an NSBitmapImageRep
    imageRep = [ [ NSBitmapImageRep alloc] 
                    initWithBitmapDataPlanes:(unsigned char **)&surface32RGBA->pixels 
                        pixelsWide:surface->w 
                        pixelsHigh:surface->h 
                        bitsPerSample:8
                        samplesPerPixel:4 
                        hasAlpha:YES
                        isPlanar:NO
                        colorSpaceName:NSDeviceRGBColorSpace 
                        bytesPerRow:surface->w * 4
                        bitsPerPixel:32 ];
    if (imageRep == nil) {
        NSLog (@"CreateImage: Could not create image representation.");
        return nil;
    }
    
    imageSize = NSMakeSize (surface->w, surface->h);
    
    image = [ [ NSImage alloc ] initWithSize:imageSize ];
    if (image == nil) {
        NSLog (@"CreateImage: Could not allocate image");
        return nil;
    }
    
    [ image addRepresentation:imageRep ];
    
    [ image autorelease ];
    [ imageRep release ];
    free (surface32RGBA);
    
    return image;
}


const char* SurfacePrint (SDL_Surface *surface, int showDialog)
{
    NSImage*          image;
    ImageView*        printView;
    NSWindow*         printWindow;
    NSPrintOperation* printOperation;
    NSPrintInfo*      printInfo;
    ModalDelegate*    delegate;
    BOOL              ok = YES;
    const char*       error = NULL;
    
    // create image for surface
    image = CreateImage (surface);
    if (image == nil)
        return "Could not create image";

    // create print view
    printView = [ [ ImageView alloc ] initWithFrame: NSMakeRect (0, 0, surface->w, surface->h) ];
    if (printView == nil)
        return "Could not create print view";
    [ printView setImage:image ];
    
    // attach view to offscreen window
    printWindow = [ [ NSWindow alloc ]
                        initWithContentRect: NSMakeRect (0, 0, surface->w, surface->h)
                        styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered
                        defer:NO ];
    if (printWindow == nil)
        return "Could not create offscreen window";
        
    [ printWindow setContentView:printView ];
    
    // create print control objects
    printInfo = [ NSPrintInfo sharedPrintInfo ];
    
    delegate = [ [ ModalDelegate alloc ] init ];
    
    // run page layout dialog
    if (showDialog) {
    
        NSPageLayout*     pageLayout;
        
        pageLayout = [ NSPageLayout pageLayout ];
        [ pageLayout beginSheetWithPrintInfo:printInfo 
            modalForWindow:[ NSApp mainWindow ]
            delegate:delegate
            didEndSelector:@selector(pageLayoutEnded:returnCode:contextInfo:)
            contextInfo:nil ];
        
        ok = [ delegate wait ];
        [ delegate reset ];
    }

    if (!ok) {
        error = "Canceled printing at page layout";
        goto bail;
    }
    
    // run printing
    printOperation = [ NSPrintOperation printOperationWithView:printView printInfo:printInfo ];
    [ printOperation setShowPanels:showDialog ];
    
    [ printOperation runOperationModalForWindow:[ NSApp mainWindow ]
        delegate:delegate didRunSelector:@selector(printDidRun:success:contextInfo:) contextInfo:nil ];
    
    ok = [ delegate wait ];
    if (!ok)
        error = "Canceled or error when printing";
  
  bail:       
    // cleanup
    [ delegate release ];
    [ image release ];
    [ printView release ];
    [ printWindow release ];
    
    return error;
}

