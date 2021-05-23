// *******************************
// This is a port of a classic old voxel terrain renderer. Andrea kindly
// released his code back in 2000 so here it is again but on PSP!!!
// これは「Andrea kindly」さんが２０００年に書いて発表した古いコードを
// ＰＳＰ用に作り直した物だぎゃ！！！。
// The other person that needs thanking is Nem - your hello world, sound
// and pg lib made there way here, I hope thats ok? :-)
// 他の人の協力も必要だった。ＮＥＭさんとか。彼の「ＨｅｌｌｏＷｏｒｌｄ」の
// 音とかＰＧのライブラリとか、勝手につかっても良いよねぇ？（＾＾）
// The rest of it is written by Grover.. for anyone to use, and abuse.
// The windows rendering is incomplete and only a few sce functions have
// been emulated - its an ongoing thing, so it will get filled out in the
// next week or so...
// 残りは私（Grover）が書いた..。何に使ってもいいから、勝手にしてくれ。
// 画面表示のレンダリングは（コードでなくて画面が）書きかけだし。
// ＳＣＥの関数はちょっとしかわかんないし、－  でもそのうちなんとかなるじゃろ。
// 来週あたりには、（ＳＣＥライブラリの解析がハックされて）出来てるんじゃぁないかな...
//				Grover - May 2005.（２００５年５月）
//				dlannan@gagagames.com

// *******************************（「Glvox」＝＝このプログラムの名称）
// Credits for Original Glvox writer     「Glvox」のオリジナルコードを書いた人。
//  - Andrea "6502" Griffini, programmer 「Andrea "6502" Griffini」さん。プログラマーでし。
//    agriff@ix.netcom.com
//    http://vv.val.net/~agriffini
// *******************************

#include "syscall.h"
#include "pg.h"
#include "_clib.h"

// *******************************
// Not yet used.. yet..まだないけど、使ってるの
#include "Sky.c"
//#include "Water1.c"

// *******************************

#ifdef WIN32
#include "win32_psp.h"
#endif

// *******************************

#define PIXELSKIP		4
#define CLOCKS_PER_SEC	1000000

float FOV=3.141592654/4;	// half of the xy field of viewＸＹ領域の半分を見せる

// *******************************

#define BUFFER_WIDTH	((512/PIXELSKIP)+1)
#define BUFFER_HEIGHT	271

// *******************************
// These dont seem to hurt the ps2 compiler..これらはＥＥのコンパイラにないよ...。

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

//unsigned char app_running=1;	// 実行中フラグ＝１（実行中でんがな）
//仕様変更した為、使用禁止でんがな。[HOME]終了してくれ。

//unsigned short *HMap=(unsigned short *)Water1Data;	// Height fieldハイトフィールド

unsigned char HMap[MAP_W*MAP_H];						// Color map色マップ
unsigned char CMap[MAP_W*MAP_H];						// Color map色マップ
unsigned char LMap[MAP_W*MAP_H];

unsigned short *Video;
// unsigned char Video[BUFFER_WIDTH * BUFFER_HEIGHT];	// Off-screen buffer仮想画面

static float gl_ss, gl_sa, gl_a, gl_s;
static int gl_x0, gl_y0;

static my_height;





volatile int bSleep=0;

// -----------------------------------------------------------------------------

// ホームボタン終了時にコールバック
int exit_callback(void)
{
	bSleep=1;
	scePowerSetClockFrequency(222,222,111);
// Cleanup the games resources etc (if required)

// Exit game
	sceKernelExitGame();
	return 0;
}

// 電源スイッチ操作時や不定期にコールバック。
// この関数がまだ実行中でもサスペンド・スタンバイに入る可能性がある。
void power_callback(int unknown, int pwrflags)
{
	//if(pwrflags & (POWER_CB_SUSPEND|POWER_CB_STANDBY)){
	if(pwrflags & POWER_CB_POWER){
		if (!bSleep){
			bSleep=1;

			// ファイルアクセス中にサスペンド・スタンバイされて
			// 0byteのセーブファイルができてしまうことがあるので、
			// 書き込み中はサスペンド・スタンバイを無効化。
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

			// 強制サスペンド。
			// バッテリが約10%を切りパワーランプが点滅を始めると、
			// 動作が極端に遅くなりフリーズしたりセーブできなくなったりする。
			// 市販ゲームでは0%まで使えてるようなのが謎。
			scePowerRequestSuspend();
		}
	}
	if(pwrflags & POWER_CB_RESCOMP){
		bSleep=0;
	}
	// コールバック関数の再登録
	// （一度呼ばれたら再登録しておかないと次にコールバックされない）
	int cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);
}

// ポーリング用スレッド
int CallbackThread(int args, void *argp)
{
	int cbid;

	// コールバック関数の登録
	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
	scePowerRegisterCallback(0, cbid);

	// ポーリング
	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	// ポーリング用スレッドの生成
	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);

	return thid;
}
// *******************************
// Reduces a value to 0..255 (used in height field computation)
// 縮小値０～２５５（高さの計算でつかってます。だぎゃ）
int Clamp(int x)
{
	return (x<0 ? 0 : (x>255 ? 255 : x));
}

// *******************************（光源計算部）
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
					// ちょっと回りくどく変な風になってんのは、私が見つけたＧＣＣのバグのせいだ。
					if( (*(unsigned int *)&t) & 0x80000000 ) t=0.0f;
//					if( t<0.0f ) t=0.0f;			// 本当はこう書きたかった。
					if( ((int )t) > 255) t=255.0f;
//					if( t > 255.0f ) t=255.0f;		// 本当はこう書きたかった。
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
//ハイトフィールドと色マップの計算
void ComputeMap(void)
{
int p,i,j,k,k2,p2;

	// Start from a plasma clouds fractal
	//「フラクタルのプラズマ雲」から開始
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
	//色計算（高さ計算も一緒にやる）
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
	//スムージング
	for(i=0; i<256*256; i+=256){
		for(j=0; j<256; j++){
			HMap[i+j]=
				(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
				 HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
		}
	}
}

// *******************************

int lasty[ BUFFER_WIDTH];	// Last pixel drawn on a given column描画に必要な横位置（Ｘ）で考えた、最後の点
int lastc[ BUFFER_WIDTH];	// Color of last pixel on a column横位置（Ｘ）で考えた、最後にあたる色の点
int lastx0[BUFFER_WIDTH];	// Color of last pixel on a column横位置（Ｘ）で考えた、最後にあたる色の点
int lasty0[BUFFER_WIDTH];	// Color of last pixel on a column横位置（Ｘ）で考えた、最後にあたる色の点
int lastl[ BUFFER_WIDTH];

// Draw a "section" of the landscape; x0,y0 and x1,y1 and the xy coordinates
// on the height field, hy is the viewpoint height, s is the scaling factor
// for the distance. x0,y0,x1,y1 are 16.16 fixed point numbers and the
// scaling factor is a 16.8 fixed point value.
//地形の描画ルーチン； ハイトフィールドは「x0,y0」と「x1,y1」と「ＸＹ位置」。
// 「hy」 は視点の高さ、「s」は距離で考えた拡大率。「x0,y0,x1,y1」は、
// ３２ビット＝＝（１６ビット．１６ビット）の固定小数点形式だ。
// で、拡大率は（１６ビット．８ビット）の固定小数点形式。
void Line(int x0,int y0,int x1,int y1,int hy,int s,int depth)
{
int state=0;
int i,sx,sy;
int hys=hy*s;

	// Compute xy speedＸＹ速度の計算
	sx=(x1-x0)/BUFFER_WIDTH; sy=(y1-y0)/BUFFER_WIDTH;
	for(i=0; i<BUFFER_WIDTH-PIXELSIZE; i++)
	{
	int l,c,y,h,u0,v0,u1,v1,a,b,h0,h1,h2,h3;

		// Compute the xy coordinates; a and b will be the position inside the
		// single map cell (0..255).
		//ＸＹ位置を計算；ａとｂはシングルマップのセル（０～２５５）の中身になるべき位置。
		u0=(x0>>16)&0xFF;	 a=(x0>>8)&255;
		v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
		u1=(u0+1)&0xFF;
		v1=(v0+256)&0xFF00;

		// Fetch the height at the four corners of the square the point is in
		//四隅の高さを調べ、四角の点に反映させる。
		h0=HMap[u0+v0]; h2=HMap[u0+v1];
		h1=HMap[u1+v0]; h3=HMap[u1+v1];
		h0=(h0<<8)+a*(h1-h0);
		h2=(h2<<8)+a*(h3-h2);
		h= (h0<<8)+b*(h2-h0);

		// Fetch the color at the four corners of the square the point is in
		//四隅の色を調べ、四角の点に反映させる。
		h0=LMap[u0+v0]; h2=LMap[u0+v1];
		h1=LMap[u1+v0]; h3=LMap[u1+v1];
		h0=(h0<<8)+a*(h1-h0);
		h2=(h2<<8)+a*(h3-h2);
		l=((h0<<8)+b*(h2-h0));

		// Compute screen height using the scaling factor
		//拡大率を考慮にいれて、画面の高さを計算
		y=(h/256.0f)*(s/256.0f);
		y-=hys;
		y>>=11;
		y+=BUFFER_HEIGHT/2;

		// Draw the column
		//行（Ｘ）を描く
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
		//次のＸＹ位置に進める
		x0+=sx; y0+=sy;
	}
}

// *******************************
//
// Draw the view from the point x0,y0 (16.16) looking at angle a
// x0,y0 は３２ビット＝＝（１６ビット．１６ビット）の固定小数点形式を使って、
// 角度をみながら描いて表示。
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

void input_key( void )	// キー（方向やボタン）入力だぎゃ
{
	static unsigned long pad,pad1,lastpad,mvg;
	pad1=pgiGetpad();

//	if ((pad1&CTRL_START)!=0) { 	app_running=0;	}	// スタートで終了
// （仕様変更したため禁止、暴走するよ）[HOME]終了してくれ。

	if ((pad1&CTRL_RIGHT)!=0) {		gl_sa += 0.005f;	}	//   右（→）で角速度、増やす
	if ((pad1&CTRL_LEFT)!=0) {		gl_sa -= 0.005f;	}	//   左（←）で角速度、減らす

	if ((pad1&CTRL_UP)!=0) {		gl_ss += gl_s;		}	//   ↑（↑）でスピードアップ
	else{				if(0<gl_ss) gl_ss -= gl_s;		}	// 空気抵抗による自然ブレーキ
	if ((pad1&CTRL_DOWN)!=0) { 		gl_ss -= gl_s;		}	//   ↓（↓）でスピードダウン
	else{				if(0>gl_ss) gl_ss += gl_s;		}	// 空気抵抗による自然ブレーキ

	if ((pad1&CTRL_CIRCLE)!=0) {	gl_sa  = 0;			}	//   丸（○）で角速度、クリア
	if ((pad1&CTRL_CROSS)!=0) {		gl_ss  = 0;			}	// バツ（×）で スピード停止

	if ((pad1&CTRL_TRIANGLE)!=0)	// 三角（△）で、ジェット噴射。
	{	my_height += 1;	// 視点の高さ増やす
		mvg=0;			// 重力加速度なし
	}

	if(my_height<0) {	/*高さがない（つまり地面に衝突（埋まった））時*/
		/* 接地反転（つまり跳ねる） */
		mvg>>=1;	/* エネルギーロス（完全弾性衝突ではない） */
		my_height= -my_height; mvg= -mvg;
	}else
	{	my_height-=mvg;			mvg++;				}	/* 重力加速度発生 */


	if ((pad1&CTRL_SQUARE)!=0) {	my_height -= 1;	mvg=0;	}	// 四角（□）で 、減らす
	// mvgの誤差累積するみたいなので少し誤魔化す。

//	if ((pad1&CTRL_SELECT)!=0) {	gl_ss  = 0;			}	//   セレクトでスピード停止
//	if ((pad1&CTRL_CIRCLE)!=0) {	gl_sa += 0.005f;	}	//   丸（○）で角速度、増やす
//	if ((pad1&CTRL_CROSS)!=0) { 	gl_ss -= gl_s;		}	// バツ（×）でスピードダウン
	lastpad=pad;
}

// *******************************

void glvox_main( void )
{
	View(gl_x0,gl_y0,gl_a);

	// Update position/angle
	// 位置や角度を更新する
	gl_x0+=gl_ss*fcos(gl_a);
	gl_y0+=gl_ss*fsin(gl_a);
	gl_a+=gl_sa;
	gl_sa*=0.975f;
}

// *******************************

int xmain(void)
{
static char flipper=1;	// 「表画面と裏画面を切り替える」用のフラグ

	pgInit();
	SetupCallbacks();

	//	Main loop	メインループの説明
	gl_a=0;			// a	= angle								角度
	gl_x0=0x0;		// x0	= current position x				今の横位置
	gl_y0=0x0;		// y0	= current position y				今の縦位置
	gl_s=4096;		// s	= speed constant					スピード（速度）
	gl_ss=100.0f;	// ss	= current forward/backward speed	今の前進／後退してるスピード
	gl_sa=0;		// sa	= angular speed						角速度
	my_height=30;

	// Compute the height map	ハイトマップを計算する。
	//（ハイトマップってのは、２次元の配列で高さの情報のみを持つ奴の事）

	ComputeMap();	// 	マップ（地図）計算

	pgcPuts("Loading...");	// ロード中だぎゃ。

//	while(app_running)		// 実行中でんがな
for(;;)
	{
		Video=pgGetVramAddr(0,0);	// ＰＳＰのＶＲＡＭのアドレスを持ってくる
		pgScreenFrame(2,flipper);	// （表示用）表画面と裏画面を実際に切り替える

//		pgcCls();	// 画面を消す
		// Rather than a screen clear, render half of the screen as a background
		// 画面を消して、バック画面の半分にレンダリング（描画）
		pgBitBlt_clip(0, 0, 1694, 150, 1, SkyData, 1);

		flipper=(1-flipper);	// 表画面と裏画面を切り替える（フラグ操作のみ）
		glvox_main();			// 描画のメインルーチンだぎゃ
		input_key();			// キー（方向やボタン）入力だぎゃ
		pgScreenFlipV();		// （描画用）表画面と裏画面を実際に切り替える
	}

//	pgScreenFrame(1,0);		// 仮想画面を切り替える
//	pgcCls();				// 画面を消す

	return 0;	// 仕様変更したので、ここには絶対にこない。（万一きたら暴走する）[HOME]終了してくれ。
}

// *******************************

