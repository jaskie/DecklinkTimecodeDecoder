#include "pch.h"
#include "TimecodeDecoder.h"
#include "VideoFormat.h"
#include "DecklinkTimecodeSource.h"
#include "DecklinkTimecodeDecoder.h"

namespace TimecodeDecoder {

	BMDDisplayMode VideoFormatToBMDDisplayMode(VideoFormat format)
	{
		switch (format)
		{
		case VideoFormat::PalFHA:
			return BMDDisplayMode::bmdModePAL;
		case VideoFormat::HD1080i50:
			return BMDDisplayMode::bmdModeHD1080i50;
		case VideoFormat::HD1080p50:
			return BMDDisplayMode::bmdModeHD1080p50;
		default:
			break;
		}
		throw gcnew System::InvalidOperationException("Invalid video mode");
	}

	BMDTimecodeFormat DecklinkTimemecodeSourceToBMDTimecodeFormat(DecklinkTimecodeSource timecodeSource)
	{
		switch (timecodeSource)
		{
		case DecklinkTimecodeSource::RP188Any:
			return BMDTimecodeFormat::bmdTimecodeRP188Any;
		case DecklinkTimecodeSource::VITC:
			return BMDTimecodeFormat::bmdTimecodeVITC;
		case DecklinkTimecodeSource::VITC2:
			return BMDTimecodeFormat::bmdTimecodeVITCField2;
		default:
			throw gcnew System::InvalidOperationException("Invalid timecode source specified");
		}
	}



	TimecodeDecoder::TimecodeDecoder(int inputDecklinkIndex, int outputDecklinkIndex, VideoFormat format, DecklinkTimecodeSource timecodeSource, Keyer keyer)
		: decoder_native_(new std::unique_ptr<DecklinkTimecodeDecoder>(new DecklinkTimecodeDecoder(inputDecklinkIndex, outputDecklinkIndex, VideoFormatToBMDDisplayMode(format), DecklinkTimemecodeSourceToBMDTimecodeFormat(timecodeSource), keyer)))
	{ }

	TimecodeDecoder::~TimecodeDecoder()
	{
		this->!TimecodeDecoder();
	}
	TimecodeDecoder::!TimecodeDecoder()
	{
		if (!decoder_native_)
			return;
		decoder_native_->reset();
		decoder_native_ = nullptr;
	}
}