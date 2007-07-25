/*   SDLMain.m - main entry point for our Cocoa-ized SDL app
       Initial Version: Darrell Walisser <dwaliss1@purdue.edu>
       Non-NIB-Code & other changes: Max Horn <max@quendi.de>

    Feel free to customize this file to suit your needs
	 $Id$
*/

//#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_2
//#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_2

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
{
}

- (IBAction)onNew:(id)sender;
- (IBAction)onOpen:(id)sender;
- (IBAction)onSave:(id)sender;
- (IBAction)onPrint:(id)sender;
- (IBAction)onPageSetup:(id)sender;
- (IBAction)onUndo:(id)sender;
- (IBAction)onRedo:(id)sender;
- (IBAction)onHelp:(id)sender;
- (IBAction)onQuit:(id)sender;

- (void) sendSDLControlKeystroke:(int)key;
- (void) sendSDLControlShiftKeystroke:(int)key;
- (void) setupBridge;

@end
