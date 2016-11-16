
#ifndef JUICE_LIB_H
#define JUICE_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libudev.h>
#include <libinput.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include "linux/input.h"

static int open_restricted(const char *path, int flags,
        void *user_data);
static void close_restricted(int fd,
        void *user_data);
static void sighandler(int signal, siginfo_t *siginfo,
        void *userdata);

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static int open_restricted(const char *path, int flags, void *user_data)
{
    int fd=open(path,flags);
    return fd;
}

static void close_restricted(int fd, void *user_data)
{
    close(fd);
}

typedef struct juice_input
{
    void *userdata;
    struct udev *udev;
    struct libinput *libinput;
}juice_input_t;

juice_input_t *InitInput()
{
    juice_input_t *pItem=(juice_input_t *)malloc(sizeof(juice_input_t));
    if(pItem!=NULL)
    {
        pItem->userdata=NULL;
        pItem->udev=udev_new();
        pItem->libinput=libinput_udev_create_context(&interface, pItem->userdata, pItem->udev);
        if(libinput_udev_assign_seat(pItem->libinput,"seat0"))
        {
            libinput_unref(pItem->libinput);
            udev_unref(pItem->udev);
            free(pItem);
            pItem=NULL;
        }
    }
    return pItem;
}

void FreeInput(juice_input_t *pItem)
{
    if(pItem!=NULL)
    {
        free(pItem);
        pItem=NULL;
    }
}

void process_input()
{
    int status=1;
    struct udev *udev;
    struct libinput *libinput;
    void *userdata=NULL;
    struct pollfd fds;
    struct sigaction action;
    struct libinput_event *pEvent;
    struct libinput_event_pointer *pPointer=NULL;
    struct libinput_event_touch *pTouch=NULL;
    struct libinput_event_keyboard *pKeyboard=NULL;
    enum libinput_key_state state;
    // set libinput
    udev=udev_new();
    libinput=libinput_udev_create_context(&interface, userdata, udev);
    if(libinput_udev_assign_seat(libinput,"seat0"))
    {
        libinput_unref(libinput);
        udev_unref(udev);
        exit(1);
    }
    // set fds
    fds.fd=libinput_get_fd(libinput);
    fds.events=POLLIN;
    fds.revents=0;

    /*
    // set action
    memset(&action,0,sizeof(action));
    action.sa_sigaction=sighandler;
    action.sa_flags=SA_SIGINFO;
    sigaction(SIGINT,&action,NULL);
    */

    while(status && poll(&fds,1,-1)>-1)
    {
        libinput_dispatch(libinput);
        while((pEvent=libinput_get_event(libinput)))
        {
            switch(libinput_event_get_type(pEvent))
            {
                case LIBINPUT_EVENT_NONE:
                    abort();
                case LIBINPUT_EVENT_DEVICE_ADDED:
                    break;
                case LIBINPUT_EVENT_DEVICE_REMOVED:
                    break;
                case LIBINPUT_EVENT_KEYBOARD_KEY:
                    break;
                case LIBINPUT_EVENT_POINTER_MOTION:
                    break;
                case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
                    break;
                case LIBINPUT_EVENT_TOUCH_DOWN:
                    break;
                case LIBINPUT_EVENT_TOUCH_MOTION:
                    status=0;
                    break;
                case LIBINPUT_EVENT_TOUCH_UP:
                    break;
                default:
                    break;
            }
        }
        libinput_event_destroy(pEvent);
        libinput_dispatch(libinput);
    }
    libinput_unref(libinput);
    udev_unref(udev);
}


/*
void process_input()
{
    // input part
    sys.mouse_x=0;
    sys.mouse_y=0;
    int mouse_x=0;
    int mouse_y=0;
    candy_sys_t candy;
    double x,y;
    double x_total=0,y_total=0;
    candy.r=0x00;
    candy.g=0x00;
    candy.b=0x00;
    width=10,height=20;

    struct udev *udev;
    struct libinput *libinput;
    void *userdata=NULL;
    struct pollfd fds;
    struct sigaction action;

    struct libinput_event *ev;
    struct libinput_event_pointer *p=NULL;
    struct libinput_event_touch *t=NULL;
    struct libinput_event_keyboard *k=NULL;
    enum libinput_key_state state;
    uint32_t key;
    const char *keyname;

    udev=udev_new();
    libinput=libinput_udev_create_context(&interface, userdata, udev);
    if(libinput_udev_assign_seat(libinput,"seat0"))
    {
        libinput_unref(libinput);
        udev_unref(udev);
        exit(1);
    }

    fds.fd=libinput_get_fd(libinput);
    fds.events=POLLIN;
    fds.revents=0;
    memset(&action,0,sizeof(action));
    action.sa_sigaction=sighandler;
    action.sa_flags=SA_SIGINFO;
    sigaction(SIGINT,&action,NULL);

    while(!stop&&poll(&fds,1,-1)>-1)
    {
        mouse_x=mouse_x>0?mouse_x:0;
        mouse_x=mouse_x<1920-width?mouse_x:1920-width;
        mouse_y=mouse_y>0?mouse_y:0;
        mouse_y=mouse_y<1080-height-2?mouse_y:1080-height-2;

        candy_curse(mouse_y,mouse_x,mouse_y+height,mouse_x+width,&candy);
        // spinach_draw_curse(fd);

        libinput_dispatch(libinput);
        while((ev=libinput_get_event(libinput)))
        {
            switch(libinput_event_get_type(ev))
            {
                case LIBINPUT_EVENT_NONE:
                    abort();
                case LIBINPUT_EVENT_DEVICE_ADDED:
                    printf("Add device\n");
                    break;
                case LIBINPUT_EVENT_DEVICE_REMOVED:
                    printf("Remove device\n");
                    break;
                case LIBINPUT_EVENT_KEYBOARD_KEY:
                    k=libinput_event_get_keyboard_event(ev);
                    state = libinput_event_keyboard_get_key_state(k);
                    key = libinput_event_keyboard_get_key(k);
                    keyname = libevdev_event_code_get_name(EV_KEY, key);
                    printf("%s %s\n",keyname,state==LIBINPUT_KEY_STATE_PRESSED ? "pressed" : "released");
                    if(key==KEY_Q&&state==LIBINPUT_KEY_STATE_RELEASED) stop=1;
                    break;
                case LIBINPUT_EVENT_POINTER_MOTION:
                    //printf("New motion\n");
                    p=libinput_event_get_pointer_event(ev);
                    x=libinput_event_pointer_get_dx(p);
                    y=libinput_event_pointer_get_dy(p);
                    //printf("%6.2f/%6.2f\n", x, y);
                    x_total+=x;
                    y_total+=y;
                    x_total=(int)(x_total>0?x_total:0);
                    x_total=(int)(x_total<1920?x_total:1920);
                    y_total=(int)(y_total>0?y_total:0);
                    y_total=(int)(y_total<1080?y_total:1080);
                    mouse_x=x_total;
                    mouse_y=y_total;
                    //printf("%6.2f\n",x_total);
                    break;
                case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
                    p=libinput_event_get_pointer_event(ev);
                    x = libinput_event_pointer_get_absolute_x_transformed(p,1920);
                    y = libinput_event_pointer_get_absolute_y_transformed(p,1080);
                    //printf("%6.2f/%6.2f\n", x, y);
                    break;
                case LIBINPUT_EVENT_TOUCH_DOWN:
                    //printf("New touch down\n");
                    t = libinput_event_get_touch_event(ev);
                    x = libinput_event_touch_get_x_transformed(t,1920);
                    y = libinput_event_touch_get_y_transformed(t,1080);
                    double xmm = libinput_event_touch_get_x(t);
                    double ymm = libinput_event_touch_get_y(t);
                    printf("%d (%d) %5.2f/%5.2f (%5.2f/%5.2fmm)\n", libinput_event_touch_get_slot(t), libinput_event_touch_get_seat_slot(t), x, y, xmm, ymm);
                    mouse_x=x;
                    mouse_y=y;
                    break;
                case LIBINPUT_EVENT_TOUCH_MOTION:
                    //printf("New touch motion\n");
                    break;
                case LIBINPUT_EVENT_TOUCH_UP:
                    printf("New touch up\n");
                    break;
                default:
                    break;
            }
        }
        libinput_event_destroy(ev);
        libinput_dispatch(libinput);
    }
    udev_unref(udev);
}
*/

#endif

