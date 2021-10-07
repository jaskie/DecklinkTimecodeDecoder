#include "pch.h"
#include "DecklinkTimecodeDecoder.h"

namespace TimecodeDecoder {

	DecklinkTimecodeDecoder::DecklinkTimecodeDecoder(int inputDecklinkIndex, int outputDecklinkIndex, BMDDisplayMode format, BMDTimecodeFormat timecodeSource, Keyer keyer)
		: timecodeSource_(timecodeSource)
		, keyer_(keyer)
		, background_(Gdiplus::Color(150, 16, 16, 16))
		, foreground_(Gdiplus::Color(255, 232, 232, 232))
		, scale_x_(1)
	{
		timecode_format_.SetAlignment(Gdiplus::StringAlignmentCenter);
		timecode_format_.SetLineAlignment(Gdiplus::StringAlignmentCenter);

		CComPtr<IDeckLinkIterator> pDecklinkIterator;
		if (FAILED(pDecklinkIterator.CoCreateInstance(CLSID_CDeckLinkIterator)))
			throw std::exception("Iterator create failed");
		IDeckLink* decklink;
		int index = 0;
		while (pDecklinkIterator->Next(&decklink) == S_OK)
		{
			if (index == inputDecklinkIndex)
			{
				input_ = decklink;
				input_attributes_ = decklink;
			}
			if (index == outputDecklinkIndex)
				output_ = decklink;
			if (index == inputDecklinkIndex && index == outputDecklinkIndex && keyer == Keyer::Internal)
				decklink_keyer_ = decklink;
			index++;
			decklink->Release();
		}
		if (!input_) 
			throw std::exception("Input decklink not found");
		if (!output_)
			throw std::exception("Output decklink not found");
		input_->SetCallback(this);
		OpenOutput(format);
		OpenInput(format);
	}

	DecklinkTimecodeDecoder::~DecklinkTimecodeDecoder()
	{
		CloseInput();
		output_->DisableVideoOutput();
	}

	void DecklinkTimecodeDecoder::SetupGdiElements()
	{
		scale_x_ = current_mode_->GetDisplayMode() == BMDDisplayMode::bmdModePAL ? 9.f / 16 : 1.f;
		int height = current_mode_->GetHeight() / 6;
		int width = static_cast<int>(current_mode_->GetHeight() * scale_x_);
		background_rect_ = Gdiplus::Rect((current_mode_->GetWidth() - width) / 2, current_mode_->GetHeight() - (height * 4 / 3), width, height);
		timecode_position_ = Gdiplus::PointF(background_rect_.Width / (scale_x_ * 2), static_cast<Gdiplus::REAL>(background_rect_.Height * 21 / 40));
		font_ = std::make_unique<Gdiplus::Font>(L"Tahoma", static_cast<Gdiplus::REAL>(current_mode_->GetHeight() / 9), Gdiplus::FontStyle::FontStyleBold);
	}

	void DecklinkTimecodeDecoder::Draw(CComPtr<IDeckLinkMutableVideoFrame>& video, IDeckLinkVideoInputFrame* time)
	{
		void* dest_bytes = nullptr;
		if (FAILED(video->GetBytes(&dest_bytes)))
			return;
		CComPtr<IDeckLinkTimecode> timecode;
		std::wstring wstr;
		BSTR timecode_str = nullptr;
		if (SUCCEEDED(time->GetTimecode(timecodeSource_, &timecode))
			&& timecode
			&& SUCCEEDED(timecode->GetString(&timecode_str)))
		{
			wstr = timecode_str;
			::SysReleaseString(timecode_str);
		}
		else
			wstr = L"NO TC DATA";

		Gdiplus::Bitmap frame_bitmap(video->GetWidth(), video->GetHeight(), video->GetRowBytes(), PixelFormat32bppARGB, static_cast<BYTE*>(dest_bytes));
		Gdiplus::Graphics frame_graphics(&frame_bitmap);
		if (decklink_keyer_)
			frame_graphics.Clear(Gdiplus::Color(0, 16, 16, 16));
		else
		{
			if (!frame_converter_)
				frame_converter_.CoCreateInstance(CLSID_CDeckLinkVideoConversion);
			frame_converter_->ConvertFrame(time, video);
		}
		frame_graphics.FillRectangle(&background_, background_rect_);
		Gdiplus::Bitmap overlay_bitmap(background_rect_.Width, background_rect_.Height, PixelFormat32bppARGB);
		Gdiplus::Graphics overlay_graphics(&overlay_bitmap);
		overlay_graphics.ScaleTransform(scale_x_, 1.0);
		overlay_graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
		overlay_graphics.DrawString(wstr.c_str(), -1, font_.get(), timecode_position_, &timecode_format_, &foreground_);
		frame_graphics.DrawImage(&overlay_bitmap, background_rect_);
		frame_graphics.Flush(Gdiplus::FlushIntentionSync);
	}


	void DecklinkTimecodeDecoder::OpenInput(BMDDisplayMode format)
	{
		BMDDisplayModeSupport support;
		if (FAILED(input_->DoesSupportVideoMode(format, BMDPixelFormat::bmdFormat8BitYUV, bmdVideoInputFlagDefault, &support, &current_mode_)))
			throw std::exception("DecklinkInput: DoesSupportVideoMode failed");
		if (support == BMDDisplayModeSupport::bmdDisplayModeNotSupported)
			throw std::exception("DecklinkInput: Display mode not supported");

		BOOL format_auto_detection = false;
		if (FAILED(input_attributes_->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &format_auto_detection)))
			format_auto_detection = false;
		if (FAILED(input_->EnableVideoInput(current_mode_->GetDisplayMode(), BMDPixelFormat::bmdFormat8BitYUV, format_auto_detection ? bmdVideoInputEnableFormatDetection : bmdVideoInputFlagDefault)))
			throw std::exception("DecklinkInput: EnableVideoInput failed");
		SetupGdiElements();
		if (FAILED(input_->StartStreams()))
			throw std::exception("DecklinkInput: StartStreams failed");
	}

	void DecklinkTimecodeDecoder::CloseInput()
	{
		if (FAILED(input_->StopStreams()))
			throw std::exception("DecklinkInput: StopStreams failed");
		if (FAILED(input_->DisableVideoInput()))
			throw std::exception("DecklinkInput: DisableVideoInput failed");
	}

	void DecklinkTimecodeDecoder::OpenOutput(BMDDisplayMode format)
	{
		BMDDisplayModeSupport modeSupport;
		if (FAILED(output_->DoesSupportVideoMode(format, BMDPixelFormat::bmdFormat8BitYUV, BMDVideoOutputFlags::bmdVideoOutputFlagDefault, &modeSupport, NULL))
			|| modeSupport == bmdDisplayModeNotSupported)
			throw std::exception("DecklinkOutput: display mode not supported");
		if (decklink_keyer_)
		{
			CComQIPtr<IDeckLinkAttributes> attributes_(output_);
			BOOL support = FALSE;
			if (SUCCEEDED(attributes_->GetFlag(BMDDeckLinkAttributeID::BMDDeckLinkSupportsInternalKeying, &support)) && support)
				decklink_keyer_->Enable(FALSE);
			if (support)
				decklink_keyer_->SetLevel(255);
		}
		if (FAILED(output_->EnableVideoOutput(format, BMDVideoOutputFlags::bmdVideoOutputFlagDefault)))
			throw std::exception("EnableVideoOutput failed");
	}

	HRESULT __stdcall DecklinkTimecodeDecoder::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents notificationEvents, IDeckLinkDisplayMode* newDisplayMode, BMDDetectedVideoInputFormatFlags detectedSignalFlags)
	{
		if (bmdVideoInputDisplayModeChanged && notificationEvents)
		{
			CloseInput();
			output_->DisableVideoOutput();
			OpenOutput(newDisplayMode->GetDisplayMode());
			OpenInput(newDisplayMode->GetDisplayMode());
		}
		return S_OK;
	}

	HRESULT __stdcall DecklinkTimecodeDecoder::VideoInputFrameArrived(IDeckLinkVideoInputFrame* input_frame, IDeckLinkAudioInputPacket* audioPacket)
	{
		CComPtr<IDeckLinkMutableVideoFrame> frame;
		if (SUCCEEDED(output_->CreateVideoFrame(current_mode_->GetWidth(), current_mode_->GetHeight(), current_mode_->GetWidth() * 4, BMDPixelFormat::bmdFormat8BitBGRA, bmdFrameFlagDefault, &frame)))
		{
			Draw(frame, input_frame);
			output_->DisplayVideoFrameSync(frame);
		}
		return S_OK;
	}


}