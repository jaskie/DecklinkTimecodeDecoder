#pragma once
#include "ComInitializer.h"
#include "GdiplusInitializer.h"
#include "Keyer.h"

#pragma comment( lib, "gdiplus.lib" )

namespace TimecodeDecoder {
	public class DecklinkTimecodeDecoder : public IDeckLinkInputCallback
	{
	private:
		ComInitializer com_;
		GdiplusInitializer gdi_;
		const BMDTimecodeFormat timecodeSource_;
		const Keyer keyer_;
		CComQIPtr<IDeckLinkInput> input_;
		CComQIPtr<IDeckLinkOutput> output_;
		CComQIPtr<IDeckLinkKeyer> decklink_keyer_;
		CComQIPtr<IDeckLinkAttributes> input_attributes_;
		CComPtr<IDeckLinkVideoConversion> frame_converter_;
		IDeckLinkDisplayMode* current_mode_;
		Gdiplus::SolidBrush					background_;
		Gdiplus::SolidBrush					foreground_;
		std::unique_ptr<Gdiplus::Font>		font_;
		Gdiplus::StringFormat				timecode_format_;
		float								scale_x_; // for formats with non-square pixels
		Gdiplus::Rect						background_rect_;
		Gdiplus::PointF						timecode_position_;

		void SetupGdiElements();
		void Draw(CComPtr<IDeckLinkMutableVideoFrame>& video, IDeckLinkVideoInputFrame* time);
		void OpenInput(BMDDisplayMode displayMode);
		void OpenOutput(BMDDisplayMode format);
		void CloseInput();
	public:
		DecklinkTimecodeDecoder(int inputDecklinkIndex, int outputDecklinkIndex, BMDDisplayMode format, BMDTimecodeFormat timecodeSource, Keyer keyer);
		~DecklinkTimecodeDecoder();

		//IDeckLinkInputCallback
		virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(
			/* [in] */ BMDVideoInputFormatChangedEvents notificationEvents,
			/* [in] */ IDeckLinkDisplayMode* newDisplayMode,
			/* [in] */ BMDDetectedVideoInputFormatFlags detectedSignalFlags) override;

		virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(
			/* [in] */ IDeckLinkVideoInputFrame* input_frame,
			/* [in] */ IDeckLinkAudioInputPacket* audioPacket) override;

		//IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*) override { return E_NOINTERFACE; }
		ULONG STDMETHODCALLTYPE AddRef() override { return S_OK; }
		ULONG STDMETHODCALLTYPE Release() override { return S_OK; }

	};
}
