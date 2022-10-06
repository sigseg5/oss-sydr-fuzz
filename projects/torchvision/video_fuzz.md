# Fix linker err pytorch
* `rm /usr/bin/x86_64-linux-gnu-gcc`
* `ln /usr/bin/clang /usr/bin/x86_64-linux-gnu-gcc`

# Install ffmpeg
```
apt update && apt upgrade -y && \
apt-get install -y software-properties-common && \
add-apt-repository ppa:savoury1/ffmpeg4 -y && add-apt-repository ppa:savoury1/ffmpeg5 -y && \
apt update -y && apt upgrade -y && apt install -y ffmpeg libavformat-dev libswscale-dev
```

`apt install libavcodec-dev`

# PyTorch:
* `git clone https://github.com/pytorch/pytorch.git /pytorch_fuzz/`
* `git checkout 664058fa83f1d8eede5d66418abff6e20bd76ca8`
* `git submodule update --init --recursive --jobs 0`
* Build:

```
MAX_JOBS=$(nproc) USE_FBGEMM=0 BUILD_BINARY=1 CC=clang CXX=clang++ USE_STATIC_MKL=1 \
        USE_DISTRIBUTED=0 USE_MPI=0 BUILD_CAFFE2_OPS=0 BUILD_CAFFE2=0 BUILD_TEST=0 \
        BUILD_SHARED_LIBS=OFF USE_OPENMP=0 USE_MKLDNN=0 \
        CXXFLAGS='-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero' \
        CFLAGS='-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero' \
        python3 setup.py build
```

## Patch defines:

```
sed -i '1 i\#define ORDERED_DICT' /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/ordered_dict.h
sed -i '1 i\#ifndef ORDERED_DICT' /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/ordered_dict.h
echo "#endif" >> /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/ordered_dict.h

sed -i '1 i\#define ORDERED_DICT' /pytorch_fuzz/torch/csrc/api/include/torch/ordered_dict.h
sed -i '1 i\#ifndef ORDERED_DICT' /pytorch_fuzz/torch/csrc/api/include/torch/ordered_dict.h
echo "#endif" >> /pytorch_fuzz/torch/csrc/api/include/torch/ordered_dict.h


sed -i '1 i\#define TYPES' /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/types.h
sed -i '1 i\#ifndef TYPES' /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/types.h
echo "#endif" >> /pytorch_fuzz/torch/include/torch/csrc/api/include/torch/types.h

sed -i '1 i\#define TYPES' /pytorch_fuzz/torch/csrc/api/include/torch/types.h
sed -i '1 i\#ifndef TYPES' /pytorch_fuzz/torch/csrc/api/include/torch/types.h
echo "#endif" >> /pytorch_fuzz/torch/csrc/api/include/torch/types.h
```

# vision:
* `git clone https://github.com/pytorch/vision.git /vision_fuzz-ffmpeg`
* `git checkout 0.13.1 bddbd7e6d65ecacc2e40cf6c9e2059669b8dbd44`
* Patch STATIC's:
```
sed -i 's/add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})/add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})/' /vision_fuzz-ffmpeg/CMakeLists.txt && \
    sed -i 's/target_link_libraries(${PROJECT_NAME} PRIVATE ${TORCH_LIBRARIES} ${PNG_LIBRARY} ${JPEG_LIBRARIES})/add_definitions(-DJPEG_FOUND -DPNG_FOUND)/' /vision_fuzz-ffmpeg/CMakeLists.txt
```

* Build:
```
Torch_DIR=/pytorch_fuzz/ cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_C_FLAGS="-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero" \
        -DCMAKE_CXX_FLAGS="-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero -I/pytorch_fuzz/torch/csrc/api/include -I/pytorch_fuzz/torch/include -I/libjpeg-turbo-2.1.3-fuzz/" \
        -S . -B build/
```
* `cd build`
* `cmake --build . -j$(nproc)`

## Alt build with CMake
* Build:
```
TORCHVISION_USE_FFMPEG=1 Torch_DIR=/pytorch_fuzz/ cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS="-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero" -DCMAKE_CXX_FLAGS="-g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero -I/pytorch_fuzz/torch/csrc/api/include -I/pytorch_fuzz/torch/include -I/libjpeg-turbo-2.1.3-fuzz/" -S . -B build/
```
* `cd build`
* `cmake --build . -j$(nproc)`

# Build custom ffmpeg
* Install dep's:
```
apt-get update -qq && apt-get -y install \
  autoconf \
  automake \
  build-essential \
  cmake \
  git-core \
  libass-dev \
  libfreetype6-dev \
  libgnutls28-dev \
  libmp3lame-dev \
  libsdl2-dev \
  libtool \
  libva-dev \
  libvdpau-dev \
  libvorbis-dev \
  libxcb1-dev \
  libxcb-shm0-dev \
  libxcb-xfixes0-dev \
  meson \
  ninja-build \
  pkg-config \
  texinfo \
  wget \
  yasm \
  zlib1g-dev \
  nasm \
  gnutls-bin libunistring-dev 
```

* `mkdir -p /ffmpeg_sources /bin`

* `wget -O ffmpeg-snapshot.tar.bz2 https://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2`
* `tar xjvf ffmpeg-snapshot.tar.bz2`
* `cd ffmpeg`

Change `cc` and `cxx` to `clang` and `clang++` in `configure` (3 times)

* Build:
```
PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure \
  --prefix="$HOME/ffmpeg_build" \
  --pkg-config-flags="--static" \
  --extra-cflags="-I$HOME/ffmpeg_build/include -g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero" \
  --extra-ldflags="-L$HOME/ffmpeg_build/lib -g -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero" \
  --extra-libs="-lpthread -lm" \
  --ld="clang++" \
  --bindir="$HOME/bin" \
  --enable-gpl \
  --enable-gnutls 
```

* `PATH="$HOME/bin:$PATH" make -j50`

# Build video target
```
clang++ -std=c++14 -DAT_PER_OPERATOR_HEADERS -DCPUINFO_SUPPORTED_PLATFORM=1 -DFMT_HEADER_ONLY=1 \
    -DFXDIV_USE_INLINE_ASSEMBLY=0 -DHAVE_MALLOC_USABLE_SIZE=1 -DHAVE_MMAP=1 -DHAVE_SHM_OPEN=1 \
    -DHAVE_SHM_UNLINK=1 -DMINIZ_DISABLE_ZIP_READER_CRC32_CHECKS -DNNP_CONVOLUTION_ONLY=0 -DNNP_INFERENCE_ONLY=0 \
    -DONNXIFI_ENABLE_EXT=1 -DONNX_ML=1 -DONNX_NAMESPACE=onnx_torch -DUSE_EXTERNAL_MZCRC -D_FILE_OFFSET_BITS=64 \
    -DUSE_PTHREADPOOL -DNDEBUG -DUSE_KINETO -DLIBKINETO_NOCUPTI -DUSE_QNNPACK -DUSE_PYTORCH_QNNPACK \
    -DUSE_XNNPACK -DSYMBOLICATE_MOBILE_DEBUG_HANDLE -DEDGE_PROFILER_USE_KINETO -DTH_HAVE_THREAD \
    -g -O2 \
    -fsanitize=fuzzer,address,bounds,integer,undefined,null,float-divide-by-zero \
    -I/pytorch_fuzz/torch/include /video_reader_fuzz.cc \
    -I/pytorch_fuzz/torch/csrc/api/include \
    -I/vision_fuzz-ffmpeg/torchvision/csrc/io/video_reader/ -c \
    -o ./video_reader_fuzz.o
```

# Link video target (wip)

```
clang++ -pthread -static -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -fsanitize=fuzzer-no-link,address,bounds,integer,undefined,null,float-divide-by-zero -O2 -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/video_reader/video_reader.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/video_sampler.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/memory_buffer.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/subtitle_stream.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/subtitle_sampler.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/seekable_buffer.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/video_stream.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/time_keeper.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/decoder.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/util.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/audio_stream.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/cc_stream.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/audio_sampler.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/stream.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/decoder/sync_decoder.o /vision_fuzz-tmp/build/temp.linux-x86_64-3.8/vision_fuzz-tmp/torchvision/csrc/io/video/video.o \
    -Wl,--whole-archive,"/pytorch_fuzz/torch/lib/libtorch.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/libavcodec/libavcodec.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libavutil/libavutil.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libswresample/libswresample.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libpostproc/libpostproc.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libswscale/libswscale.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libavfilter/libavfilter.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/root/ffmpeg_sources/ffmpeg/./libavdevice/libavdevice.a" -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/vision_fuzz/build/libtorchvision.a" -Wl,--no-whole-archive \
    /pytorch_fuzz/build/lib/libtorch.a \
    /pytorch_fuzz/build/lib/libtorch_cpu.a \
    /vision_fuzz/build/libtorchvision.a /pytorch_fuzz/build/lib/libbreakpad.a \
    /pytorch_fuzz/build/lib/libbreakpad_common.a \
    -Wl,--whole-archive,"/pytorch_fuzz/build/lib/libcaffe2_protos.a" -Wl,--no-whole-archive \
    /pytorch_fuzz/torch/lib/libqnnpack.a /pytorch_fuzz/torch/lib/libpytorch_qnnpack.a \
    /pytorch_fuzz/torch/lib/libnnpack.a /pytorch_fuzz/torch/lib/libXNNPACK.a \
    /pytorch_fuzz/torch/lib/libpthreadpool.a /pytorch_fuzz/torch/lib/libcpuinfo.a \
    /pytorch_fuzz/torch/lib/libclog.a /pytorch_fuzz/build/lib/libfoxi_loader.a \
    -lrt -lm \
    /pytorch_fuzz/torch/lib/libkineto.a /pytorch_fuzz/build/sleef/lib/libsleef.a \
    -Wl,--whole-archive,"/pytorch_fuzz/build/lib/libonnx.a" -Wl,--no-whole-archive \
    /pytorch_fuzz/build/lib/libonnx_proto.a /pytorch_fuzz/torch/lib/libprotobuf.a \
    -pthread \
    -Wl,--whole-archive,"/pytorch_fuzz/build/lib/libCaffe2_perfkernels_avx.a" \
    -Wl,--no-whole-archive \
    -Wl,--whole-archive,"/pytorch_fuzz/build/lib/libCaffe2_perfkernels_avx2.a" \
    -Wl,--no-whole-archive \
    /pytorch_fuzz/torch/lib/libc10.a \
    -Wl,--whole-archive,"/pytorch_fuzz/build/lib/libCaffe2_perfkernels_avx512.a" \
    -Wl,--no-whole-archive \
    /libjpeg-turbo-2.1.3-fuzz/build/libturbojpeg.a \
    /libpng-1.6.37-fuzz/./.libs/libpng16.a \
    -o build/lib.linux-x86_64-3.8/torchvision/video_reader.a -std=c++14 
```

# Build shared
* `pip3 install torch`
* Build torchvision without instr
```
TORCHVISION_USE_FFMPEG=1 Torch_DIR=/pytorch_fuzz/ python setup.py
```
* Build video fuzz target
```
clang++ -std=c++14 -fPIC -DAT_PER_OPERATOR_HEADERS -DCPUINFO_SUPPORTED_PLATFORM=1 -DFMT_HEADER_ONLY=1 \
    -DFXDIV_USE_INLINE_ASSEMBLY=0 -DHAVE_MALLOC_USABLE_SIZE=1 -DHAVE_MMAP=1 -DHAVE_SHM_OPEN=1 \
    -DHAVE_SHM_UNLINK=1 -DMINIZ_DISABLE_ZIP_READER_CRC32_CHECKS -DNNP_CONVOLUTION_ONLY=0 -DNNP_INFERENCE_ONLY=0 \
    -DONNXIFI_ENABLE_EXT=1 -DONNX_ML=1 -DONNX_NAMESPACE=onnx_torch -DUSE_EXTERNAL_MZCRC -D_FILE_OFFSET_BITS=64 \
    -DUSE_PTHREADPOOL -DNDEBUG -DUSE_KINETO -DLIBKINETO_NOCUPTI -DUSE_QNNPACK -DUSE_PYTORCH_QNNPACK \
    -DUSE_XNNPACK -DSYMBOLICATE_MOBILE_DEBUG_HANDLE -DEDGE_PROFILER_USE_KINETO -DTH_HAVE_THREAD \
    -g -O2 \
    -I/pytorch_fuzz/torch/include /video_reader_fuzz.cc \
    -I/pytorch_fuzz/torch/csrc/api/include \
    -I/vision_fuzz-ffmpeg/torchvision/csrc/io/video_reader/ -c \
    -o ./video_reader_fuzz.o
```

* Link video fuzz target
```
clang++ -g -O2 -std=gnu++14 -DNDEBUG \
    -pthread -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -O2 -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fwrapv -O2 -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 \
    ./video_reader.so -lrt -lm \
    -Wl,--whole-archive,"/usr/local/lib/python3.8/dist-packages/torch/lib/libtorch_python.so" -Wl,--no-whole-archive \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/video_sampler.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/memory_buffer.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/subtitle_stream.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/subtitle_sampler.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/seekable_buffer.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/video_stream.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/time_keeper.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/decoder.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/util.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/audio_stream.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/cc_stream.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/audio_sampler.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/stream.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/decoder/sync_decoder.o \
    /vision_fuzz-ffmpeg/build/temp.linux-x86_64-3.8/vision_fuzz-ffmpeg/torchvision/csrc/io/video/video.o \
    -L/usr/local/lib/python3.8/dist-packages/torch/lib -lavcodec -lavformat -lavutil -lswresample -lswscale -lc10 -ltorch -ltorch_cpu -ltorch_python \
    -std=c++14 \
    -o /test
    
```