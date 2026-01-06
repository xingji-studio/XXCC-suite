/*
 *
 *      XINGJI XJ380 Application Compile Tools (XACT) 
 *      XACT C\C++ Header
 *      XJ380API XAPI Edition GUI Part
 *      Release Version 1.0.0 - 2025/5/10
 *      Copyright(C) XINGJI Interactive Software 2017 - 2026 All rights reserved.
 * 
 *      A XINGJI Interactive Software Production
 * 
 */

#pragma once

#include "stdint.h"

// define
typedef UINT64 		HDLE;
typedef void 		(*MsgPrcor)(
	UINT64 Type, 
	UINT64 hData, 
	UINT64 lData
);

struct XWINDOW
{
    UINT32 	width;
    UINT32 	height;
    WSTR 	title;
    UINT8 	sets;
};

#pragma pack(1)

struct XCOLOR
{
	UINT8 	Red;
	UINT8 	Green;
	UINT8 	Blue;
};

#pragma pack()

struct XCOLORA
{
	UINT8 	Red;
	UINT8 	Green;
	UINT8 	Blue;
	UINT8	Alpha;
};


// 4-1
#define XWIN_NORMAL				0
#define XWIN_FRAME_OFF			1
#define XWIN_FULL_SCR			2

void  xapi_CreateWindow(
	HDLE*		handle,
	XWINDOW* 	xwin
);

void  xapi_SetWindowTitle(
	HDLE  	handle,
	WSTR  	str
);

void  xapi_CloseWindow(
    HDLE  	handle
);

void  xapi_SetIcon(
	HDLE  	handle,
	WSTR  	path
);

void xapi_GetWindowSize(
	HDLE handle, 
	UINT64* width, 
	UINT64* height
);


// 4-2
void  xapi_DrawPoint (
	HDLE  	handle,
	UINT32  x,
	UINT32  y,
	UINT32  color
);

void  xapi_DrawLine(
    HDLE  	handle,
	UINT32 	x1,
	UINT32 	y1,
	UINT32 	x2,
	UINT32 	y2,
	UINT32 	color
);

void  xapi_DrawRect(
	HDLE 	handle,
	UINT32 	x1,
	UINT32  y1,
	UINT32  x2,
	UINT32 	y2,
	UINT32 	color,
	bool  	fill
);

// void  xapi_DrawCircle(
// 	HDLE  	handle,
// 	UINT32 	x,
// 	UINT32 	y,
// 	UINT32 	radius,
// 	UINT32 	color,
// 	bool  	fill
// );

void  xapi_DrawText(
	HDLE  	handle,
	UINT32 	x,
	UINT32 	y,
	WSTR 	str,
	UINT32	size,
	UINT32  color
);

void  xapi_DrawTextl(
	HDLE  	handle,
	UINT32 	x,
	UINT32 	y,
	WSTR 	str,
	UINT32	size,
	UINT32  color,
	UINT32 *width
);

void xapi_DrawSWText(
	HDLE  	handle,
	UINT32 	x,
	UINT32 	y,
	WSTR 	str,
	UINT32  color
);

UINT64 xapi_CalcTextWidth(WSTR str, UINT32 size);

// 4-3
void  xapi_DrawBMP (
	HDLE  	handle,
	UINT32  x,
	UINT32  y,
	UINT32	width,
	UINT32	height,
	WSTR  	path
);

void  xapi_DrawPNG(
	HDLE  	handle,
	UINT32  x,
	UINT32  y,
	UINT32	width,
	UINT32	height,
	WSTR  	path
);

void xapi_DrawPicture(
	HDLE  	handle,
	UINT32  x,
	UINT32  y,
	UINT32	width,
	UINT32	height,
	WSTR  	path
);

// 4-4
void SetMsgPrcor(HDLE handle, MsgPrcor func);
#define MSG_CHAR		0
#define MSG_SPCHAR		7
#define MSG_MOVE		1
#define MSG_LBUTTON		2
#define MSG_RBUTTON		3
#define MSG_MBUTTON		4
#define MSG_ROLLER		5
#define MSG_CRL			6

// 4-5
void  xapi_ReadBuffer(
	HDLE  	 	handle,
	UINT32  	x,
	UINT32  	y,
	UINT32  	width,
	UINT32		height,
	XCOLOR*  	buffer
);

void xapi_WriteBuffer(
	HDLE  	 	handle,
	UINT32  	x,
	UINT32  	y,
	UINT32  	width,
	UINT32  	height,
	XCOLOR*  	buffer
);

void xapi_WriteBufferA(
	HDLE  	 	handle,
	UINT32  	x,
	UINT32  	y,
	UINT32  	width,
	UINT32  	height,
	XCOLORA*  	buffer
);

void xapi_RefreshWindow(
	HDLE		handle
);

// 4-6
void xapi_Button(
	HDLE  	handle,
	UINT64  CRLid,
	UINT64  x,
	UINT64  y,
	WSTR  	text
);

void xapi_ButtonEmp(
	HDLE  	handle,
	UINT64  CRLid,
	UINT64  x,
	UINT64  y,
	WSTR  	text
);
