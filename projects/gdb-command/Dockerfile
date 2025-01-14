# Copyright 2022 ISP RAS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
FROM sweetvishnya/ubuntu20.04-sydr-fuzz

MAINTAINER Andrey Fedotov
RUN apt-get update && \
    apt-get -y install curl

# Install nightly
RUN rustup install nightly

# Install cargo-fuzz
RUN cargo install cargo-fuzz

# Copy build script.
COPY build_fuzz.sh build_sydr.sh /

# Clone target from GitHub.
RUN git clone https://github.com/anfedotoff/gdb-command.git

WORKDIR /gdb-command

# Checkout specified commit. It could be updated later.
RUN git checkout f6f906d2c5b902e8ed7162aa515cac0d145f8f09

# Build fuzz targets.
RUN ../build_fuzz.sh

RUN rm -rf fuzz/target

COPY build-config/Cargo.toml fuzz/

COPY sydr_*.rs fuzz/fuzz_targets/

WORKDIR fuzz

# Build sydr targets.
RUN /build_sydr.sh

WORKDIR /
