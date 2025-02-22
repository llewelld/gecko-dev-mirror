/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "RenderTextureHostWrapper.h"

#include "mozilla/gfx/Logging.h"
#include "mozilla/layers/RemoteTextureMap.h"
#include "mozilla/webrender/RenderThread.h"

namespace mozilla {
namespace wr {

RenderTextureHostWrapper::RenderTextureHostWrapper(
    ExternalImageId aExternalImageId)
    : mExternalImageId(aExternalImageId) {
  MOZ_COUNT_CTOR_INHERITED(RenderTextureHostWrapper, RenderTextureHost);
  EnsureTextureHost();
}

RenderTextureHostWrapper::RenderTextureHostWrapper(
    const layers::RemoteTextureId aTextureId,
    const layers::RemoteTextureOwnerId aOwnerId, const base::ProcessId aForPid)
    : mExternalImageId({}),
      mTextureId(Some(aTextureId)),
      mOwnerId(Some(aOwnerId)),
      mForPid(Some(aForPid)) {
  MOZ_COUNT_CTOR_INHERITED(RenderTextureHostWrapper, RenderTextureHost);
}

RenderTextureHostWrapper::~RenderTextureHostWrapper() {
  MOZ_COUNT_DTOR_INHERITED(RenderTextureHostWrapper, RenderTextureHost);
}

void RenderTextureHostWrapper::EnsureTextureHost() const {
  MOZ_ASSERT(mTextureId.isNothing());

  if (mTextureHost) {
    return;
  }

  mTextureHost = RenderThread::Get()->GetRenderTexture(mExternalImageId);
  MOZ_ASSERT(mTextureHost);
  if (!mTextureHost) {
    gfxCriticalNoteOnce << "Failed to get RenderTextureHost for extId:"
                        << AsUint64(mExternalImageId);
  }
}

void RenderTextureHostWrapper::EnsureRemoteTexture() const {
  MOZ_ASSERT(mTextureId.isSome());

  if (mTextureHost) {
    return;
  }

  auto externalImageId =
      layers::RemoteTextureMap::Get()->GetExternalImageIdOfRemoteTextureSync(
          *mTextureId, *mOwnerId, *mForPid);
  if (externalImageId.isNothing()) {
    // This could happen with IPC abnormal shutdown
    return;
  }

  mTextureHost = RenderThread::Get()->GetRenderTexture(*externalImageId);
  MOZ_ASSERT(mTextureHost);
  if (!mTextureHost) {
    gfxCriticalNoteOnce << "Failed to get RenderTextureHost for extId:"
                        << AsUint64(*externalImageId);
  }
}

wr::WrExternalImage RenderTextureHostWrapper::Lock(uint8_t aChannelIndex,
                                                   gl::GLContext* aGL) {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }

  if (!mTextureHost) {
    return InvalidToWrExternalImage();
  }

  return mTextureHost->Lock(aChannelIndex, aGL);
}

void RenderTextureHostWrapper::Unlock() {
  if (mTextureHost) {
    mTextureHost->Unlock();
  }
}

std::pair<gfx::Point, gfx::Point> RenderTextureHostWrapper::GetUvCoords(
    gfx::IntSize aTextureSize) const {
  if (mTextureHost) {
    return mTextureHost->GetUvCoords(aTextureSize);
  }
  return RenderTextureHost::GetUvCoords(aTextureSize);
}

void RenderTextureHostWrapper::ClearCachedResources() {
  if (mTextureHost) {
    mTextureHost->ClearCachedResources();
  }
}

void RenderTextureHostWrapper::PrepareForUse() {
  if (!mTextureHost) {
    return;
  }
  mTextureHost->PrepareForUse();
}

void RenderTextureHostWrapper::NotifyForUse() {
  if (!mTextureHost) {
    return;
  }
  mTextureHost->NotifyForUse();
}

void RenderTextureHostWrapper::NotifyNotUsed() {
  if (!mTextureHost) {
    return;
  }
  mTextureHost->NotifyNotUsed();
}

bool RenderTextureHostWrapper::SyncObjectNeeded() { return false; }

RenderMacIOSurfaceTextureHost*
RenderTextureHostWrapper::AsRenderMacIOSurfaceTextureHost() {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderMacIOSurfaceTextureHost();
}

RenderDXGITextureHost* RenderTextureHostWrapper::AsRenderDXGITextureHost() {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderDXGITextureHost();
}

RenderDXGIYCbCrTextureHost*
RenderTextureHostWrapper::AsRenderDXGIYCbCrTextureHost() {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderDXGIYCbCrTextureHost();
}

RenderDcompSurfaceTextureHost*
RenderTextureHostWrapper::AsRenderDcompSurfaceTextureHost() {
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderDcompSurfaceTextureHost();
}

RenderTextureHostSWGL* RenderTextureHostWrapper::AsRenderTextureHostSWGL() {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderTextureHostSWGL();
}

RenderAndroidHardwareBufferTextureHost*
RenderTextureHostWrapper::AsRenderAndroidHardwareBufferTextureHost() {
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderAndroidHardwareBufferTextureHost();
}

RenderAndroidSurfaceTextureHost*
RenderTextureHostWrapper::AsRenderAndroidSurfaceTextureHost() {
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderAndroidSurfaceTextureHost();
}

RenderTextureHostSWGL* RenderTextureHostWrapper::EnsureRenderTextureHostSWGL()
    const {
  if (mTextureId.isSome()) {
    EnsureRemoteTexture();
  }
  if (!mTextureHost) {
    return nullptr;
  }
  return mTextureHost->AsRenderTextureHostSWGL();
}

bool RenderTextureHostWrapper::IsWrappingAsyncRemoteTexture() {
  return mTextureId.isSome();
}

size_t RenderTextureHostWrapper::GetPlaneCount() const {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    return swglHost->GetPlaneCount();
  }
  return 0;
}

gfx::SurfaceFormat RenderTextureHostWrapper::GetFormat() const {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    return swglHost->GetFormat();
  }
  return gfx::SurfaceFormat::UNKNOWN;
}

gfx::ColorDepth RenderTextureHostWrapper::GetColorDepth() const {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    return swglHost->GetColorDepth();
  }
  return gfx::ColorDepth::COLOR_8;
}

gfx::YUVRangedColorSpace RenderTextureHostWrapper::GetYUVColorSpace() const {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    return swglHost->GetYUVColorSpace();
  }
  return gfx::YUVRangedColorSpace::Default;
}

bool RenderTextureHostWrapper::MapPlane(RenderCompositor* aCompositor,
                                        uint8_t aChannelIndex,
                                        PlaneInfo& aPlaneInfo) {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    return swglHost->MapPlane(aCompositor, aChannelIndex, aPlaneInfo);
  }
  return false;
}

void RenderTextureHostWrapper::UnmapPlanes() {
  if (RenderTextureHostSWGL* swglHost = EnsureRenderTextureHostSWGL()) {
    swglHost->UnmapPlanes();
  }
}

}  // namespace wr
}  // namespace mozilla
