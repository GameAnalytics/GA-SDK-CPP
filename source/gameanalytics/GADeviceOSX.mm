#import "GADeviceOSX.h"
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>

typedef struct {
    NSInteger majorVersion;
    NSInteger minorVersion;
    NSInteger patchVersion;
} MyOperatingSystemVersion;

const char* getOSXVersion()
{
    if([[NSProcessInfo processInfo] respondsToSelector:@selector(operatingSystemVersion)])
    {
        MyOperatingSystemVersion version;
        NSMethodSignature* methodSignature = [[NSProcessInfo processInfo] methodSignatureForSelector:@selector(operatingSystemVersion)];
        NSInvocation* inv = [NSInvocation invocationWithMethodSignature:methodSignature];
        inv.selector = @selector(operatingSystemVersion);
        [inv invokeWithTarget:[NSProcessInfo processInfo]];
        [inv getReturnValue:&version];
        
        NSString* v = [NSString stringWithFormat:@"%ld.%ld.%ld", version.majorVersion, version.minorVersion, (long)version.patchVersion];
        return [v UTF8String];
    }
    else
    {
        SInt32 majorVersion,minorVersion,bugFixVersion;
        
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        Gestalt(gestaltSystemVersionMajor, &majorVersion);
        Gestalt(gestaltSystemVersionMinor, &minorVersion);
        Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
#pragma GCC diagnostic pop
        
        NSString* v = [NSString stringWithFormat:@"%d.%d.%d", majorVersion, minorVersion, bugFixVersion];
        return [v UTF8String];
    }
}
