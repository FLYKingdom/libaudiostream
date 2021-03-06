/*

Copyright (C) Grame 2002-2013

This library is free software; you can redistribute it and modify it under
the terms of the GNU Library General Public License as published by the
Free Software Foundation version 2 of the License, or any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public License
for more details.

You should have received a copy of the GNU Library General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Grame Research Laboratory, 9, rue du Garet 69001 Lyon - France
research@grame.fr

*/

#ifndef __TPanTable__
#define __TPanTable__

#include <math.h>

#if !defined(PI)
 #define PI (float)3.14159265359
#endif

//------------------
// Class TPanTable
//------------------
/*!
\brief Tools for volume and panning control.
*/

class TPanTable
{

    private:

        static float fPanTable[128];
        static float fVolTable[128];

        static int Range(int val, int min, int max)
        {
            return (val > max) ? max : (val < min) ? min : val;
        }

    public:

        TPanTable()
        {}
        virtual ~TPanTable()
        {}
		
		static void GetLR(float vol, float pan, float* left_vol, float* right_vol)
		{
			*left_vol = vol * sin(pan * PI/2.0);
			*right_vol = vol * cos(pan * PI/2.0);
		}
	
		static float GetVol(short vol)
        {
            return fVolTable[Range(vol, 0, 127)];
        }
	  
        static void FillTable()
        {
            int i;

            for (i = 0; i < 128; i++) {
                float val = float(cos(((float)i) / 127.0f * PI / 2));
                fPanTable[i] = val * val;
            }

            for (i = 1; i < 128; i++) {
                float val = ((float)i) / 127.0f;
                fVolTable[i] = val * val;
            }
            fVolTable[0] = 0;
        }

};

typedef TPanTable * TPanTablePtr;

#endif


