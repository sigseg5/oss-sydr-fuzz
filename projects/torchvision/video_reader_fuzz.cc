#include "video_reader.h"
#include "../decoder/memory_buffer.h"
#include "../decoder/sync_decoder.h"

#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <torch/types.h>
#include <unistd.h>

using namespace ffmpeg;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char name[] = "/tmp/torch-fuzz-XXXXXX";
  char *dir = mktemp(name);
  std::ofstream fp;
  fp.open(dir, std::ios::out | std::ios::binary);
  fp.write((char *)data, size);
  fp.close();

  if (size <= 0) {
    unlink(dir);
    return 0;
  }
  try {
    torch::List<torch::Tensor> out_tensor = vision::video_reader::probe_video_from_file(dir);
  } catch (const c10::Error &e) {
    std::string err = e.what();
    unlink(dir);
    abort();
  }

  unlink(dir);

  return 0;
}
