#pragma once
#include "../extlib/zstd/zstd.h"
#include <cstdlib>
#include <streambuf>

namespace bgl {
class zstd_decode_filter_buf : public std::streambuf {
public:
  zstd_decode_filter_buf(std::streambuf* src)
      : source_buf_{src},
        buf_in_size_{ZSTD_DStreamInSize()},
        buf_out_size_{ZSTD_DStreamOutSize()},
        buf_in_{new char[buf_in_size_]},
        buf_out_{new char[buf_out_size_]},
        dstream_{ZSTD_createDStream()},
        input_{nullptr, 0, 0} {
    ASSERT(dstream_);
    to_read_ = ZSTD_initDStream(dstream_);
    ASSERT_MSG(!ZSTD_isError(to_read_), "ZSTD_initDStream: {}", ZSTD_getErrorName(to_read_));
    setg(buf_out_, buf_out_, buf_out_);
  }

  zstd_decode_filter_buf(const zstd_decode_filter_buf&) = delete;
  zstd_decode_filter_buf(zstd_decode_filter_buf&&) = delete;
  zstd_decode_filter_buf& operator=(const zstd_decode_filter_buf&) = delete;
  zstd_decode_filter_buf& operator=(zstd_decode_filter_buf&&) = delete;

  virtual ~zstd_decode_filter_buf() {
    delete[] buf_in_;
    delete[] buf_out_;
    ZSTD_freeDStream(dstream_);
  }

protected:
  virtual int_type underflow() {
    while (gptr() == egptr()) {
      if (input_.pos == input_.size) {
        if (to_read_ == 0) {
          // ensure EOF
          ASSERT_MSG(source_buf_->sgetc() == traits_type::eof(), "invalid zstd file");
          return traits_type::eof();
        }

        // read from source buffer
        std::streamsize read = source_buf_->sgetn(buf_in_, to_read_);
        ASSERT_MSG(static_cast<std::size_t>(read) == to_read_, "premature end of file");
        input_ = {buf_in_, to_read_, 0};
      }

      // decompression
      ZSTD_outBuffer output = {buf_out_, buf_out_size_, 0};
      to_read_ = ZSTD_decompressStream(dstream_, &output, &input_);
      ASSERT_MSG(!ZSTD_isError(to_read_), "ZSTD_decompressStream: {}", ZSTD_getErrorName(to_read_));

      setg(buf_out_, buf_out_, buf_out_ + output.pos);
    }

    return traits_type::to_int_type(*gptr());
  }

private:
  std::streambuf* const source_buf_;
  const std::size_t buf_in_size_;
  const std::size_t buf_out_size_;
  char* const buf_in_;
  char* const buf_out_;
  ZSTD_DStream* const dstream_;
  ZSTD_inBuffer input_;
  std::size_t to_read_;
};
}  // namespace bgl
