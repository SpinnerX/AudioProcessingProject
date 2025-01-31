#include <formatCpp/formatCpp.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>
#include <cstring>

/**
 * @brief 
 *
 * Shows how to list directories through the libavformat AVIOContext API.
 * 
 */

// Help with our includes for ffmpeg
extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/mem.h> // For av_packet_alloc()
    #include <libavutil/imgutils.h> // This is to include av_get_pix_fmt_name()
    #include <libavutil/parseutils.h> // Including this to allow us to call the function av_parse_video_size()
    #include <libavutil/channel_layout.h>
    #include <libavutil/pixdesc.h>
    #include <libavfilter/avfilter.h> // This is how we include functions like avfilter_inout_alloc()
    #include <libavutil/opt.h> // To include AV_OPT_SEARCH_CHILDREN
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
    #include <inttypes.h>
};

// decode packets into frames
/*static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);

// save a frame into a .pgm file
static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);

int main(int argc, const char *argv[])
{
  if (argc < 2) {
    printf("You need to specify a media file.\n");
    return -1;
  }
  
  formatCpp::print("initializing all the containers, codecs and protocols.\n");

  // AVFormatContext holds the header information from the format (Container)
  // Allocating memory for this component
  // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  AVFormatContext *pFormatContext = avformat_alloc_context();
  if (!pFormatContext) {
    formatCpp::print("ERROR could not allocate memory for Format Context\n");
    return -1;
  }

  formatCpp::print("opening the input file ({}) and loading format (container) header\n", argv[1]);
  // Open the file and read its header. The codecs are not opened.
  // The function arguments are:
  // AVFormatContext (the component we allocated memory for),
  // url (filename),
  // AVInputFormat (if you pass NULL it'll do the auto detect)
  // and AVDictionary (which are options to the demuxer)
  // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
  if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0) {
    formatCpp::print("ERROR could not open the file\n");
    return -1;
  }

  // now we have access to some information about our file
  // since we read its header we can say what format (container) it's
  // and some other information related to the format itself.
  formatCpp::print("format {}, duration {} us, bit_rate {}\n", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);

  formatCpp::print("finding stream info from format\n");
  // read Packets from the Format to get stream information
  // this function populates pFormatContext->streams
  // (of size equals to pFormatContext->nb_streams)
  // the arguments are:
  // the AVFormatContext
  // and options contains options for codec corresponding to i-th stream.
  // On return each dictionary will be filled with options that were not found.
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
  if (avformat_find_stream_info(pFormatContext,  NULL) < 0) {
    formatCpp::print("ERROR could not get the stream info\n");
    return -1;
  }

  // the component that knows how to enCOde and DECode the stream
  // it's the codec (audio or video)
  // http://ffmpeg.org/doxygen/trunk/structAVCodec.html
  const AVCodec *pCodec = NULL;
  // this component describes the properties of a codec used by the stream i
  // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
  AVCodecParameters *pCodecParameters =  NULL;
  int video_stream_index = -1;

  // loop though all the streams and print its main information
  for (int i = 0; i < pFormatContext->nb_streams; i++)
  {
    AVCodecParameters *pLocalCodecParameters =  NULL;
    pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
    formatCpp::print("AVStream->time_base before open coded {}/{}\n", pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
    formatCpp::print("AVStream->r_frame_rate before open coded {}/{}\n", pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
    formatCpp::print("AVStream->start_time %\n" PRId64, pFormatContext->streams[i]->start_time);
    formatCpp::print("AVStream->duration %\n" PRId64, pFormatContext->streams[i]->duration);

    formatCpp::print("finding the proper decoder (CODEC)");

    // AVCodec *pLocalCodec = NULL;

    // finds the registered decoder for a codec ID
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
    const AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

    if (pLocalCodec==NULL) {
      formatCpp::print("ERROR unsupported codec!\n");
      // In this example if the codec is not found we just skip it
      continue;
    }

    // when the stream is a video we store its index, codec parameters and codec
    if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
      if (video_stream_index == -1) {
        video_stream_index = i;
        pCodec = pLocalCodec;
        pCodecParameters = pLocalCodecParameters;
      }

      formatCpp::print("Video Codec: resolution {} x {}\n", pLocalCodecParameters->width, pLocalCodecParameters->height);
    } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
      formatCpp::print("Audio Codec: {} channels, sample rate {}\n", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
    }

    // print its name, id and bitrate
    formatCpp::print("\tCodec {} ID {} bit_rate {}\n", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
  }

  if (video_stream_index == -1) {
    formatCpp::print("File {} does not contain a video stream!\n", argv[1]);
    return -1;
  }

  // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
  AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
  if (!pCodecContext)
  {
    formatCpp::print("failed to allocated memory for AVCodecContext\n");
    return -1;
  }

  // Fill the codec context based on the values from the supplied codec parameters
  // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
  if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
  {
    formatCpp::print("failed to copy codec params to codec context\n");
    return -1;
  }

  // Initialize the AVCodecContext to use the given AVCodec.
  // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
  if (avcodec_open2(pCodecContext, pCodec, NULL) < 0)
  {
    formatCpp::print("failed to open codec through avcodec_open2\n");
    return -1;
  }

  // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
  AVFrame *pFrame = av_frame_alloc();
  if (!pFrame)
  {
    formatCpp::print("failed to allocate memory for AVFrame\n");
    return -1;
  }
  // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
  AVPacket *pPacket = av_packet_alloc();
  if (!pPacket)
  {
    formatCpp::print("failed to allocate memory for AVPacket\n");
    return -1;
  }

  int response = 0;
  int how_many_packets_to_process = 8;

  // fill the Packet with data from the Stream
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
  while (av_read_frame(pFormatContext, pPacket) >= 0)
  {
    // if it's the video stream
    if (pPacket->stream_index == video_stream_index) {
    formatCpp::print("AVPacket->pts %\n" PRId64, pPacket->pts);
      response = decode_packet(pPacket, pCodecContext, pFrame);
      if (response < 0)
        break;
      // stop it, otherwise we'll be saving hundreds of frames
      if (--how_many_packets_to_process <= 0) break;
    }
    // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
    av_packet_unref(pPacket);
  }

  formatCpp::print("releasing all the resources\n");

  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
  return 0;
}

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame)
{
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0) {
    formatCpp::print("Error while sending a packet to the decoder: %s\n", av_err2str(response));
    return response;
  }

  while (response >= 0)
  {
    // Return decoded output data (into a frame) from a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      formatCpp::print("Error while receiving a frame from the decoder: {}\n", av_err2str(response));
      return response;
    }

    if (response >= 0) {
      formatCpp::print(
          "Frame {} (type={}, size={} bytes, format={}) pts {} key_frame {} [DTS {}]\n",
          pCodecContext->frame_number,
          av_get_picture_type_char(pFrame->pict_type),
          pFrame->pkt_size,
          pFrame->format,
          pFrame->pts,
          pFrame->key_frame,
          pFrame->coded_picture_number
      );

      char frame_filename[1024];
        snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
       formatCpp::print("{}-{}.pgm\n", frame_filename, pCodecContext->frame_number);
      // Check if the frame is a planar YUV 4:2:0, 12bpp
      // That is the format of the provided .mp4 file
      // RGB formats will definitely not give a gray image
      // Other YUV image may do so, but untested, so give a warning
      if (pFrame->format != AV_PIX_FMT_YUV420P)
      {
        formatCpp::print("Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB\n");
      }
      // save a grayscale frame into a .pgm file
      save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    }
  }
  return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}*/

int main(){
    AVFormatContext* context = nullptr;
    std::string filename = "../image0.jpg";

    if (avformat_alloc_output_context2(&context, nullptr, nullptr, filename.c_str()) < 0) {
        // std::cerr << "Error creating output format context" << std::endl;
        formatCpp::print("Error created #1\n");
        return -1;
    }

    const AVCodec* codecEncoder = avcodec_find_encoder(AV_CODEC_ID_H264);

    if(!codecEncoder){
        formatCpp::print("Error detected #2\n");
        return -1;
    }

    AVStream* stream = avformat_new_stream(context, codecEncoder);

    if(!stream){
        formatCpp::print("Error detected #3\n");
        return -1;
    }

    AVCodecParameters* params = stream->codecpar;
    params->codec_id = codecEncoder->id;
    params->codec_type = AVMEDIA_TYPE_VIDEO;
    params->width = 640;
    params->height = 480;

    // setting bitrate
    params->bit_rate = 1000000;

    AVCodecContext* avContext = avcodec_alloc_context3(codecEncoder);

    if(!avContext){
        formatCpp::print("Error detected #4\n");
        return -1;
    }

    AVFrame* frame = av_frame_alloc();

    frame->format = avContext->pix_fmt;
    frame->width = avContext->width;
    frame->height = avContext->height;

    if(av_frame_get_buffer(frame, 32) < 0){
        formatCpp::print("Error detected #5\n");
        return -1;
    }

}