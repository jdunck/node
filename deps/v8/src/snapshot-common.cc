// Copyright 2006-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// The common functionality when building with or without snapshots.

#include "v8.h"

#include "api.h"
#include "serialize.h"
#include "snapshot.h"
#include "platform.h"

namespace v8 {
namespace internal {

bool Snapshot::Deserialize(const byte* content, int len) {
  Deserializer des(content, len);
  des.GetFlags();
  return V8::Initialize(&des);
}


bool Snapshot::Deserialize2(const byte* content, int len) {
  SnapshotByteSource source(content, len);
  Deserializer2 deserializer(&source);
  return V8::Initialize(&deserializer);
}


bool Snapshot::Initialize(const char* snapshot_file) {
  if (snapshot_file) {
    int len;
    byte* str = ReadBytes(snapshot_file, &len);
    if (!str) return false;
    bool result = Deserialize(str, len);
    DeleteArray(str);
    return result;
  } else if (size_ > 0) {
    return Deserialize(data_, size_);
  }
  return false;
}


bool Snapshot::Initialize2(const char* snapshot_file) {
  if (snapshot_file) {
    int len;
    byte* str = ReadBytes(snapshot_file, &len);
    if (!str) return false;
    Deserialize2(str, len);
    DeleteArray(str);
  } else if (size_ > 0) {
    Deserialize2(data_, size_);
  }
  return true;
}


bool Snapshot::WriteToFile(const char* snapshot_file) {
  Serializer ser;
  ser.Serialize();
  byte* str;
  int len;
  ser.Finalize(&str, &len);

  int written = WriteBytes(snapshot_file, str, len);

  DeleteArray(str);
  return written == len;
}


class FileByteSink : public SnapshotByteSink {
 public:
  explicit FileByteSink(const char* snapshot_file) {
    fp_ = OS::FOpen(snapshot_file, "wb");
    if (fp_ == NULL) {
      PrintF("Unable to write to snapshot file \"%s\"\n", snapshot_file);
      exit(1);
    }
  }
  virtual ~FileByteSink() {
    if (fp_ != NULL) {
      fclose(fp_);
    }
  }
  virtual void Put(int byte, const char* description) {
    if (fp_ != NULL) {
      fputc(byte, fp_);
    }
  }

 private:
  FILE* fp_;
};


bool Snapshot::WriteToFile2(const char* snapshot_file) {
  FileByteSink file(snapshot_file);
  Serializer2 ser(&file);
  ser.Serialize();
  return true;
}



} }  // namespace v8::internal
