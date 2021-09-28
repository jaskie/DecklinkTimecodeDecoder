#pragma once
#include <gdiplus.h>

namespace TimecodeDecoder {
	class GdiplusInitializer {
	private:
		Gdiplus::GdiplusStartupInput	gdiplus_startup_input_;
		ULONG_PTR						gdiplus_token_;
	public:
		GdiplusInitializer() { Gdiplus::GdiplusStartup(&gdiplus_token_, &gdiplus_startup_input_, NULL); }
		~GdiplusInitializer() { Gdiplus::GdiplusShutdown(gdiplus_token_); }
	};

}