/*
    Copyright � 1995-2015, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Parallel hidd class implementation.
    Lang: english
*/

#define __OOP_NOATTRBASES__

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/oop.h>
#include <exec/libraries.h>

#include <utility/tagitem.h>
#include <hidd/parallel.h>


#include "parallel_intern.h"

#include <aros/debug.h>


/*static AttrBase HiddGCAttrBase;*/

static OOP_AttrBase HiddParallelUnitAB;

/*** HIDDParallel::NewUnit() *********************************************************/


OOP_Object *AmigaPar__Hidd_Parallel__NewUnit(OOP_Class *cl, OOP_Object *obj,
					  struct pHidd_Parallel_NewUnit *msg)
{
	OOP_Object *su = NULL;
	struct HIDDParallelData * data = OOP_INST_DATA(cl, obj);
	ULONG unitnum = -1;

	EnterFunc(bug("HIDDParallel::NewParallel()\n"));

	D(bug("Request for unit number %d\n",msg->unitnum));

	if (msg->unitnum >= 0 && msg->unitnum < PAR_MAX_UNITS) {
            unitnum = msg->unitnum;

            if (0 != (data->usedunits & (1 << unitnum))) {
                unitnum = -1;
            }
        } else if (msg->unitnum == -1) {
                /* search for the next available unit */
	        unitnum = 0;

                while (unitnum < PAR_MAX_UNITS) {
                        if (0 == (data->usedunits & (1 << unitnum))) {
                                break;
                        }

                        unitnum++;
                }
	}

	if (unitnum >= 0 && unitnum < PAR_MAX_UNITS) {
		struct TagItem tags[] =
		{
			{aHidd_ParallelUnit_Unit, unitnum},
			{TAG_DONE		       }
		};

		su = OOP_NewObject(NULL, CLID_Hidd_ParallelUnit, tags);
		data->ParallelUnits[unitnum] = su;

		/*
		** Mark it as used
		*/
		data->usedunits |= (1 << unitnum); 
	}

	ReturnPtr("HIDDParallel::NewParallel", OOP_Object *, su);
}


/*** HIDDParallel::DisposeUnit() ****************************************************/

VOID AmigaPar__Hidd_Parallel__DisposeUnit(OOP_Class *cl, OOP_Object *obj,
				       struct pHidd_Parallel_DisposeUnit *msg)
{
	OOP_Object * pu = msg->unit;
	struct HIDDParallelData * data = OOP_INST_DATA(cl, obj);

	EnterFunc(bug("HIDDParallel::DisposeUnit()\n"));

	if(pu) {
		ULONG unitnum = 0;

		while (unitnum < PAR_MAX_UNITS) {
			if (data->ParallelUnits[unitnum] == pu) {
				D(bug("Disposing ParallelUnit!\n"));
				OOP_DisposeObject(pu);
				data->ParallelUnits[unitnum] = NULL;
				data->usedunits &= ~(1 << unitnum);
				break;
			}

			unitnum++;
		}
	}

	ReturnVoid("HIDDParallel::DisposeUnit");
}
