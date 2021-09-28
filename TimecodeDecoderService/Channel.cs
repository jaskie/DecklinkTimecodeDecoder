using System;
using System.Linq;
using System.Xml.Serialization;

namespace TimecodeDecoderService
{
    public enum Keyer
    {
        Internal,
        Passthrough
    }

    [XmlType("channel")]
    public class Channel: IDisposable
    {
        private TimecodeDecoder.TimecodeDecoder _timecodeDecoder;

        [XmlAttribute]
        public int Input { get; set; }

        [XmlAttribute]
        public int Output { get; set; }

        [XmlAttribute]
        public Keyer Keyer { get; set; }

        [XmlAttribute]
        public TimecodeDecoder.VideoFormat VideoFormat { get; set; }

        [XmlAttribute]
        public TimecodeDecoder.DecklinkTimecodeSource TcSource { get; set; }

        public void Dispose()
        {
            _timecodeDecoder?.Dispose();
            _timecodeDecoder = null;
        }

        public void Start()
        {
            _timecodeDecoder?.Dispose();
            _timecodeDecoder = new TimecodeDecoder.TimecodeDecoder(Input, Output, VideoFormat, TcSource);
        }
    }
}