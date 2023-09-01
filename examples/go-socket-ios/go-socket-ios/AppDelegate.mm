//
//  AppDelegate.m
//  go-socket-ios
//
//  Created by zhaoxy on 2023/8/28.
//

#import "AppDelegate.h"
#import <gotcp/net_core.hpp>
#import <CoreTelephony/CTCellularData.h>

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Make sure the network is available
    NSURL *url = [NSURL URLWithString:@"https://www.stackoverflow.com"];
    NSURLSessionDataTask *task = [[NSURLSession sharedSession] dataTaskWithURL:url];
    [task resume];
    // Check if the network is available
    CTCellularData *cellularData = [CTCellularData new];
    cellularData.cellularDataRestrictionDidUpdateNotifier = ^(CTCellularDataRestrictedState state) {
        switch (state) {
            case kCTCellularDataRestricted:
                NSLog(@"Restricted");
                // Disconnect if the network is not available
                NetCore::DisconnectAsync(kDisCodeNetLost, false);
                break;
            case kCTCellularDataNotRestricted:
                NSLog(@"Not restricted");
                break;
            case kCTCellularDataRestrictedStateUnknown:
                NSLog(@"Unknown");
                break;
            default:
                break;
        }
    };
    // Connect the server
    [self connectServer];
    return YES;
}

- (void)connectServer {
    NetCore::InitHostAndPort("192.168.0.191", "192.168.0.191", 8080, true, [self]()->std::string{
        return "{\"username\":\"haha\", \"password\":\"xxxxx\"}";
    });
    NetCore::MakeSureConnected();
}


#pragma mark - UISceneSession lifecycle


- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


@end
