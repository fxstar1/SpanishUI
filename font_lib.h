
#ifndef FONT_LIB_H
#define FONT_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include "candy_lib.h"

#include <ft2build.h>
#include FT_FREETYPE_H

void print_str(candy_canvas_t *pCanvas, wchar_t *pStr,int FontSize)
{
    int32_t i,j,count=0;
    int32_t Left=0,Top=0;
    FT_Library library;
    FT_Face face;
    FT_UInt glyph_index;
    FT_GlyphSlot slot;
    FT_Init_FreeType (&library); 
    FT_New_Face(library,"/usr/share/fonts/wenquanyi/wqy-microhei/wqy-microhei.ttc",0,&face); 
    FT_Set_Pixel_Sizes(face,0,FontSize);
    if(pCanvas!=NULL && pCanvas->pBitmap!=NULL)
    {
        while(count<wcslen(pStr))
        {
            glyph_index=FT_Get_Char_Index(face,pStr[count]);
            FT_Load_Glyph(face,glyph_index,FT_LOAD_DEFAULT);
            slot=face->glyph;
            FT_Render_Glyph(slot,FT_RENDER_MODE_NORMAL);
            if(Left+slot->bitmap.width < pCanvas->Width && 
                    Top+slot->bitmap.rows < pCanvas->Height)
            {
                for(i=0;i<slot->bitmap.width;i++)
                    for(j=0;j<slot->bitmap.rows;j++)
                    {
                        pCanvas->pBitmap[(Left+i)+(Top+j+FontSize-slot->bitmap_top-1)*pCanvas->Width]=
                            ((slot->bitmap.buffer[i+j*slot->bitmap.width])<<16)+
                            ((slot->bitmap.buffer[i+j*slot->bitmap.width])<<8)+
                            (slot->bitmap.buffer[i+j*slot->bitmap.width]);
                    }
                Left+=slot->bitmap.width;
            }
            else if(slot->bitmap.width < pCanvas->Width &&
                    Top+FontSize+slot->bitmap.rows < pCanvas->Height)
            {
                Left=0;
                Top=Top+FontSize;
                for(i=0;i<slot->bitmap.width;i++)
                    for(j=0;j<slot->bitmap.rows;j++)
                    {
                        pCanvas->pBitmap[(Left+i)+(Top+j+FontSize-slot->bitmap_top-1)*pCanvas->Width]=
                            ((slot->bitmap.buffer[i+j*slot->bitmap.width])<<16)+
                            ((slot->bitmap.buffer[i+j*slot->bitmap.width])<<8)+
                            (slot->bitmap.buffer[i+j*slot->bitmap.width]);
                    }
                Left+=slot->bitmap.width;
            }
            else
                break;
            count++;
        }
    }
    FT_Done_Face(face); 
    FT_Done_FreeType(library);
}

#endif

