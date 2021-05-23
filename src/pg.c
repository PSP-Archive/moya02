// primitive graphics for PSP
// now not only graphics :)

#include "pg.h"
#include "syscall.h"
#include "_clib.h"

#include "font.c"

#ifdef WIN32
#include "win32_psp.h"
#else

//variables
unsigned char *pg_vramtop=(char *)0x04000000;
#endif

long pg_screenmode;
long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

char pg_mypath[MAX_PATH];
char pg_workdir[MAX_PATH];




void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


unsigned char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return
	(
pg_vramtop
+(pg_drawframe?FRAMESIZE:0)
+x*(PIXELSIZE*2)
+y*(LINESIZE*2)
+0x40000000
	);
}


void pgInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);
}


void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x<<3,y<<3,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x<<4,y<<4,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x<<5,y<<5,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}


void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}
}

/*no clip version*/
void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

/*clipped version*/
void pgBitBlt_clip(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d, char clip)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	unsigned long endx, startx, endy, starty;
	// Should always provide screen clipping?!
	starty = 0;
	startx = 0;
	endx = w;
	endy = h;

	if(clip)
	{
		if(x < 0) startx = 0;
		if(y < 0) starty = 0;
		if(w > SCREEN_WIDTH) endx = SCREEN_WIDTH;
		if(y > SCREEN_HEIGHT) endy = SCREEN_HEIGHT;
	}

	vptr0=pgGetVramAddr(x,y);
	for (yy=starty; yy<endy; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=startx; xx<endx; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}


void pgMoverect(unsigned long x, unsigned long y, long w, long h, unsigned long tx, unsigned long ty)
{
	unsigned char *vptr_src, *vptr_dest;
	long xx,yy,dx,dy,ofs,end;

	if (h>=0) {
		dy=LINESIZE*2;
	} else {
		dy=-LINESIZE*2;
		h=-h;
	}

	vptr_src=pgGetVramAddr(x,y);
	vptr_dest=pgGetVramAddr(tx,ty);

	if (w>=0) {
		for (yy=h; yy>0; --yy) {
			end=PIXELSIZE*2*w;
			for (ofs=0; ofs!=end; ofs+=PIXELSIZE*2) {
				*(unsigned short *)(vptr_dest+ofs)=*(unsigned short *)(vptr_src+ofs);
			}
			vptr_src+=dy;
			vptr_dest+=dy;
		}
	} else {
		for (yy=h; yy>0; --yy) {
			end=PIXELSIZE*2*w;
			for (ofs=0; ofs!=end; ofs-=PIXELSIZE*2) {
				*(unsigned short *)(vptr_dest+ofs)=*(unsigned short *)(vptr_src+ofs);
			}
			vptr_src+=dy;
			vptr_dest+=dy;
		}
	}
}


void pgFillrect(unsigned long x, unsigned long y, long w, long h, unsigned long color)
{
	unsigned char *vptr0;
	unsigned char *vptr;
	long xx,yy,dx,dy;
	unsigned long i;

	if (w<0) w=-w;
	if (h<0) h=-h;
	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		vptr=vptr0;
		for (xx=0; xx<w; xx++) {
			*(unsigned short *)vptr=color;
			vptr+=PIXELSIZE*2;
		}
		vptr0+=LINESIZE*2;
	}
}




void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

//	if (ch>255) return;	// ???(uchar)by safety
	cfont=font+(((long)(ch))<<3);
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}


void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=(pg_showframe?0:1);
	pg_drawframe=(pg_drawframe?0:1);
	sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}


/******************************************************************************/


void pgcLocate(unsigned long x, unsigned long y)
{
	if (x>=CMAX_X) x=0;
	if (y>=CMAX_Y) y=0;
	pgc_csr_x[pg_drawframe?1:0]=x;
	pgc_csr_y[pg_drawframe?1:0]=y;
}


void pgcColor(unsigned long fg, unsigned long bg)
{
	pgc_fgcolor[pg_drawframe?1:0]=fg;
	pgc_bgcolor[pg_drawframe?1:0]=bg;
}


void pgcDraw(char drawfg, char drawbg)
{
	pgc_fgdraw[pg_drawframe?1:0]=drawfg;
	pgc_bgdraw[pg_drawframe?1:0]=drawbg;
}


void pgcSetmag(char mag)
{
	pgc_mag[pg_drawframe?1:0]=mag;
}


void pgcPutchar(const char ch)
{
	if (ch==0x0d) {
		pgc_csr_x[pg_drawframe]=0;
		return;
	}
	if (ch==0x0a) {
		if ((++pgc_csr_y[pg_drawframe])>=CMAX_Y) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-1;
			pgMoverect(0,8,SCREEN_WIDTH,SCREEN_HEIGHT-8,0,0);
		}
		return;
	}
	pgcPutchar_nocontrol(ch);
}


void pgcPutchar_nocontrol(const char ch)
{
	pgPutChar(pgc_csr_x[pg_drawframe]*8, pgc_csr_y[pg_drawframe]*8, pgc_fgcolor[pg_drawframe], pgc_bgcolor[pg_drawframe], ch, pgc_fgdraw[pg_drawframe], pgc_bgdraw[pg_drawframe], pgc_mag[pg_drawframe]);
	pgc_csr_x[pg_drawframe]+=pgc_mag[pg_drawframe];
	if (pgc_csr_x[pg_drawframe]>CMAX_X-pgc_mag[pg_drawframe]) {
		pgc_csr_x[pg_drawframe]=0;
		pgc_csr_y[pg_drawframe]+=pgc_mag[pg_drawframe];
		if (pgc_csr_y[pg_drawframe]>CMAX_Y-pgc_mag[pg_drawframe]) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-pgc_mag[pg_drawframe];
			pgMoverect(0,pgc_mag[pg_drawframe]*8,SCREEN_WIDTH,SCREEN_HEIGHT-pgc_mag[pg_drawframe]*8,0,0);
		}
	}
}


void pgcPuts(const char *str)
{
	for (; *str!=0; ++str) pgcPutchar(*str);
}


void pgcPutsn(const char *str,unsigned long max)
{
	for (; *str!=0 && max>0; ++str, --max) pgcPutchar(*str);
}


void pgcPutsln(const char *str)
{
	pgcPuts(str);
	pgcPuts("\r\n");
}


void pgcPuthex2(const unsigned long s)
{
	char ch;
	ch=((s>>4)&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
	ch=(s&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
}


void pgcPuthex8(const unsigned long s)
{
	pgcPuthex2(s>>24);
	pgcPuthex2(s>>16);
	pgcPuthex2(s>>8);
	pgcPuthex2(s);
}


void pgcCls()
{
	pgFillvram(pgc_bgcolor[pg_drawframe]);
	pgcLocate(0,0);
}


void pgcClearlines(unsigned long st, unsigned long ed)
{
	if (st>=CMAX_Y) return;
	if (ed>=CMAX_Y) ed=CMAX_Y-1;
	if (st>ed) return;
	pgFillrect(0,st*8,SCREEN_WIDTH,(ed-st)*8+8,pgc_bgcolor[pg_drawframe]);
}


void pgcCleareol()
{
	pgFillrect(pgc_csr_x[pg_drawframe]*8,pgc_csr_y[pg_drawframe]*8,SCREEN_WIDTH-pgc_csr_x[pg_drawframe]*8,8,pgc_bgcolor[pg_drawframe]);
}



/******************************************************************************/



void pgiInit()
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
}


unsigned long pgiGetpad()
{
	ctrl_data_t paddata;
	sceCtrlReadBufferPositive(&paddata,1);
	return paddata.buttons;
}


unsigned long pgiGetTick()
{
	ctrl_data_t paddata;
	sceCtrlReadBufferPositive(&paddata,1);
	return paddata.frame;
}



void pgiGetCtrl(pgi_ctrl_t *d)
{
	ctrl_data_t paddata;
	sceCtrlReadBufferPositive(&paddata,1);
	d->tick=paddata.frame;
	d->pad=paddata.buttons;
	d->analog=*(unsigned long *)paddata.analog;
}



/******************************************************************************/



#define PGA_CHANNELS 3
#define PGA_SAMPLES 256
#define MAXVOLUME 0x8000

int pga_ready=0;
int pga_handle[PGA_CHANNELS];

short pga_sndbuf[PGA_CHANNELS][2][PGA_SAMPLES][2];


void (*pga_channel_callback[PGA_CHANNELS])(void *buf, unsigned long reqn);

int pga_threadhandle[PGA_CHANNELS];


volatile int pga_terminate=0;


static int pga_channel_thread(int args, void *argp)
{
	volatile int bufidx=0;
	int channel=*(int *)argp;

	while (pga_terminate==0) {
		void *bufptr=&pga_sndbuf[channel][bufidx];
		void (*callback)(void *buf, unsigned long reqn);
		callback=pga_channel_callback[channel];
		if (callback) {
			callback(bufptr,PGA_SAMPLES);
		} else {
			unsigned long *ptr=bufptr;
			int i;
			for (i=0; i<PGA_SAMPLES; ++i) *(ptr++)=0;
		}
		pgaOutBlocking(channel,0x8000,0x8000,bufptr);
		bufidx=(bufidx?0:1);
	}
	sceKernelExitThread(0);
	return 0;
}


void pga_channel_thread_callback(int channel, void *buf, unsigned long reqn)
{
	void (*callback)(void *buf, unsigned long reqn);
	callback=pga_channel_callback[channel];
}


void pgaSetChannelCallback(int channel, void *callback)
{
	pga_channel_callback[channel]=callback;
}



/******************************************************************************/

int pgaInit()
{
	int i,ret;
	int failed=0;
	char str[32];

	pga_terminate=0;
	pga_ready=0;

	for (i=0; i<PGA_CHANNELS; i++) {
		pga_handle[i]=-1;
		pga_threadhandle[i]=-1;
		pga_channel_callback[i]=0;
	}
	for (i=0; i<PGA_CHANNELS; i++) {
		if ((pga_handle[i]=sceAudioChReserve(-1,PGA_SAMPLES,0))<0) failed=1;
	}
	if (failed) {
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_handle[i]!=-1) sceAudioChRelease(pga_handle[i]);
			pga_handle[i]=-1;
		}
		return -1;
	}
	pga_ready=1;

	_strcpy(str,"pgasnd0");
	for (i=0; i<PGA_CHANNELS; i++) {
		str[6]='0'+i;
		pga_threadhandle[i]=sceKernelCreateThread(str,(pg_threadfunc_t)&pga_channel_thread,0x12,0x10000,0,NULL);
		if (pga_threadhandle[i]<0) {
			pga_threadhandle[i]=-1;
			failed=1;
			break;
		}
		ret=sceKernelStartThread(pga_threadhandle[i],sizeof(i),&i);
		if (ret!=0) {
			failed=1;
			break;
		}
	}
	if (failed) {
		pga_terminate=1;
		for (i=0; i<PGA_CHANNELS; i++) {
			if (pga_threadhandle[i]!=-1) {
				sceKernelWaitThreadEnd(pga_threadhandle[i],NULL);
				sceKernelDeleteThread(pga_threadhandle[i]);
			}
			pga_threadhandle[i]=-1;
		}
		pga_ready=0;
		return -1;
	}
	return 0;
}


void pgaTermPre()
{
	pga_ready=0;
	pga_terminate=1;
}


void pgaTerm()
{
	int i;
	pga_ready=0;
	pga_terminate=1;

	for (i=0; i<PGA_CHANNELS; i++) {
		if (pga_threadhandle[i]!=-1) {
			sceKernelWaitThreadEnd(pga_threadhandle[i],NULL);
			sceKernelDeleteThread( pga_threadhandle[i]);
		}
		pga_threadhandle[i]=-1;
	}

	for (i=0; i<PGA_CHANNELS; i++) {
		if (pga_handle[i]!=-1) {
			sceAudioChRelease(pga_handle[i]);
			pga_handle[i] =-1;
		}
	}
}



int pgaOutBlocking(unsigned long channel,unsigned long vol1,unsigned long vol2,void *buf)
{
	if (!pga_ready) return -1;
	if (channel>=PGA_CHANNELS) return -1;
	if (vol1>MAXVOLUME) vol1=MAXVOLUME;
	if (vol2>MAXVOLUME) vol2=MAXVOLUME;
	return sceAudioOutputPannedBlocking(pga_handle[channel],vol1,vol2,buf);
}

//バッファは64バイト境界じゃなくても大丈夫みたい
//[0]が左、[1]が右
//サンプル速度は44100
//vol1が左


//int xmain(void);

//void __exit();

volatile int pg_terminate=0;


void pgErrorHalt(const char *str)
{
	pgScreenFrame(1,0);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgcPuts(str);
	while (1) { pgWaitV(); }
}

void pgMain(unsigned long args, void *argp)
{
	int ret;
	int n;

	n=args;
	if (n>sizeof(pg_mypath)-1) n=sizeof(pg_mypath)-1;
	_memcpy(pg_mypath,argp,n);
	pg_mypath[sizeof(pg_mypath)-1]=0;
	_strcpy(pg_workdir,pg_mypath);
	for (n=_strlen(pg_workdir); n>0 && pg_workdir[n-1]!='/'; --n) pg_workdir[n-1]=0;

	sceDisplaySetMode(1,SCREEN_WIDTH,SCREEN_HEIGHT);

	pgScreenFrame(0,1);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgScreenFrame(0,0);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);

	pgiInit();

	ret=pgaInit();
	if (ret) pgErrorHalt("pga subsystem initialization failed.");

	ret=xmain();
	pgExit(ret);
}


void pgExit(int n)
{
	pg_terminate=1;

	// terminate subsystem preprocess
	pgaTermPre();

	// terminate subsystem
	pgaTerm();

//	__exit();
}



const char *pguGetMypath()
{
	return pg_mypath;
}

const char *pguGetWorkdir()
{
	return pg_workdir;
}



int pgfOpen(const char *filename, unsigned long flag)
{
	char fn[MAX_PATH*2];
	if (_strchr(filename,':')!=NULL || *filename=='/' || *filename=='\\') {
		return sceIoOpen(filename,flag);
	} else {
		_strcpy(fn,pg_workdir);
		_strcat(fn,filename);
		return sceIoOpen(fn,flag);
	}
}

void pgfClose(int fd)
{
	sceIoClose(fd);
}

int pgfRead(int fd, void *data, int size)
{
	return sceIoRead(fd,data,size);
}

int pgfWrite(int fd, void *data, int size)
{
	return sceIoWrite(fd,data,size);
}

int pgfSeek(int fd, int offset, int whence)
{
	return sceIoLseek(fd,offset,whence);
}


