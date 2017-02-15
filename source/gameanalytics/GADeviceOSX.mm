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
        NSMethodSignature* methodSignsture = [[NSProcessInfo processInfo] methodSignatureForSelector:@selector(operatingSystemVersion)];
        NSInvocation* inv = [NSInvocation invocationWithMethodSignature:methodSignsture];
        inv.selector = @selector(operatingSystemVersion);
        [inv invokeWithTarget:[NSProcessInfo processInfo]];
        [inv getReturnValue:&version];
        
        NSString* v = [NSString stringWithFormat:@"%ld.%ld.%ld", version.majorVersion, version.minorVersion, (long)version.patchVersion];
        return [v UTF8String];
    }
    else
    {
        SInt32 majorVersion,minorVersion,bugFixVersion;
        
        Gestalt(gestaltSystemVersionMajor, &majorVersion);
        Gestalt(gestaltSystemVersionMinor, &minorVersion);
        Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
        
        NSString* v = [NSString stringWithFormat:@"%d.%d.%d", majorVersion, minorVersion, bugFixVersion];
        return [v UTF8String];
    }
}
