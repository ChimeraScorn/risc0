// Copyright 2022 Risc0, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "risc0/core/align.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace risc0 {

template <typename T> struct Encoder {
  template <typename Archive> static void transfer(Archive& ar, const T& value) {
    const_cast<T&>(value).transfer(ar);
  }
};

template <typename T> struct Decoder {
  template <typename Archive> static void transfer(Archive& ar, T& value) { value.transfer(ar); }
};

template <typename Stream> class ArchiveWriter {
public:
  ArchiveWriter(Stream& stream) : stream(stream) {}

  template <typename T> void transfer(const T& value) { Encoder<T>::transfer(*this, value); }

  void transfer(bool value) { stream.write_word(value); }

  void transfer(int8_t value) { stream.write_word(value); }
  void transfer(uint8_t value) { stream.write_word(value); }

  void transfer(int16_t value) { stream.write_word(value); }
  void transfer(uint16_t value) { stream.write_word(value); }

  void transfer(int32_t value) { stream.write_word(value); }
  void transfer(uint32_t value) { stream.write_word(value); }

  void transfer(int64_t value) { stream.write_dword(value); }
  void transfer(uint64_t value) { stream.write_dword(value); }

  void transfer(const void* buf, size_t len) { stream.write_buffer(buf, len); }

private:
  Stream& stream;
};

template <typename Stream> class ArchiveReader {
public:
  ArchiveReader(Stream& stream) : stream(stream) {}

  template <typename T> void transfer(T& value) { Decoder<T>::transfer(*this, value); }

  void transfer(bool& value) { value = stream.read_word(); }

  void transfer(int8_t& value) { value = stream.read_word(); }
  void transfer(uint8_t& value) { value = stream.read_word(); }

  void transfer(int16_t& value) { value = stream.read_word(); }
  void transfer(uint16_t& value) { value = stream.read_word(); }

  void transfer(int32_t& value) { value = stream.read_word(); }
  void transfer(uint32_t& value) { value = stream.read_word(); }

  void transfer(int64_t& value) { value = stream.read_dword(); }
  void transfer(uint64_t& value) { value = stream.read_dword(); }

  void transfer(void* buf, size_t len) { stream.read_buffer(buf, len); }

private:
  Stream& stream;
};

template <typename T> struct Encoder<std::vector<T>> {
  template <typename Archive> static void transfer(Archive& ar, const std::vector<T>& container) {
    ar.transfer(static_cast<uint32_t>(container.size()));
    for (const auto& item : container) {
      ar.transfer(item);
    }
  }
};

template <typename T> struct Decoder<std::vector<T>> {
  template <typename Archive> static void transfer(Archive& ar, std::vector<T>& container) {
    uint32_t size;
    ar.transfer(size);
    container.reserve(size);
    container.clear();
    for (size_t i = 0; i < size; i++) {
      T item;
      ar.transfer(item);
      container.push_back(item);
    }
  }
};

template <typename CharT> struct Encoder<std::basic_string<CharT>> {
  template <typename Archive>
  static void transfer(Archive& ar, const std::basic_string<CharT>& value) {
    ar.transfer(static_cast<uint32_t>(value.size()));
    ar.transfer(value.data(), value.size() * sizeof(CharT));
  }
};

template <typename CharT> struct Decoder<std::basic_string<CharT>> {
  template <typename Archive> static void transfer(Archive& ar, std::basic_string<CharT>& value) {
    uint32_t size;
    ar.transfer(size);
    value.resize(size);
    ar.transfer(value.data(), size * sizeof(CharT));
  }
};

struct BufferStreamWriter {
  BufferStreamWriter(uint32_t* ptr) : ptr(ptr) {}

  void write_word(uint32_t word) { *ptr++ = word; }

  void write_dword(uint64_t dword) {
    *ptr++ = dword & 0xffffffff;
    *ptr++ = dword >> 32;
  }

  void write_buffer(const void* buf, size_t len) {
    uint8_t* ptr_u8 = reinterpret_cast<uint8_t*>(ptr);
    uint32_t* last = reinterpret_cast<uint32_t*>(align(ptr_u8 + len)) - 1;
    *last = 0;
    memcpy(ptr, buf, len);
    ptr = ++last;
  }

  uint32_t* ptr;
};

struct BufferStreamReader {
  BufferStreamReader(uint32_t* ptr) : ptr(ptr) {}

  uint32_t read_word() { return *ptr++; }

  uint64_t read_dword() {
    uint32_t low = *ptr++;
    uint32_t high = *ptr++;
    return static_cast<uint64_t>(low) | static_cast<uint64_t>(high) << 32;
  }

  void read_buffer(void* buf, size_t len) {
    memcpy(buf, ptr, len);
    ptr += align(len) / sizeof(uint32_t);
  }

  uint32_t* ptr;
};

struct VectorStreamWriter {
  void write_word(uint32_t word) { vec.push_back(word); }

  void write_dword(uint64_t dword) {
    vec.push_back(dword & 0xffffffff);
    vec.push_back(dword >> 32);
  }

  void write_buffer(const void* buf, size_t len) {
    size_t aligned = align(len);
    auto tmp = std::make_unique<uint8_t[]>(aligned);
    uint32_t* ptr = reinterpret_cast<uint32_t*>(tmp.get());
    memcpy(tmp.get(), buf, len);
    for (size_t i = 0; i < aligned / 4; i++) {
      vec.push_back(*ptr++);
    }
  }

  std::vector<uint32_t> vec;
};

} // namespace risc0
