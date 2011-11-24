#pragma once

#define SL_COLOR_BUFFER 0x0001
#define SL_DEPTH_BUFFER 0x0002
#define SL_LIGHTING		0x0004
#define SL_BLENDING		0x0008
#define SL_SHADE_FLAT	0x0010
#define SL_SHADE_SMOOTH	0x0020
#define SL_DEPTH_TEST	0x0040

class CRenderState
{
public:
	CRenderState(void);
	~CRenderState(void);

	void setState(int _state, int _val);
	int state() { return mState; }

	inline bool isFlatShading() const { return (mState & SL_LIGHTING) && (mState & SL_SHADE_FLAT); }
	inline bool isSmoothShading() const { return (mState & SL_LIGHTING) && (mState & SL_SHADE_SMOOTH); }
	inline bool isLighting() const { return (mState&SL_LIGHTING) > 0; }
	inline bool isBlending() const { return (mState&SL_BLENDING) > 0; }
	
private:
	int mState;
};
