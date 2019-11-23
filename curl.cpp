/* SPDX-License-Identifier: BSD 3-Clause "New" or "Revised" License */
/* Copyright © 2019 Giovanni Petrantoni */

#define STB_DS_IMPLEMENTATION 1

#include <dllblock.hpp>

namespace chainblocks {
struct Common {
  static inline CBTypeInfo intInfo{CBType::Int};

  static inline CBTypeInfo anyInfo{CBType::Any};
  static inline CBTypesInfo anyInfos = nullptr;

  static inline CBTypeInfo strInfo{CBType::String};
  static inline CBTypesInfo strInfos = nullptr;

  static inline CBTypeInfo botInfo{CBType::Object};
  static inline CBTypesInfo botInfos = nullptr;

  static inline CBTypeInfo noneInfo{CBType::None};
  static inline CBTypeInfo blockInfo{CBType::Block};
  static inline CBTypeInfo blocksInfo{CBType::Seq};
  static inline CBTypesInfo blocksOrNoneInfo = nullptr;

  static inline CBExposedTypeInfo botExpInfo{
      "TgBot.Bot", "The current active telegram bot", Common::botInfo};
  static inline CBExposedTypesInfo botExpInfos = nullptr;

  static void init() {
    stbds_arrpush(anyInfos, anyInfo);

    stbds_arrpush(strInfos, strInfo);

    botInfo.objectVendorId = 'frag';
    botInfo.objectTypeId = 'TgBo';
    stbds_arrpush(botInfos, botInfo);

    stbds_arrpush(blocksOrNoneInfo, noneInfo);
    stbds_arrpush(blocksOrNoneInfo, blockInfo);
    blocksInfo.seqType = &blockInfo;
    stbds_arrpush(blocksOrNoneInfo, blocksInfo);

    stbds_arrpush(botExpInfos, botExpInfo);
  }
};

  struct Get {
     static CBTypesInfo outputTypes() { return Common::strInfos; }

  static CBTypesInfo inputTypes() { return Common::anyInfos; }
  };

void registerBlocks() {
  Common::init();
  Core::registerBlock("Curl.Get", &GetBlock::create);
}
}; // namespace chainblocks
