// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oManipulator.h>
#include "oManipulatorTranslation.h"
#include "oManipulatorScale.h"
#include "oManipulatorRotation.h"

bool oManipulatorCreate(const oManipulator::DESC& _Desc, oManipulator** _ppManipulator)
{
	bool success = false;
	switch (_Desc.Type)
	{
	case oManipulator::DESC::TRANSLATION:
		{
			oCONSTRUCT(_ppManipulator, oManipulatorTranslation(_Desc, &success));
		}
		break;
	case oManipulator::DESC::SCALE:
		{
			oCONSTRUCT(_ppManipulator, oManipulatorScale(_Desc, &success));
		}
		break;
	case oManipulator::DESC::ROTATION:
		{
			oCONSTRUCT(_ppManipulator, oManipulatorRotation(_Desc, &success));
		}
		break;
	}
	return success;
}