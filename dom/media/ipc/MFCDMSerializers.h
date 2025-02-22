/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_MEDIA_IPC_MFCDMSERIALIZERS_H_
#define DOM_MEDIA_IPC_MFCDMSERIALIZERS_H_

#include "ipc/EnumSerializer.h"
#include "MediaData.h"
#include "mozilla/KeySystemConfig.h"

namespace IPC {

template <>
struct ParamTraits<mozilla::KeySystemConfig::Requirement>
    : public ContiguousEnumSerializerInclusive<
          mozilla::KeySystemConfig::Requirement,
          mozilla::KeySystemConfig::Requirement::Required,
          mozilla::KeySystemConfig::Requirement::NotAllowed> {};

template <>
struct ParamTraits<mozilla::KeySystemConfig::SessionType>
    : public ContiguousEnumSerializerInclusive<
          mozilla::KeySystemConfig::SessionType,
          mozilla::KeySystemConfig::SessionType::Temporary,
          mozilla::KeySystemConfig::SessionType::PersistentLicense> {};

template <>
struct ParamTraits<mozilla::CryptoScheme>
    : public ContiguousEnumSerializerInclusive<mozilla::CryptoScheme,
                                               mozilla::CryptoScheme::None,
                                               mozilla::CryptoScheme::Cbcs> {};

}  // namespace IPC

#endif  // DOM_MEDIA_IPC_MFCDMSERIALIZERS_H_
