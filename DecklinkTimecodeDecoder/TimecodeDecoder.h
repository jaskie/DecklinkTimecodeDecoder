#pragma once
namespace TimecodeDecoder {
	enum class VideoFormat;
	enum class DecklinkTimecodeSource;
	class DecklinkTimecodeDecoder;

	public ref class TimecodeDecoder
	{
	public:
		TimecodeDecoder(int inputDecklinkIndex, int outputDecklinkIndex, VideoFormat format, DecklinkTimecodeSource timecodeSource);
		~TimecodeDecoder();
		!TimecodeDecoder();
	private:
		std::unique_ptr<DecklinkTimecodeDecoder>* decoder_native_;
	};

}
