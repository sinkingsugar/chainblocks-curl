/* SPDX-License-Identifier: BSD 3-Clause "New" or "Revised" License */
/* Copyright © 2019 Giovanni Petrantoni */

#define STB_DS_IMPLEMENTATION 1

#include <curl/curl.h>
#include <dllblock.hpp>
#include <future>
#include <vector>

namespace chainblocks {
namespace Curl {
struct Globals {
  Globals() { curl_global_init(CURL_GLOBAL_ALL); }
  ~Globals() { curl_global_cleanup(); }
};

struct Common {
  static inline Globals globals{};

  static inline CBTypeInfo anyInfo{CBType::Any};
  static inline CBTypesInfo anyInfos = nullptr;

  static inline CBTypeInfo strInfo{CBType::String};
  static inline CBTypesInfo strInfos = nullptr;

  static inline CBTypeInfo bytesInfo{CBType::Bytes};
  static inline CBTypesInfo bytesInfos = nullptr;

  static void init() {
    stbds_arrpush(anyInfos, anyInfo);

    stbds_arrpush(strInfos, strInfo);

    stbds_arrpush(bytesInfos, bytesInfo);
  }
};

struct Get {
  CURL *_curl;
  bool _canceling;
  std::vector<uint8_t> _buffer;

  Get() { _curl = curl_easy_init(); }

  ~Get() { curl_easy_cleanup(_curl); }

  static CBTypesInfo inputTypes() { return Common::strInfos; }
  static CBTypesInfo outputTypes() { return Common::bytesInfos; }

  static size_t write_data(void *ptr, size_t size, size_t nmemb, void *pget) {
    auto get = reinterpret_cast<Get *>(pget);
    if (get->_canceling)
      return 0; // this should stop the download!

    auto offset = get->_buffer.size();
    get->_buffer.resize(offset + nmemb + 1);
    memcpy(&get->_buffer[offset], ptr, nmemb);
    // to allow easy string conversion we do this little trick
    get->_buffer[offset + nmemb] = 0;
    get->_buffer.resize(offset + nmemb);
    return nmemb;
  }

  CBVar activate(CBContext *context, const CBVar &input) {
    // prepare
    curl_easy_setopt(_curl, CURLOPT_URL, input.payload.stringValue);
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
    _buffer.clear();
    _canceling = false;
    // run
    auto asyncRes =
        std::async(std::launch::async, [this]() { curl_easy_perform(_curl); });
    // wait suspending!
    while (true) {
      auto state = asyncRes.wait_for(std::chrono::seconds(0));
      if (state == std::future_status::ready)
        break;
      auto chainState = Core::suspend(context, 0);
      if (chainState.payload.chainState != Continue) {
        Core::log("Curl: Canceling...");
        _canceling = true;
        // wait the worker blocking
        asyncRes.wait();
        return chainState;
      }
    }
    // this should throw if we had exceptions, but curl won't throw
    asyncRes.get();
    // return
    CBVar res{};
    res.valueType = CBType::Bytes;
    res.payload.bytesSize = _buffer.size();
    res.payload.bytesValue = &_buffer[0];
    return res;
  }
};

typedef BlockWrapper<Curl::Get> GetBlock;

} // namespace Curl
void registerBlocks() {
  Curl::Common::init();
  Core::registerBlock("Curl.Get", &Curl::GetBlock::create);
}
}; // namespace chainblocks
