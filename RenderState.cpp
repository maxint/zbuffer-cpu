#include "RenderState.h"

CRenderState::CRenderState(void)
:mState(SL_COLOR_BUFFER | SL_DEPTH_BUFFER | SL_DEPTH_TEST | // depth test
		SL_SHADE_FLAT | SL_LIGHTING) // lighting
{
}

CRenderState::~CRenderState(void)
{
}

//------------------------------------------------------------------------------
// Render States
//------------------------------------------------------------------------------
void CRenderState::setState(int _state, int _val)
{
	switch (_state)
	{
	case SL_LIGHTING:
		if (_val)
			mState |= _state;
		else
			mState &= ~_state;
		break;
	case SL_BLENDING:
		if (_val)
			mState |= _state;
		else
			mState &= ~_state;
		break;
	case SL_COLOR_BUFFER:
		break;
	case SL_SHADE_FLAT:
		if (_val)
		{
			mState |= _state;
			mState &= ~SL_SHADE_SMOOTH;
		}
		else
		{
			mState |= SL_SHADE_SMOOTH;
			mState &= ~_state;
		}
		break;
	case SL_SHADE_SMOOTH:
		if (_val)
		{
			mState |= _state;
			mState &= ~SL_SHADE_FLAT;
		}
		else
		{
			mState |= SL_SHADE_FLAT;
			mState &= ~_state;
		}
		break;
	default:
		break;
	}
}