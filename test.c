
#include "wchar.h"
#include "string.h"
#include "spinach_lib.h"
#include "candy_lib.h"
#include "picture_lib.h"
#include "font_lib.h"
#include "juice_lib.h"


int main(int argc, char **argv)
{
    int fd;
    const char* card = "/dev/dri/card0";
    const char* picture =
 //       "/home/fiks/Pictures/壁纸/saber2.jpg";
        "/home/fiks/Pictures/壁纸/御坂美琴.jpg";
    sys.card=card;
    sys.list=NULL;
    if(spinach_open(&fd,card)==-1) exit(1);
    spinach_prepare(fd);
    spinach_modesetting(fd);
    spinach_draw_background(fd);
    //---------- user function ----------
    // candy canvas initalize
    spinach_buf_t * buf;
    candy_canvas_t *pRootCanvas,*pTempCanvas;
    buf=&sys.list->bufs[sys.list->front_buf==1?0:1];
    pRootCanvas=InitRootCanvas(0,0,1920,1080,(uint32_t*)&buf->map[0]);
    // candy canvas print picture test
    pTempCanvas=print_picture(picture);
    UpdateCanvas(pRootCanvas, pTempCanvas);
    FreeCanvas(pTempCanvas);
    spinach_draw_flip(fd);
    // candy canvas test
    pTempCanvas=RequestCanvas(0,0,200,200);
    UpdateCanvas(pRootCanvas, pTempCanvas);
    FreeCanvas(pTempCanvas);
    spinach_draw_flip(fd);
    // candy canvas print str test
    pTempCanvas=RequestCanvas(250,0,200,200);
    print_str(pTempCanvas,L"abcdefghijklmnopqrstuvwxyz中文",36);
    UpdateCanvas(pRootCanvas, pTempCanvas);
    FreeCanvas(pTempCanvas);
    spinach_draw_flip(fd);
    // clean canvas
    FreeRootCanvas(pRootCanvas);

    process_input();
    //---------- end ----------
    spinach_cleanup(fd);
    close(fd);
    exit(0);
}



