
#ifndef CANDY_LIB_H
#define CANDY_LIB_H

#include "math.h"

#define clWhite 0x00FFFFFF
#define clBlack 0x00000000
#define clRed 0x00FF0000
#define clGreen 0x00FF0000
#define clBlue 0x00FF0000

// standard candy_lib structure and function

typedef struct candy_canvas
{
    int Left;
    int Top;
    int Width;
    int Height;
    uint32_t *pBitmap;
}candy_canvas_t;

candy_canvas_t *InitRootCanvas(int Left, int Top, int Width, int Height, uint32_t *pBitmap)
{
    candy_canvas_t *pItem=(candy_canvas_t *)malloc(sizeof(candy_canvas_t));
    if(pItem!=NULL)
    {
        pItem->Left=Left;
        pItem->Top=Top;
        pItem->Width=Width;
        pItem->Height=Height;
        pItem->pBitmap=pBitmap;
        return pItem;
    }
    return NULL;
}

candy_canvas_t *RequestCanvas(int Left, int Top, int Width, int Height)
{
    candy_canvas_t *pItem=(candy_canvas_t *)malloc(sizeof(candy_canvas_t));
    if(pItem!=NULL)
    {
        pItem->pBitmap=(uint32_t *)malloc(Width*Height*sizeof(uint32_t));
        if(pItem->pBitmap != NULL)
        {
            pItem->Left=Left;
            pItem->Top=Top;
            pItem->Width=Width;
            pItem->Height=Height;
            memset(pItem->pBitmap,0,Width*Height*sizeof(uint32_t));
            return pItem;
        }
        else
        {
            free(pItem);
            pItem=NULL;
        }
    }
    return NULL;
}

void FreeRootCanvas(candy_canvas_t *pItem)
{
    if(pItem!=NULL)
    {
        pItem->pBitmap=NULL;
        free(pItem);
        pItem=NULL;
    }
}

void FreeCanvas(candy_canvas_t *pItem)
{
    if(pItem!=NULL)
    {
        free(pItem->pBitmap);
        pItem->pBitmap=NULL;
        free(pItem);
        pItem=NULL;
    }
}

void UpdateCanvas(candy_canvas_t *pDestItem, candy_canvas_t *pSrcItem)
{
    int32_t i;
    int32_t Left=0,Top=0,Right=0,Bottom=0;
    if(pDestItem!=NULL||pSrcItem!=NULL)
    {
        Left=0>pSrcItem->Left?0:pSrcItem->Left;
        Top=0>pSrcItem->Top?0:pSrcItem->Top;
        Right=pDestItem->Width<(pSrcItem->Left+pSrcItem->Width)?
            pDestItem->Width:(pSrcItem->Left+pSrcItem->Width);
        Bottom=pDestItem->Height<(pSrcItem->Top+pSrcItem->Height)?
            pDestItem->Height:(pSrcItem->Top+pSrcItem->Height);
        if(Right>Left && Bottom>Top)
        {
            for(i=Top;i<Bottom;i++)
            {
                memcpy(&pDestItem->pBitmap[Left+i*pDestItem->Width],
                        &pSrcItem->pBitmap[Left-pSrcItem->Left+(i-pSrcItem->Top)*pSrcItem->Width],
                        (Right-Left)*sizeof(uint32_t));
            }
        }
    }
}

// lagency part

typedef struct _candy_sys_t
{
	uint8_t r,g,b;
    uint8_t linewidth;
}candy_sys_t;

typedef struct _candy_bitmap_t
{
    int x;
    int y;
    int width;
    int height;
    int8_t * bitmap;
}candy_bitmap_t;

void candy_pixel_at(int x,int y,candy_sys_t* p_candy);
void candy_rectangle(int x,int y,int x2,int y2,candy_sys_t* p_candy);
void candy_rectangle_fill(int x,int y,int x2,int y2,candy_sys_t* p_candy);
void candy_line(int x,int y,int x2,int y2,candy_sys_t* p_candy);
void candy_test(int fd);
void candy_color_table(int x,int y,candy_sys_t* p_candy);
void candy_button(int fd);

void candy_pixel_at(int x,int y,candy_sys_t* p_candy)
{
	uint8_t r,g,b;
    spinach_buf_t* buf;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;
    *(uint32_t*)&buf->map[x*buf->stride+y*4]=(r<<16)|(g<<8)|b;
}

void candy_rectangle(int x,int y,int x2,int y2,candy_sys_t* p_candy)
{
    int i;
	uint8_t r,g,b;
    spinach_buf_t* buf;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;
    for(i=x;i<=x2;i++)
    {
        *(uint32_t*)&buf->map[i*buf->stride+y*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[i*buf->stride+y2*4]=(r<<16)|(g<<8)|b;
    }
    for(i=y;i<y2;i++)
    {
        *(uint32_t*)&buf->map[x*buf->stride+i*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[x2*buf->stride+i*4]=(r<<16)|(g<<8)|b;
    }
}

void candy_rectangle_fill(int x,int y,int x2,int y2,candy_sys_t* p_candy)
{
    int i,j;
	uint8_t r,g,b;
    spinach_buf_t* buf;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;
    for(i=x;i<=x2;i++)
        for(j=y;j<=y2;j++)
            *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
}

void candy_line(int x,int y,int x2,int y2,candy_sys_t* p_candy)
{
    int i,j;
	uint8_t r,g,b;
    spinach_buf_t* buf;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;
    if(abs(x-x2)>=abs(y-y2))
    {
        if(x-x2>0)
        {
            for(i=x2;i<x;i++)
            {
                j=(y-y2)*(i-x2)/(x-x2)+y2;
                *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
            }
        }
        else
        {
            for(i=x;i<x2;i++)
            {
                j=(y2-y)*(i-x)/(x2-x)+y;
                *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
            }
        }
    }
    else
    {
         if(y-y2>0)
        {
            for(j=y2;j<y;j++)
            {
                i=(x-x2)*(j-y2)/(y-y2)+x2;
                *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
            }
        }
        else
        {
            for(j=y;j<y2;j++)
            {
                i=(x2-x)*(j-y)/(y2-y)+x;
                *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
            }
        }
    }
}

void candy_circle(int x,int y,int radius,candy_sys_t* p_candy)
{
    int i,j,count;
	uint8_t r,g,b;
    float y_temp;
    spinach_buf_t* buf;
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    count=round(radius/sqrt(2));
    for(i=0;i<=count;i++)
    {
        y_temp=sqrt(radius*radius-i*i);
        j=round(y_temp);
        *(uint32_t*)&buf->map[(x+i)*buf->stride+(y+j)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x-i)*buf->stride+(y+j)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x+i)*buf->stride+(y-j)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x-i)*buf->stride+(y-j)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x+j)*buf->stride+(y+i)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x-j)*buf->stride+(y+i)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x+j)*buf->stride+(y-i)*4]=(r<<16)|(g<<8)|b;
        *(uint32_t*)&buf->map[(x-j)*buf->stride+(y-i)*4]=(r<<16)|(g<<8)|b;
    }
}

void candy_curse(int x,int y,int x2,int y2,candy_sys_t* p_candy)
{
    int i,j;
	uint8_t r,g,b;
    spinach_buf_t* buf,* buf_temp;
    // buf=&sys.list->bufs[sys.list->mouse_front_buf==3?2:3];
    buf=&sys.list->bufs[2];
    buf_temp=&sys.list->bufs[sys.list->front_buf];
    r=p_candy->r;
    g=p_candy->g;
    b=p_candy->b;

    for(i=sys.mouse_x;i<=x2-x+sys.mouse_x;i++)
        for(j=sys.mouse_y;j<=y2-y+sys.mouse_y;j++)
        {
            *(uint32_t*)&buf->map[i*buf->stride+j*4]=*(uint32_t*)&buf_temp->map[i*buf->stride+j*4];
        }

    for(i=x;i<=x2;i++)
        for(j=y;j<=y2;j++)
        {
            *(uint32_t*)&buf->map[i*buf->stride+j*4]=(r<<16)|(g<<8)|b;
        }
    sys.mouse_x=x;
    sys.mouse_y=y;
}

void candy_copy_bitmap(candy_bitmap_t * p_bitmap)
{
    int i;
    spinach_buf_t * buf;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    for(i=0;i<p_bitmap->height;i++)
        memcpy(&buf->map[(p_bitmap->x+i)*buf->stride+p_bitmap->y*4],(p_bitmap->bitmap+p_bitmap->width*i*4),p_bitmap->width*4);
}

void candy_test(int fd)
{
    candy_sys_t candy;
    int i,j;
    candy.r=0xFF;
    candy.g=0x00;
    candy.b=0x00;
    // pixel test
    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
            candy_pixel_at(0+i,100+j,&candy);
    spinach_draw_flip(fd);
    // rectangle test
    candy_rectangle_fill(100,100,200,300,&candy);
    candy_rectangle(200,100,300,300,&candy);
    candy_color_table(300,100,&candy);
    spinach_draw_flip(fd);
    // line test
    candy.r=0x00;
    candy.g=0xFF;
    candy.b=0x00;
    for(i=0;i<=5;i++)
    {
        candy_line(500,500,500-i*20,400,&candy);
        candy_line(500,500,500-i*20,600,&candy);
        candy_line(500,500,500+i*20,400,&candy);
        candy_line(500,500,500+i*20,600,&candy);
        candy_line(500,500,400,500-i*20,&candy);
        candy_line(500,500,600,500-i*20,&candy);
        candy_line(500,500,400,500+i*20,&candy);
        candy_line(500,500,600,500+i*20,&candy);
    }
    spinach_draw_flip(fd);
    // circle test
    candy.r=0x00;
    candy.g=0x00;
    candy.b=0xFF;
    candy_circle(300,500,50,&candy);
    spinach_draw_flip(fd);
}

void candy_color_table(int x,int y,candy_sys_t* p_candy)
{   
    int i,j;
	uint8_t r,g,b;
    spinach_buf_t* buf;
    r=0xFF;
    g=0xFF;
    b=0xFF;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    for(i=0;i<256;i++)
    {
        for(j=0;j<256;j++)
        {
            r=i;
            g=j;
            b=0;
            *(uint32_t*)&buf->map[(x+i)*buf->stride+(y+j)*4]=(r<<16)|(g<<8)|b;
        }
    }
}

void candy_button(int fd)
{
    candy_sys_t candy;
    int row=100,column=100;
    int width=200,height=100;
    candy.r=0x00;
    candy.g=0x00;
    candy.b=0x00;
    candy_rectangle(row,column,row+height,column+width,&candy);
    candy.r=0x00;
    candy.g=0x00;
    candy.b=0xFF;
    candy_rectangle_fill(row+2,column+2,row+height-2,column+(width-2)/2,&candy);
    spinach_draw_flip(fd);
}

void beaf_center(int fd)
{
    int beaf_row=100,beaf_column=200;
    int row=0,column=0;
    int height=64,width=600;
    candy_sys_t candy;
    // background
    candy.r=0xAA;candy.g=0xAA;candy.b=0xAA;
    row=0;column=0;height=64;width=64*10;
    candy_rectangle_fill(beaf_row,beaf_column,beaf_row+height,beaf_column+width,&candy);
    // logo
    candy.r=0xFF;candy.g=0xFF;candy.b=0xFF;
    row=beaf_row+8;column=beaf_column+8;height=48;width=48;
    candy_rectangle_fill(row,column,row+height,column+width,&candy);
    // input div
    candy.r=0xFF;candy.g=0xFF;candy.b=0xFF;
    row=beaf_row+8;column=beaf_column+64;height=48;width=64*8;
    candy_rectangle_fill(row,column,row+height,column+width,&candy);
    candy.r=0x00;candy.g=0x00;candy.b=0x00;
    candy_rectangle(row,column,row+height,column+width,&candy);
    candy_rectangle(row,column+width-60,row+height,column+width,&candy);
    // other option
    candy.r=0xFF;candy.g=0xFF;candy.b=0xFF;
    row=beaf_row+8;column=beaf_column+64*9+8;height=48;width=48;
    candy_rectangle_fill(row,column,row+height,column+width,&candy);
    spinach_draw_flip(fd);
}

#endif

