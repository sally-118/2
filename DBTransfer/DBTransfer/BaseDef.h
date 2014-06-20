// ===========================================================================
//	BaseDef.h  Define some base types or Macros
//	2002 Software Inc. All rights reserved.
// 	Author: WLiang
//	Date:	2002.1
// ===========================================================================

#ifndef __BASEDEF
#define __BASEDEF

typedef unsigned char	U8;
typedef char			S8;
typedef unsigned short	U16;
typedef short			S16;
typedef unsigned long	U32;
typedef long			S32;

typedef RECT			SRect32;
typedef POINT			SPoint32;

#define DISPOSEP(pClass) {if(pClass) { delete pClass; pClass = NULL;}}

#define ROWBYTES41(WidthPixels)	 (((WidthPixels)+31)/32*4)
#define	ROWBYTES48(WidthPixels)	 (((WidthPixels)+7)/8*8)
#define	ROWBYTES4ARGB(WidthPixels)	 (WidthPixels*4)
#define ROWBYTES(WidthPixels, bc) ((bc==1)?ROWBYTES41(WidthPixels):ROWBYTES48(WidthPixels))
#define QALIGN(w) (((w)+7)/8*8)
#define RECTW(rect)	(rect.right-rect.left)
#define RECTH(rect)	(rect.bottom-rect.top)

#define STRUCT0(struct1)	::memset(&struct1, 0, sizeof(struct1));

const	U32		ONE_K	= 0X400;
const	U32		ONE_M	= 0X100000;
const	U32		ONE_G	= 0X40000000;
const	U32		MAXID	= 0XFFFFFF;
const 	double	PI		= 3.1415926535898;


const U32 CONSTPyramidLevel = 8;

typedef struct tagDataTransfer
{
	U8*	bitsStream;
	S32	rowBytes, dtX, dtY;
	U16	retValue;
}DT;

typedef struct tagRGBTransfer
{
	U8*	dataStream;
	S32	rowBytes, dtX, dtY;
	U8	retValue;
}RGBDT;

typedef struct tagARGBTransfer
{
	U8*	bitsStream;
	S32	rowBytes, dtX, dtY;
	U8	retValue;
	U8*	redStream;
	U8*	greenStream;
	U8*	blueStream;
}ARGBDT;

typedef struct tagCombineInfo
{
	S16		mOpacity;//0--256, 0=0%, 256=100%
	S16		mCombineMode;//1=normal; 2=copy;3=not 0 copy
	U32		mReserved1;
}CombineInfo;
//mode 0x10 a1*a2
//mode 0x11 a1*~a2
//mode 0x1 a1+a2-a1*a2
//mode 0x3 if a2 =0, keep a1, else a1 =a2
//mode 0x4 a1-a1*a2*opacity/512
//mode 0x5 a1+a1*a2*opacity/512

const S16 CONSTHalfOpacity = 128;

enum 	OPCODE	{ copy, or, and, xor, xand };
enum	NetShape{ROUND=0,SQUARE,CROSS,LINE,BEE7CELL,BEECELL,SELFDEFINED};

// A CombineData hook 
typedef void (* POPDT_HOOK)(DT* dt1, DT* dt2, S32 opW, S32 opH, S16 bitsCnt1, S16 bitsCnt2, void* lparam);

#endif
