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

#include "risc0/zkp/prove/c_api.h"

#include <memory>

#include "risc0/core/log.h"
#include "risc0/zkp/prove/proof.h"
#include "risc0/zkp/verify/verify.h"

extern "C" {

struct risc0_string {
  std::string str;
};

} // extern "C"

template <typename F> void ffi_wrap_void(risc0_error* err, F fn) {
  try {
    err->msg = nullptr;
    fn();
  } catch (const std::exception& ex) {
    err->msg = new risc0_string{ex.what()};
  } catch (...) {
    err->msg = new risc0_string{"C++ exception"};
  }
}

template <typename T, typename F> T ffi_wrap(risc0_error* err, T val, F fn) {
  try {
    err->msg = nullptr;
    return fn();
  } catch (const std::exception& ex) {
    err->msg = new risc0_string{ex.what()};
    return val;
  } catch (...) {
    err->msg = new risc0_string{"C++ exception"};
    return val;
  }
}

extern "C" {

struct risc0_prover {
  std::unique_ptr<risc0::Prover> prover;
};

struct risc0_proof {
  risc0::Proof proof;
};

void risc0_init() {
  if (const char* level = std::getenv("RISC0_LOG")) {
    risc0::setLogLevel(std::atoi(level));
  }
}

const char* risc0_string_ptr(risc0_string* str) {
  return str->str.c_str();
}

void risc0_string_free(risc0_string* str) {
  risc0_error err;
  ffi_wrap_void(&err, [&] { delete str; });
}

risc0_prover* risc0_prover_new(risc0_error* err, const char* elf_path) {
  return ffi_wrap<risc0_prover*>(
      err, nullptr, [&] { return new risc0_prover{std::make_unique<risc0::Prover>(elf_path)}; });
}

void risc0_prover_free(risc0_error* err, risc0_prover* ptr) {
  ffi_wrap_void(err, [&] { delete ptr; });
}

void risc0_prover_add_input(risc0_error* err, risc0_prover* ptr, const uint8_t* buf, size_t len) {
  ffi_wrap_void(err, [&] { ptr->prover->writeInput(buf, len); });
}

const void* risc0_prover_get_output_buf(risc0_error* err, risc0_prover* ptr) {
  return ffi_wrap<const void*>(err, nullptr, [&] { return ptr->prover->getOutput().data(); });
}

size_t risc0_prover_get_output_len(risc0_error* err, const risc0_prover* ptr) {
  return ffi_wrap(err, 0, [&] { return ptr->prover->getOutput().size(); });
}

risc0_proof* risc0_prover_run(risc0_error* err, risc0_prover* ptr) {
  return ffi_wrap<risc0_proof*>(err, nullptr, [&] {
    risc0::Proof proof = ptr->prover->run();
    return new risc0_proof{proof};
  });
}

void risc0_proof_verify(risc0_error* err, const char* elf_path, const risc0_proof* ptr) {
  ffi_wrap_void(err, [&] { ptr->proof.verify(elf_path); });
}

const uint32_t* risc0_proof_get_core_buf(risc0_error* err, const risc0_proof* ptr) {
  return ffi_wrap<const uint32_t*>(err, nullptr, [&] { return ptr->proof.core.data(); });
}

size_t risc0_proof_get_core_len(risc0_error* err, const risc0_proof* ptr) {
  return ffi_wrap(err, 0, [&] { return ptr->proof.core.size(); });
}

const void* risc0_proof_get_message_buf(risc0_error* err, const risc0_proof* ptr) {
  return ffi_wrap<const void*>(err, nullptr, [&] { return ptr->proof.getMessage().data(); });
}

size_t risc0_proof_get_message_len(risc0_error* err, const risc0_proof* ptr) {
  return ffi_wrap(err, 0, [&] { return ptr->proof.getMessage().size(); });
}

void risc0_proof_free(risc0_error* err, const risc0_proof* ptr) {
  ffi_wrap_void(err, [&] { delete ptr; });
}

} // extern "C"
