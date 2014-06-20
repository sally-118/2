/////////////////////////////////////////////////////////////////////////////
// This file is part of the completely free tetris clone "CGTetris".
//
// This is free software.
// You may redistribute it by any means providing it is not sold for profit
// without the authors written consent.
//
// No warrantee of any kind, expressed or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
/////////////////////////////////////////////////////////////////////////////


#ifndef DIBPAL_h
#define DIBPAL_h

class CBmpPalette : public CPalette
{
public:
    CBmpPalette( CDIBitmap* pBmp );
};

#endif	// DIBPAL_h