// *******************************
// This is a port of a classic old voxel terrain renderer. Andrea kindly
// released his code back in 2000 so here it is again but on PSP!!!
// ����́uAndrea kindly�v���񂪂Q�O�O�O�N�ɏ����Ĕ��\�����Â��R�[�h��
// �o�r�o�p�ɍ�蒼������������I�I�I�B
// The other person that needs thanking is Nem - your hello world, sound
// and pg lib made there way here, I hope thats ok? :-)
// ���̐l�̋��͂��K�v�������B�m�d�l����Ƃ��B�ނ́u�g���������v���������v��
// ���Ƃ��o�f�̃��C�u�����Ƃ��A����ɂ����Ă��ǂ���˂��H�i�O�O�j
// The rest of it is written by Grover.. for anyone to use, and abuse.
// The windows rendering is incomplete and only a few sce functions have
// been emulated - its an ongoing thing, so it will get filled out in the
// next week or so...
// �c��͎��iGrover�j��������..�B���Ɏg���Ă���������A����ɂ��Ă���B
// ��ʕ\���̃����_�����O�́i�R�[�h�łȂ��ĉ�ʂ��j�������������B
// �r�b�d�̊֐��͂�����Ƃ����킩��Ȃ����A�|  �ł����̂����Ȃ�Ƃ��Ȃ邶���B
// ���T������ɂ́A�i�r�b�d���C�u�����̉�͂��n�b�N����āj�o���Ă�񂶂႟�Ȃ�����...
//				Grover - May 2005.�i�Q�O�O�T�N�T���j
//				dlannan@gagagames.com

// *******************************�i�uGlvox�v�������̃v���O�����̖��́j
// Credits for Original Glvox writer     �uGlvox�v�̃I���W�i���R�[�h���������l�B
//  - Andrea "6502" Griffini, programmer �uAndrea "6502" Griffini�v����B�v���O���}�[�ł��B
//    agriff@ix.netcom.com
//    http://vv.val.net/~agriffini
// *******************************

#include "syscall.h"
#include "pg.h"
#include "_clib.h"

// *******************************
// Not yet used.. yet..�܂��Ȃ����ǁA�g���Ă��
#include "Sky.c"
//#include "Water1.c"

// *******************************

#ifdef WIN32
#include "win32_psp.h"
#endif

// *******************************

#define PIXELSKIP		4
#define CLOCKS_PER_SEC	1000000

float FOV=3.141592654/4;	// half of the xy field of view�w�x�̈�̔�����������

// *******************************

#define BUFFER_WIDTH	((512/PIXELSKIP)+1)
#define BUFFER_HEIGHT	271

// *******************************
// These dont seem to hurt the ps2 compiler..�����͂d�d�̃R���p�C���ɂȂ���...�B

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

// *******************************

typedef unsigned char byte;

#define MAP_W		256
#define MAP_H		256
#define MAP_MASK	0xFFFF

// *******************************

//unsigned char app_running=1;	// ���s���t���O���P�i���s���ł񂪂ȁj
//�d�l�ύX�����ׁA�g�p�֎~�ł񂪂ȁB[HOME]�I�����Ă���B

//unsigned short *HMap=(unsigned short *)Water1Data;	// Height field�n�C�g�t�B�[���h

unsigned char HMap[MAP_W*MAP_H];						// Color map�F�}�b�v
unsigned char CMap[MAP_W*MAP_H];						// Color map�F�}�b�v
unsigned char LMap[MAP_W*MAP_H];

unsigned short *Video;
// unsigned char Video[BUFFER_WIDTH * BUFFER_HEIGHT];	// Off-screen buffer���z���

static float gl_ss, gl_sa, gl_a, gl_s;
static int gl_x0, gl_y0;

static my_height;





volatile int bSleep=0;

// -----------------------------------------------------------------------------

// �z�[���{�^���I�����ɃR�[���o�b�N
int exit_callback(void)
{
	bSleep=1;
	scePowerSetClockFrequency(222,222,111);
// Cleanup the games resources etc (if required)

// Exit game
	sceKernelExitGame();
	return 0;
}

// �d���X�C�b�`���쎞��s����ɃR�[���o�b�N�B
// ���̊֐����܂����s���ł��T�X�y���h�E�X�^���o�C�ɓ���\��������B
void power_callback(int unknown, int pwrflags)
{
	//if(pwrflags & (POWER_CB_SUSPEND|POWER_CB_STANDBY)){
	if(pwrflags & POWER_CB_POWER){
		if (!bSleep){
			bSleep=1;

			// �t�@�C���A�N�Z�X���ɃT�X�y���h�E�X�^���o�C�����
			// 0byte�̃Z�[�u�t�@�C�����ł��Ă��܂����Ƃ�����̂ŁA
			// �������ݒ��̓T�X�y���h�E�X�^���o�C�𖳌����B
			sceKernelPowerLock(0);
//			set_cpu_clock(0);
//			save_config();
//			if (rom_get_loaded() && rom_has_battery())
//				save_sram(get_sram(), rom_get_info()->ram_size);
			sceKernelPowerUnlock(0);
		}
	}
	if(pwrflags & POWER_CB_BATLOW){
		//renderer_set_msg("PSP Battery is Low!");
		if (!bSleep){
			bSleep=1;

			sceKernelPowerLock(0);
//			set_cpu_clock(0);
//			save_config();
//			if (rom_get_loaded() && rom_has_battery())
//				save_sram(get_sram(), rom_get_info()->ram_size);
			sceKernelPowerUnlock(0);

			// �����T�X�y���h�B
			// �o�b�e������10%��؂�p���[�����v���_�ł��n�߂�ƁA
			// ���삪�ɒ[�ɒx���Ȃ�t���[�Y������Z�[�u�ł��Ȃ��Ȃ����肷��B
			// �s�̃Q�[���ł�0%�܂Ŏg���Ă�悤�Ȃ̂���B
			scePowerRequestSuspend();
		}
	}
	if(pwrflags & POWER_CB_RESCOMP){
		bSleep=0;
	}
	// �R�[���o�b�N�֐��̍ēo�^
	// �i��x�Ă΂ꂽ��ēo�^���Ă����Ȃ��Ǝ��ɃR�[���o�b�N����Ȃ��j
	int cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);
}

// �|�[�����O�p�X���b�h
int CallbackThread(int args, void *argp)
{
	int cbid;

	// �R�[���o�b�N�֐��̓o�^
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);

	// �|�[�����O
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	// �|�[�����O�p�X���b�h�̐���
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);

	return thid;
}
// *******************************
// Reduces a value to 0..255 (used in height field computation)
// �k���l�O�`�Q�T�T�i�����̌v�Z�ł����Ă܂��B������j
int Clamp(int x)
{
	return (x<0 ? 0 : (x>255 ? 255 : x));
}

// *******************************�i�����v�Z���j
static void calc_light( int x1, int y1, int x2, int y2 )
{
int i, j, k, c, z ;
int sqrt_max=0;
float norm;
float t;

	for(z=0; z<256 * 256; z++){
		LMap[z]=0;
	}

//for(k=2; k>=1; k--)
	for(c=0; c<2; c++){
		for(i=y1; i<=y2; i++){
			for(j=x1; j<=x2; j++){
			int h00=HMap[ ((i-1) * MAP_W + (j-1)) & MAP_MASK ];
			int h01=HMap[ ((i-1) * MAP_W + (j+1)) & MAP_MASK ];
			int h10=HMap[ ((i+1) * MAP_W + (j-1)) & MAP_MASK ];
			int h11=HMap[ ((i+1) * MAP_W + (j+1)) & MAP_MASK ];
			int dx	= (h11 - h00);
			int dy	= (h10 - h01);
//			int d	= (dx*dx)+(dy*dy);

			int sqrt_d=sqrtu((dx*dx)+(dy*dy));

				if( c == 0 )
				{
					 if( sqrt_max<sqrt_d ) {sqrt_max=sqrt_d;}
				} else
				{
//					t=(float)sqrtu(d) * norm * 0.5f;
//					t=(float)sqrtu(d) * norm;
					t=(float)sqrt_d   * norm;

//					t -= 2.0f;
					t *= 0.5f;
					// Some evil replacements until I find a solution for the gcc c.lt.s problem
					// ������Ɖ�肭�ǂ��ςȕ��ɂȂ��Ă�̂́A�����������f�b�b�̃o�O�̂������B
					if( (*(unsigned int *)&t) & 0x80000000 ) t=0.0f;
//					if( t<0.0f ) t=0.0f;			// �{���͂����������������B
					if( ((int )t) > 255) t=255.0f;
//					if( t > 255.0f ) t=255.0f;		// �{���͂����������������B
					 LMap[ (i * MAP_W + j) & MAP_MASK ] =((int)t);
					 LMap[ (i * MAP_W + j) & MAP_MASK ] ^= 0xFF;
				}
			}
			norm=   512.0f / (float)(sqrt_max);
		}
	}
}

// *******************************

// Heightfield and colormap computation
//�n�C�g�t�B�[���h�ƐF�}�b�v�̌v�Z
void ComputeMap(void)
{
int p,i,j,k,k2,p2;

	// Start from a plasma clouds fractal
	//�u�t���N�^���̃v���Y�}�_�v����J�n
	HMap[0]=128;
	for(p=256; p>1; p=p2){
		p2=p>>1;
		k=p*8+20; k2=k>>1;
		for(i=0; i<256; i+=p){
			for(j=0; j<256; j+=p){
			int a,b,c,d;
				a=HMap[(  i         <<8)+  j         ];
				b=HMap[(((i+p )&255)<<8)+  j         ];
				c=HMap[(  i         <<8)+((j+p )&255)];
				d=HMap[(((i+p )&255)<<8)+((j+p )&255)];
				  HMap[(  i         <<8)+((j+p2)&255)]= Clamp(((a  +c  )>>1)+(sceRand()%k-k2));
				  HMap[(((i+p2)&255)<<8)+((j+p2)&255)]= Clamp(((a+b+c+d)>>2)+(sceRand()%k-k2));
				  HMap[(((i+p2)&255)<<8)+  j         ]= Clamp(((a+b    )>>1)+(sceRand()%k-k2));
			}
		}
	}

	for(k=0; k<=2; k++){
		for(i=0; i<256*256; i+=256){
			for(j=0; j<256; j++){
				HMap[i+j]=
					(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
					 HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
			}
		}
	}

	// Color computation (derivative of the height field)
	//�F�v�Z�i�����v�Z���ꏏ�ɂ��j
	for(i=0; i<256*256; i+=256){
		for(j=0; j<256; j++){
			k=128+(HMap[((i+256)&0xFF00)+((j+1)&255)]-HMap[i+j])*4;
			if      (k< 0) k=  0;
			else if (k>255) k=255;
			CMap[i+j]=k;
		}
	}
	calc_light(0,0,255,255);

	// Smoothing
	//�X���[�W���O
	for(i=0; i<256*256; i+=256){
		for(j=0; j<256; j++){
			HMap[i+j]=
				(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
				 HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
		}
	}
}

// *******************************

int lasty[ BUFFER_WIDTH];	// Last pixel drawn on a given column�`��ɕK�v�ȉ��ʒu�i�w�j�ōl�����A�Ō�̓_
int lastc[ BUFFER_WIDTH];	// Color of last pixel on a column���ʒu�i�w�j�ōl�����A�Ō�ɂ�����F�̓_
int lastx0[BUFFER_WIDTH];	// Color of last pixel on a column���ʒu�i�w�j�ōl�����A�Ō�ɂ�����F�̓_
int lasty0[BUFFER_WIDTH];	// Color of last pixel on a column���ʒu�i�w�j�ōl�����A�Ō�ɂ�����F�̓_
int lastl[ BUFFER_WIDTH];

// Draw a "section" of the landscape; x0,y0 and x1,y1 and the xy coordinates
// on the height field, hy is the viewpoint height, s is the scaling factor
// for the distance. x0,y0,x1,y1 are 16.16 fixed point numbers and the
// scaling factor is a 16.8 fixed point value.
//�n�`�̕`�惋�[�`���G �n�C�g�t�B�[���h�́ux0,y0�v�Ɓux1,y1�v�Ɓu�w�x�ʒu�v�B
// �uhy�v �͎��_�̍����A�us�v�͋����ōl�����g�嗦�B�ux0,y0,x1,y1�v�́A
// �R�Q�r�b�g�����i�P�U�r�b�g�D�P�U�r�b�g�j�̌Œ菬���_�`�����B
// �ŁA�g�嗦�́i�P�U�r�b�g�D�W�r�b�g�j�̌Œ菬���_�`���B
void Line(int x0,int y0,int x1,int y1,int hy,int s,int depth)
{
int state=0;
int i,sx,sy;
int hys=hy*s;

	// Compute xy speed�w�x���x�̌v�Z
	sx=(x1-x0)/BUFFER_WIDTH; sy=(y1-y0)/BUFFER_WIDTH;
	for(i=0; i<BUFFER_WIDTH-PIXELSIZE; i++)
	{
	int l,c,y,h,u0,v0,u1,v1,a,b,h0,h1,h2,h3;

		// Compute the xy coordinates; a and b will be the position inside the
		// single map cell (0..255).
		//�w�x�ʒu���v�Z�G���Ƃ��̓V���O���}�b�v�̃Z���i�O�`�Q�T�T�j�̒��g�ɂȂ�ׂ��ʒu�B
		u0=(x0>>16)&0xFF;	 a=(x0>>8)&255;
		v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
		u1=(u0+1)&0xFF;
		v1=(v0+256)&0xFF00;

		// Fetch the height at the four corners of the square the point is in
		//�l���̍����𒲂ׁA�l�p�̓_�ɔ��f������B
		h0=HMap[u0+v0]; h2=HMap[u0+v1];
		h1=HMap[u1+v0]; h3=HMap[u1+v1];
		h0=(h0<<8)+a*(h1-h0);
		h2=(h2<<8)+a*(h3-h2);
		h= (h0<<8)+b*(h2-h0);

		// Fetch the color at the four corners of the square the point is in
		//�l���̐F�𒲂ׁA�l�p�̓_�ɔ��f������B
		h0=LMap[u0+v0]; h2=LMap[u0+v1];
		h1=LMap[u1+v0]; h3=LMap[u1+v1];
		h0=(h0<<8)+a*(h1-h0);
		h2=(h2<<8)+a*(h3-h2);
		l=((h0<<8)+b*(h2-h0));

		// Compute screen height using the scaling factor
		//�g�嗦���l���ɂ���āA��ʂ̍������v�Z
		y=(h/256.0f)*(s/256.0f);
		y-=hys;
		y>>=11;
		y+=BUFFER_HEIGHT/2;

		// Draw the column
		//�s�i�w�j��`��
		if(y<(a=lasty[i]) && y>0)
		{
		int ubl= l;
		int lastubl= lastl[i];
		int lastlastubl=lastl[i-1];
		int ubstep;
		int sc,cc;
		int endy=BUFFER_HEIGHT;
			if(lasty[i]<BUFFER_HEIGHT)
				endy=lasty[i];
			ubstep=(((lastubl - ubl)) / (endy-y)) ;
			for(a=y; (a<lasty[i]) && (a<BUFFER_HEIGHT); a++)
			{
			unsigned short *Vptr=&Video[i*PIXELSKIP + a * 512];
			unsigned short Val=((ubl & 0x7fffffff) >> 16);// & 0x001f | 0x0e0 | 0x7c00;
				Val=((Val >> 3) & 0x1f) | (((Val >> 3) & 0x1f)<<10) | (((Val >> 3) & 0x1f)<<5);
				cc=PIXELSKIP;
				while(cc--)		*Vptr++=Val;
				ubl +=ubstep;
			}
			lasty[i]=y;
		}
		lastx0[i]=x0;
		lasty0[i]=y0;
		lastc[i]=c;
		lastl[i]=l;

		// Advance to next xy position
		//���̂w�x�ʒu�ɐi�߂�
		x0+=sx; y0+=sy;
	}
}

// *******************************
//
// Draw the view from the point x0,y0 (16.16) looking at angle a
// x0,y0 �͂R�Q�r�b�g�����i�P�U�r�b�g�D�P�U�r�b�g�j�̌Œ菬���_�`�����g���āA
// �p�x���݂Ȃ���`���ĕ\���B
void View(int x0,int y0,float aa)
{
	int d,p;
	int a,b,h,u0,v0,u1,v1,h0,h1,h2,h3;
	int p1, p2, p3, p4;

	for(d=0; d<BUFFER_WIDTH; d++)
	{
		lasty[d]=BUFFER_HEIGHT;
		lastc[d]=-1;
		lastx0[d]=0;
		lasty0[d]=0;
	}

	u0=(x0>>16)&0xFF;		a=(x0>>8)&255;
	v0=((y0>>8)&0xFF00);	b=(y0>>8)&255;
	u1=(u0+1)&0xFF;
	v1=(v0+256)&0xFF00;

	h0=HMap[u0+v0];			h2=HMap[u0+v1];
	h1=HMap[u1+v0];			h3=HMap[u1+v1];
	h0=(h0<<8)+a*(h1-h0);
	h2=(h2<<8)+a*(h3-h2);
	h=((h0<<8)+b*(h2-h0))>>16;

	p=0;
	for(d=0; d<500; d+=1+(d>>6))
	{
		p1=(int)((float)x0+(float)d*65535.0f*fcos(aa-FOV)) ;//<<16;
		p2=(int)((float)y0+(float)d*65535.0f*fsin(aa-FOV)) ;//<<16;
		p3=(int)((float)x0+(float)d*65535.0f*fcos(aa+FOV)) ;//<<16;
		p4=(int)((float)y0+(float)d*65535.0f*fsin(aa+FOV)) ;//<<16;
		Line(p1,p2,p3,p4,
			h-my_height,BUFFER_HEIGHT*128/(d+1),d );
	}
}

// *******************************

void input_key( void )	// �L�[�i������{�^���j���͂�����
{
	static unsigned long pad,pad1,lastpad,mvg;
	pad1=pgiGetpad();

//	if ((pad1&CTRL_START)!=0) { 	app_running=0;	}	// �X�^�[�g�ŏI��
// �i�d�l�ύX�������ߋ֎~�A�\�������j[HOME]�I�����Ă���B

	if ((pad1&CTRL_RIGHT)!=0) {		gl_sa += 0.005f;	}	//   �E�i���j�Ŋp���x�A���₷
	if ((pad1&CTRL_LEFT)!=0) {		gl_sa -= 0.005f;	}	//   ���i���j�Ŋp���x�A���炷

	if ((pad1&CTRL_UP)!=0) {		gl_ss += gl_s;		}	//   ���i���j�ŃX�s�[�h�A�b�v
	else{				if(0<gl_ss) gl_ss -= gl_s;		}	// ��C��R�ɂ�鎩�R�u���[�L
	if ((pad1&CTRL_DOWN)!=0) { 		gl_ss -= gl_s;		}	//   ���i���j�ŃX�s�[�h�_�E��
	else{				if(0>gl_ss) gl_ss += gl_s;		}	// ��C��R�ɂ�鎩�R�u���[�L

	if ((pad1&CTRL_CIRCLE)!=0) {	gl_sa  = 0;			}	//   �ہi���j�Ŋp���x�A�N���A
	if ((pad1&CTRL_CROSS)!=0) {		gl_ss  = 0;			}	// �o�c�i�~�j�� �X�s�[�h��~

	if ((pad1&CTRL_TRIANGLE)!=0)	// �O�p�i���j�ŁA�W�F�b�g���ˁB
	{	my_height += 1;	// ���_�̍������₷
		mvg=0;			// �d�͉����x�Ȃ�
	}

	if(my_height<0) {	/*�������Ȃ��i�܂�n�ʂɏՓˁi���܂����j�j��*/
		/* �ڒn���]�i�܂蒵�˂�j */
		mvg>>=1;	/* �G�l���M�[���X�i���S�e���Փ˂ł͂Ȃ��j */
		my_height= -my_height; mvg= -mvg;
	}else
	{	my_height-=mvg;			mvg++;				}	/* �d�͉����x���� */


	if ((pad1&CTRL_SQUARE)!=0) {	my_height -= 1;	mvg=0;	}	// �l�p�i���j�� �A���炷
	// mvg�̌덷�ݐς���݂����Ȃ̂ŏ����떂�����B

//	if ((pad1&CTRL_SELECT)!=0) {	gl_ss  = 0;			}	//   �Z���N�g�ŃX�s�[�h��~
//	if ((pad1&CTRL_CIRCLE)!=0) {	gl_sa += 0.005f;	}	//   �ہi���j�Ŋp���x�A���₷
//	if ((pad1&CTRL_CROSS)!=0) { 	gl_ss -= gl_s;		}	// �o�c�i�~�j�ŃX�s�[�h�_�E��
	lastpad=pad;
}

// *******************************

void glvox_main( void )
{
	View(gl_x0,gl_y0,gl_a);

	// Update position/angle
	// �ʒu��p�x���X�V����
	gl_x0+=gl_ss*fcos(gl_a);
	gl_y0+=gl_ss*fsin(gl_a);
	gl_a+=gl_sa;
	gl_sa*=0.975f;
}

// *******************************

int xmain(void)
{
static char flipper=1;	// �u�\��ʂƗ���ʂ�؂�ւ���v�p�̃t���O

	pgInit();
	SetupCallbacks();

	//	Main loop	���C�����[�v�̐���
	gl_a=0;			// a	= angle								�p�x
	gl_x0=0x0;		// x0	= current position x				���̉��ʒu
	gl_y0=0x0;		// y0	= current position y				���̏c�ʒu
	gl_s=4096;		// s	= speed constant					�X�s�[�h�i���x�j
	gl_ss=100.0f;	// ss	= current forward/backward speed	���̑O�i�^��ނ��Ă�X�s�[�h
	gl_sa=0;		// sa	= angular speed						�p���x
	my_height=30;

	// Compute the height map	�n�C�g�}�b�v���v�Z����B
	//�i�n�C�g�}�b�v���Ă̂́A�Q�����̔z��ō����̏��݂̂����z�̎��j

	ComputeMap();	// 	�}�b�v�i�n�}�j�v�Z

	pgcPuts("Loading...");	// ���[�h��������B

//	while(app_running)		// ���s���ł񂪂�
for(;;)
	{
		Video=pgGetVramAddr(0,0);	// �o�r�o�̂u�q�`�l�̃A�h���X�������Ă���
		pgScreenFrame(2,flipper);	// �i�\���p�j�\��ʂƗ���ʂ����ۂɐ؂�ւ���

//		pgcCls();	// ��ʂ�����
		// Rather than a screen clear, render half of the screen as a background
		// ��ʂ������āA�o�b�N��ʂ̔����Ƀ����_�����O�i�`��j
		pgBitBlt_clip(0, 0, 1694, 150, 1, SkyData, 1);

		flipper=(1-flipper);	// �\��ʂƗ���ʂ�؂�ւ���i�t���O����̂݁j
		glvox_main();			// �`��̃��C�����[�`��������
		input_key();			// �L�[�i������{�^���j���͂�����
		pgScreenFlipV();		// �i�`��p�j�\��ʂƗ���ʂ����ۂɐ؂�ւ���
	}

//	pgScreenFrame(1,0);		// ���z��ʂ�؂�ւ���
//	pgcCls();				// ��ʂ�����

	return 0;	// �d�l�ύX�����̂ŁA�����ɂ͐�΂ɂ��Ȃ��B�i���ꂫ����\������j[HOME]�I�����Ă���B
}

// *******************************

