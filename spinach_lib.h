
#ifndef SPINACH_LIB_H
#define SPINACH_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

typedef struct _spinach_buf_t
{
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t size;
	uint32_t handle;
	uint8_t *map;
	uint32_t fb;
}spinach_buf_t;
typedef struct _spinach_dev_t
{
	uint32_t conn;
    struct _spinach_dev_t* next;
	drmModeModeInfo mode;
	int front_buf;
	int mouse_front_buf;
	spinach_buf_t bufs[4];
    int32_t crtc;
	drmModeCrtc *saved_crtc;
}spinach_dev_t;
typedef struct _spinach_sys_t
{
    const char* card;
    spinach_dev_t* list;
    int status;
    int mouse_x;
    int mouse_y;
}spinach_sys_t;

static int spinach_open(int *fd,const char *card);
static int spinach_prepare(int fd);
static int spinach_setup_dev(int fd,drmModeRes *p_res,drmModeConnector *p_conn,spinach_dev_t *p_dev);
static int spinach_find_crtc(int fd,drmModeRes *p_res,drmModeConnector *p_conn,spinach_dev_t *p_dev);
static int spinach_create_fb(int fd,spinach_buf_t *p_buf);
static void spinach_modesetting(int fd);
static void spinach_cleanup(int fd);
static void spinach_destroy_fb(int fd,spinach_buf_t *p_buf);
static void spinach_draw_background(int fd);
static void spinach_draw_flip(int fd);
static void spinach_draw_curse(int fd);

static spinach_sys_t sys;

static int spinach_open(int *fd,const char *card)
{
    uint64_t value;
    // open device
    *fd=open(card,O_RDWR|O_CLOEXEC);
    if(*fd==-1)
    {
        printf("cannot open %s\n",card);
        return -1;
    }
    // check dumb buffer
	if(drmGetCap(*fd,DRM_CAP_DUMB_BUFFER,&value)==-1||!value)
    {
        printf("not support dumb buffers\n");
		return -1;
	}
    else
    {
        printf("DRM_CAP_DUMB_BUFFER value:%d\n",(int)value);
    }
	return 0;
}

static int spinach_prepare(int fd)
{
    int i;
    drmModeRes *p_res;
	drmModeConnector *p_conn;
    p_res=drmModeGetResources(fd);
    spinach_dev_t *p_dev;
    if(p_res==NULL)
    {
        printf("* cannot retrieve DRM resources\n");
        return -1;
    }
    // setup dev for connectors
	for (i=0;i<p_res->count_connectors;i++)
    {
        printf("--------------------\n");
		p_conn=drmModeGetConnector(fd,p_res->connectors[i]);
        printf("connector %d: %d\n",i,p_res->connectors[i]);
        if(p_conn==NULL)
        {
		    printf("* cannot retrieve DRM connector %d:%d\n",i,p_res->connectors[i]);
            continue;
        }
        p_dev=malloc(sizeof(*p_dev));
        memset(p_dev,0,sizeof(*p_dev));
        p_dev->conn=p_conn->connector_id;
        // setup dev
        if(spinach_setup_dev(fd,p_res,p_conn,p_dev)==-1)
        {
		    printf("* cannot setup device for DRM connector %d:%d\n",i,p_res->connectors[i]);
            free(p_dev);
            drmModeFreeConnector(p_conn);
            continue;
        }
        drmModeFreeConnector(p_conn);
        // save dev to sys.list
        p_dev->next=sys.list;
        sys.list=p_dev;
    }
    printf("--------------------\n");
    return 0;
}

static int spinach_setup_dev(int fd,drmModeRes *p_res,drmModeConnector *p_conn,spinach_dev_t *p_dev)
{
	if(p_conn->connection!=DRM_MODE_CONNECTED)
    {
        printf("** ignoring unused connector %d\n",p_conn->connector_id);
        return -1;
    }
	if(p_conn->count_modes==0)
    {
        printf("** no valid mode for connector %d\n",p_conn->connector_id);
        return -1;
    }
    // copy mode information
    memcpy(&p_dev->mode,&p_conn->modes[0],sizeof(p_dev->mode));
	p_dev->bufs[0].width=p_conn->modes[0].hdisplay;
	p_dev->bufs[0].height=p_conn->modes[0].vdisplay;
	p_dev->bufs[1].width=p_conn->modes[0].hdisplay;
	p_dev->bufs[1].height=p_conn->modes[0].vdisplay;
	p_dev->bufs[2].width=p_conn->modes[0].hdisplay;
	p_dev->bufs[2].height=p_conn->modes[0].vdisplay;
	p_dev->bufs[3].width=p_conn->modes[0].hdisplay;
	p_dev->bufs[3].height=p_conn->modes[0].vdisplay;
	printf("mode for connector %d is %dx%d\n",p_conn->connector_id,p_dev->bufs[0].width,p_dev->bufs[0].height);
    // find crtc
    if(spinach_find_crtc(fd,p_res,p_conn,p_dev)==-1)
    {
        printf("** no valid crtc for connector %d\n",p_conn->connector_id);
        return -1;
    }
    // create framebuffer #1
    if(spinach_create_fb(fd,&p_dev->bufs[0])==-1)
    {
        printf("** cannot create framebuffer #1 for connector %d\n",p_conn->connector_id);
        return -1;
    }
    // create framebuffer #2
    if(spinach_create_fb(fd,&p_dev->bufs[1])==-1)
    {
        printf("** cannot create framebuffer #2 for connector %d\n",p_conn->connector_id);
        return -1;
    }
    // create framebuffer #3
    if(spinach_create_fb(fd,&p_dev->bufs[2])==-1)
    {
        printf("** cannot create framebuffer #3 for connector %d\n",p_conn->connector_id);
        return -1;
    }
    if(spinach_create_fb(fd,&p_dev->bufs[3])==-1)
    {
        printf("** cannot create framebuffer #4 for connector %d\n",p_conn->connector_id);
        return -1;
    }
    return 0;
}

static int spinach_find_crtc(int fd,drmModeRes *p_res,drmModeConnector *p_conn,spinach_dev_t *p_dev)
{
	drmModeEncoder *p_enc;
	int32_t crtc;
    int i,j;
    spinach_dev_t *iter;

    // check crtcs in resource
    printf("*** res crtcs list (%d)\n",p_res->count_crtcs);
    for(i=0;i<p_res->count_crtcs;i++)
    {
        printf("*** crtc: %d\n",p_res->crtcs[i]);
    }
    // check encoders in connector
    printf("*** encoders list (%d)\n",p_conn->count_encoders);
    for(i=0;i<p_conn->count_encoders;i++)
    {
        printf("*** encoder: %d\n",p_conn->encoders[i]);
    }
	if(p_conn->encoder_id!=0)
    {
        p_enc=drmModeGetEncoder(fd,p_conn->encoder_id);
        printf("*** encoder_id: %d,crtc_id: %d\n",p_conn->encoder_id,p_enc->crtc_id);
        // check if the crtc has been used
        if(p_enc->crtc_id)
        {
            crtc=p_enc->crtc_id;
			for(iter=sys.list;iter;iter=iter->next)
            {
				if(iter->crtc==crtc)
                {
					crtc=-1;
					break;
				}
			}
            // bound connector to encoder
            if(crtc>=0)
            {
				drmModeFreeEncoder(p_enc);
				p_dev->crtc=crtc;
				return 0;
			}
        }
        drmModeFreeEncoder(p_enc);
    }
    else p_enc=NULL;
    // iterate all other available encoders to find a matching CRTC in case the connector is not bounded to encoder
    for(i=0;i<p_conn->count_encoders;i++)
    {
        p_enc=drmModeGetEncoder(fd,p_conn->encoders[i]);
        printf("*** encoder %d: %d\n",i,p_conn->encoders[i]);
        if(p_enc==NULL)
        {
            printf("*** cannot retrieve encoder %d:%d\n",i,p_conn->encoders[i]);
            continue;
        }
        for(j=0;j<p_res->count_crtcs;j++)
        {
            if(!(p_enc->possible_crtcs&(1<<j)))
				continue;
            crtc=p_res->crtcs[j];
            for(iter=sys.list;iter;iter=iter->next)
            {
                if(iter->crtc==crtc)
                {
                    crtc=-1;
                    break;
                }
            }
            if(crtc>=0)
            {
                drmModeFreeEncoder(p_enc);
                p_dev->crtc=crtc;
                return 0;
            }
        }
        drmModeFreeEncoder(p_enc);
    }
    return -1;
}

static int spinach_create_fb(int fd,spinach_buf_t *p_buf)
{
    struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;

    memset(&creq,0,sizeof(creq));
	memset(&dreq,0,sizeof(dreq));
	memset(&mreq,0,sizeof(mreq));
	creq.width=p_buf->width;
	creq.height=p_buf->height;
	creq.bpp=32;
    // creat dumb buffer
	if(drmIoctl(fd,DRM_IOCTL_MODE_CREATE_DUMB,&creq)==-1)
    {
        printf("*** cannot create dumb buffer\n");
        return -1;
    }
    p_buf->stride=creq.pitch;
    p_buf->size=creq.size;
    p_buf->handle=creq.handle;
    // create framebuffer
    if(drmModeAddFB(fd,p_buf->width,p_buf->height,24,32,p_buf->stride,p_buf->handle,&p_buf->fb)==-1)
    {
        printf("*** cannot create framebuffer\n");
        drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
        return -1;
    }
	mreq.handle=p_buf->handle;
    // prepare buffer for memory mapping
    if(drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB,&mreq)==-1)
    {
        printf("*** cannot set map dumb buffer\n");
        drmModeRmFB(fd,p_buf->fb);
        dreq.handle=p_buf->handle;
        drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
        return -1;
    }
    // mapping memory
    p_buf->map=mmap(0,p_buf->size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,mreq.offset);
    if (p_buf->map==MAP_FAILED)
    {
        printf("*** cannot map dumb buffer\n");
        drmModeRmFB(fd,p_buf->fb);
        dreq.handle=p_buf->handle;
        drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
        return -1;
    }
    memset(p_buf->map,0,p_buf->size);
    return 0;
}

static void spinach_modesetting(int fd)
{
    spinach_buf_t* p_buf=NULL;
    spinach_dev_t* iter;
    for(iter=sys.list;iter;iter=iter->next)
    {
        iter->front_buf=0;
        iter->mouse_front_buf=2;
        iter->saved_crtc=drmModeGetCrtc(fd,iter->crtc);
        p_buf=&iter->bufs[iter->front_buf];
        if(drmModeSetCrtc(fd,iter->crtc,p_buf->fb,0,0,&iter->conn,1,&iter->mode)==-1)
        {
            printf("* cannot set CRTC for connetor %d\n",iter->conn);
        }
    }
}

static void spinach_cleanup(int fd)
{
	spinach_dev_t* iter;
	while(sys.list)
    {
		// remove from global list
		iter=sys.list;
		sys.list=iter->next;
		// restore saved CRTC configuration
		drmModeSetCrtc(fd,iter->saved_crtc->crtc_id,iter->saved_crtc->buffer_id,iter->saved_crtc->x,iter->saved_crtc->y,&iter->conn,1,&iter->saved_crtc->mode);
		drmModeFreeCrtc(iter->saved_crtc);
		// destroy framebuffers
		spinach_destroy_fb(fd, &iter->bufs[0]);
		spinach_destroy_fb(fd, &iter->bufs[1]);
		spinach_destroy_fb(fd, &iter->bufs[2]);
		spinach_destroy_fb(fd, &iter->bufs[3]);
		// free allocated memory
		free(iter);
	}
}

static void spinach_destroy_fb(int fd,spinach_buf_t *p_buf)
{
	struct drm_mode_destroy_dumb dreq;
	// unmap buffer
	munmap(p_buf->map,p_buf->size);
	// delete framebuffer
	drmModeRmFB(fd,p_buf->fb);
	// delete dumb buffer
	memset(&dreq,0,sizeof(dreq));
	dreq.handle=p_buf->handle;
	drmIoctl(fd,DRM_IOCTL_MODE_DESTROY_DUMB,&dreq);
}

static void spinach_draw_background(int fd)
{   
    int i,j;
	uint8_t r,g,b;
    spinach_dev_t* iter;
    spinach_buf_t* buf;
    r=0xFF;
    g=0xFF;
    b=0xFF;
    for(iter=sys.list;iter;iter=iter->next)
    {
        buf=&iter->bufs[iter->front_buf==1?0:1];
        for(i=0;i<buf->height;i++)
        {
            for(j=0;j<buf->width;j++)
            {
                *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
            }
        }
    }
    spinach_draw_flip(fd);
}

static void spinach_draw_flip(int fd)
{
    spinach_buf_t* buf,* buf_temp;
    spinach_dev_t* iter;
    iter=sys.list;
    buf_temp=&iter->bufs[iter->front_buf];
    iter->front_buf=iter->front_buf==1?0:1;
    buf=&iter->bufs[iter->front_buf];
    if(drmModeSetCrtc(fd,iter->crtc,buf->fb,0,0,&iter->conn,1,&iter->mode)==-1)
        printf("cannot flip CRTC for connector %d\n",iter->conn);
    else
        iter->front_buf=iter->front_buf==1?0:1;
    memcpy(buf_temp->map,buf->map,buf->size);
    buf_temp=&iter->bufs[iter->mouse_front_buf];
    memcpy(buf_temp->map,buf->map,buf->size);
    buf_temp=&iter->bufs[iter->mouse_front_buf==3?2:3];
    memcpy(buf_temp->map,buf->map,buf->size);
    spinach_draw_curse(fd);
}

/*
static void _spinach_draw_curse(int fd)
{
    spinach_buf_t* buf,* buf_temp;
    spinach_dev_t* iter;
    iter=sys.list;
    buf=&iter->bufs[iter->mouse_front_buf==3?2:3];
    if(drmModeSetCrtc(fd,iter->crtc,buf->fb,0,0,&iter->conn,1,&iter->mode)==-1)
        printf("cannot flip CRTC for connector %d\n",iter->conn);
    else
        iter->mouse_front_buf=iter->mouse_front_buf==3?2:3;
    buf=&iter->bufs[iter->front_buf];
    buf_temp=&iter->bufs[iter->mouse_front_buf==3?2:3];
    memcpy(buf_temp->map,buf->map,buf->size);
}
*/

static void spinach_draw_curse(int fd)
{
    spinach_buf_t* buf;
    spinach_dev_t* iter;
    iter=sys.list;
    buf=&iter->bufs[2];
    if(drmModeSetCrtc(fd,iter->crtc,buf->fb,0,0,&iter->conn,1,&iter->mode)==-1)
        printf("cannot flip CRTC for connector %d\n",iter->conn);
}
#endif

