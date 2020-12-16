// Copyright 2015 Victor Maraccini
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
#ifndef KEYCZAR_RW_KEYSET_IN_MEMORY_READER_H_
#define KEYCZAR_RW_KEYSET_IN_MEMORY_READER_H_

#include <string>

#include <keyczar/base/basictypes.h>
#include <keyczar/base/stl_util-inl.h>
#include <keyczar/base/values.h>
#include <keyczar/rw/keyset_reader.h>

namespace keyczar {
namespace rw {

// A class reading key sets from base64 encoded JSON strings.
class KeysetMemoryReader : public KeysetReader {
 public:
  // Initializes the reader with the Base64-encoded JSON data of Meta and Key.
  // Only 1 key is available in this reader.
  explicit KeysetMemoryReader(const std::string& base64Meta, const std::string& base64Key);

  // Read the metadata. The caller takes ownership of the returned value.
  virtual Value* ReadMetadata() const;

  // Read the key. The caller takes ownership of the returned value. Version is ignored.
  virtual Value* ReadKey(int version) const;

 private:
  std::string encodedMeta;
  std::string encodedKey;

  DISALLOW_COPY_AND_ASSIGN(KeysetMemoryReader);
};

}  // namespace rw
}  // namespace keyczar

#endif  // KEYCZAR_RW_KEYSET_IN_MEMORY_READER_H_
