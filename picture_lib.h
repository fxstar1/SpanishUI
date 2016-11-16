
#ifndef PICTURE_LIB_H
#define PICTURE_LIB_H

#include <jpeglib.h>
#include <jerror.h>

candy_canvas_t *print_picture(const char *pFileName)
{
    int i,j;
    FILE *file;
    uint8_t *pDescItem=NULL;
    uint8_t *pSrcItem=NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    candy_canvas_t *pCanvas;
    if((file=fopen(pFileName, "rb+"))!=NULL)
    {
        cinfo.err=jpeg_std_error(&jerr);
        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo,file);
        jpeg_read_header(&cinfo,TRUE);
        jpeg_start_decompress(&cinfo);
        pDescItem=(uint8_t *)malloc(cinfo.output_width*cinfo.output_height*cinfo.output_components*sizeof(uint8_t));
        pSrcItem=(uint8_t *)malloc(cinfo.output_width*cinfo.output_components*sizeof(uint8_t));
        if(pDescItem!=NULL && pSrcItem!=NULL)
        {
            memset(pDescItem,0,cinfo.output_width*cinfo.output_height*cinfo.output_components*sizeof(uint8_t));
            memset(pSrcItem,0,cinfo.output_width*cinfo.output_components*sizeof(uint8_t));
            while(cinfo.output_scanline < cinfo.output_height)
            {
                jpeg_read_scanlines(&cinfo,&pSrcItem,1);
                memcpy(pDescItem+(cinfo.output_scanline-1)*cinfo.output_width*cinfo.output_components*sizeof(uint8_t),
                        pSrcItem,cinfo.output_width*cinfo.output_components*sizeof(uint8_t));
            }
        }
        pCanvas=RequestCanvas(0,0,cinfo.output_width,cinfo.output_height);
        if(pCanvas!=NULL && pDescItem!=NULL && cinfo.output_components==3)
        {
            for(i=0;i<pCanvas->Width;i++)
                for(j=0;j<pCanvas->Height;j++)
                {
                    pCanvas->pBitmap[i+j*pCanvas->Width]=
                        ((pDescItem[(i+j*pCanvas->Width)*cinfo.output_components])<<16)+
                        ((pDescItem[(i+j*pCanvas->Width)*cinfo.output_components+1])<<8)+
                        (pDescItem[(i+j*pCanvas->Width)*cinfo.output_components+2]);
                }
        }
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        if(pDescItem!=NULL)
        {
            free(pDescItem);
            pDescItem=NULL;
        }
        if(pSrcItem!=NULL)
        {
            free(pSrcItem);
            pSrcItem=NULL;
        }
        fclose(file);
    }
    return pCanvas;
}

#endif

