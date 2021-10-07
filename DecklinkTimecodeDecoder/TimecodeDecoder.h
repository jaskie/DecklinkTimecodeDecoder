#pragma once
namespace TimecodeDecoder {
	enum class VideoFormat;
	enum class DecklinkTimecodeSource;
	enum class Keyer;
	class DecklinkTimecodeDecoder;

	public ref class TimecodeDecoder
	{
	public:
		TimecodeDecoder(int inputDecklinkIndex, int outputDecklinkIndex, VideoFormat format, DecklinkTimecodeSource timecodeSource, Keyer keyer);
		~TimecodeDecoder();
		!TimecodeDecoder();
	private:
		std::unique_ptr<DecklinkTimecodeDecoder>* decoder_native_;
	};

}
