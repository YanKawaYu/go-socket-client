//
//  ViewController.m
//  go-socket-ios
//
//  Created by zhaoxy on 2023/8/28.
//

#import "ViewController.h"
#import <gotcp/net_core.hpp>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)buttonClicked:(id)sender {
    if (NetCore::GetConnectStatus() == kConnected) {
        NetCore::Send("chat.AddMessage", "{\"message\":\"This is a message\"}", [self](ErrorCode errCode, std::string respCstr){
            NSLog(@"Content from server: %@", [NSString stringWithCString:respCstr.c_str() encoding:NSUTF8StringEncoding]);
        });
    }else {
        UIAlertController* alert = [UIAlertController alertControllerWithTitle:@"Error"
                                   message:@"Failed to connect to the server"
                                   preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction* defaultAction = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                                       handler:^(UIAlertAction * action) {}];
        [alert addAction:defaultAction];
        [self presentViewController:alert animated:YES completion:nil];
    }
}

@end
