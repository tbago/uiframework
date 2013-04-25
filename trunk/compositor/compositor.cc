// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "compositor/compositor.h"

#include <algorithm>
#include <deque>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/memory/singleton.h"
#include "base/message_loop.h"
#include "base/string_util.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#include "cc/base/thread_impl.h"
#include "cc/input/input_handler.h"
#include "cc/layers/layer.h"
#include "cc/trees/layer_tree_host.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "compositor/compositor_observer.h"
#include "compositor/compositor_switches.h"
#include "compositor/dip_util.h"
#include "compositor/layer.h"
#include "gl/gl_context.h"
#include "gl/gl_implementation.h"
#include "gl/gl_surface.h"
#include "gl/gl_switches.h"

namespace {

const double kDefaultRefreshRate = 60.0;
const double kTestRefreshRate = 100.0;

enum SwapType {
  DRAW_SWAP,
  READPIXELS_SWAP,
};

const int kCompositorLockTimeoutMs = 67;

class PendingSwap {
 public:
  PendingSwap(SwapType type, ui::PostedSwapQueue* posted_swaps);
  ~PendingSwap();

  SwapType type() const { return type_; }
  bool posted() const { return posted_; }

 private:
  friend class ui::PostedSwapQueue;

  SwapType type_;
  bool posted_;
  ui::PostedSwapQueue* posted_swaps_;

  DISALLOW_COPY_AND_ASSIGN(PendingSwap);
};

}  // namespace

namespace ui {

Texture::Texture(bool flipped, const gfx::Size& size, float device_scale_factor)
    : size_(size),
      flipped_(flipped),
      device_scale_factor_(device_scale_factor) {
}

Texture::~Texture() {
}

std::string Texture::Produce() {
  return EmptyString();
}

CompositorLock::CompositorLock(Compositor* compositor)
    : compositor_(compositor) {
  MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&CompositorLock::CancelLock, AsWeakPtr()),
      base::TimeDelta::FromMilliseconds(kCompositorLockTimeoutMs));
}

CompositorLock::~CompositorLock() {
  CancelLock();
}

void CompositorLock::CancelLock() {
  if (!compositor_)
    return;
  compositor_->UnlockCompositor();
  compositor_ = NULL;
}

class PostedSwapQueue {
 public:
  PostedSwapQueue() : pending_swap_(NULL) {
  }

  ~PostedSwapQueue() {
    DCHECK(!pending_swap_);
  }

  SwapType NextPostedSwap() const {
    return queue_.front();
  }

  bool AreSwapsPosted() const {
    return !queue_.empty();
  }

  int NumSwapsPosted(SwapType type) const {
    int count = 0;
    for (std::deque<SwapType>::const_iterator it = queue_.begin();
         it != queue_.end(); ++it) {
      if (*it == type)
        count++;
    }
    return count;
  }

  void PostSwap() {
    DCHECK(pending_swap_);
    queue_.push_back(pending_swap_->type());
    pending_swap_->posted_ = true;
  }

  void EndSwap() {
    queue_.pop_front();
  }

 private:
  friend class ::PendingSwap;

  PendingSwap* pending_swap_;
  std::deque<SwapType> queue_;

  DISALLOW_COPY_AND_ASSIGN(PostedSwapQueue);
};

}  // namespace ui

namespace {

PendingSwap::PendingSwap(SwapType type, ui::PostedSwapQueue* posted_swaps)
    : type_(type), posted_(false), posted_swaps_(posted_swaps) {
  // Only one pending swap in flight.
  DCHECK_EQ(static_cast<PendingSwap*>(NULL), posted_swaps_->pending_swap_);
  posted_swaps_->pending_swap_ = this;
}

PendingSwap::~PendingSwap() {
  DCHECK_EQ(this, posted_swaps_->pending_swap_);
  posted_swaps_->pending_swap_ = NULL;
}

}  // namespace

namespace ui {

Compositor::Compositor(CompositorDelegate* delegate,
                       gfx::AcceleratedWidget widget)
    : delegate_(delegate),
      root_layer_(NULL),
      widget_(widget),
      posted_swaps_(new PostedSwapQueue()),
      device_scale_factor_(0.0f),
      last_started_frame_(0),
      last_ended_frame_(0),
      disable_schedule_composite_(false),
      compositor_lock_(NULL)
{
  CommandLine* command_line = CommandLine::ForCurrentProcess();

  scoped_ptr<cc::Thread> thread;
}

Compositor::~Compositor() {
  CancelCompositorLock();
  DCHECK(!compositor_lock_);

  // Don't call |CompositorDelegate::ScheduleDraw| from this point.
  delegate_ = NULL;
  if (root_layer_)
    root_layer_->SetCompositor(NULL);

  // Stop all outstanding draws before telling the ContextFactory to tear
  // down any contexts that the |host_| may rely upon.
}

void Compositor::Initialize(bool use_thread) {
}

void Compositor::Terminate() {
}

void Compositor::ScheduleDraw() {
  if (delegate_)
    delegate_->ScheduleDraw();
}

void Compositor::SetRootLayer(Layer* root_layer) {
  if (root_layer_ == root_layer)
    return;
  if (root_layer_)
    root_layer_->SetCompositor(NULL);
  root_layer_ = root_layer;
  if (root_layer_ && !root_layer_->GetCompositor())
    root_layer_->SetCompositor(this);
}

void Compositor::SetHostHasTransparentBackground(
    bool host_has_transparent_background)
{
}

void Compositor::Draw(bool force_clear) {
  if (!root_layer_)
    return;

  last_started_frame_++;
  PendingSwap pending_swap(DRAW_SWAP, posted_swaps_.get());
  if (!IsLocked()) {
    // TODO(nduca): Temporary while compositor calls
    // compositeImmediately() directly.
  }
  if (!pending_swap.posted())
    NotifyEnd();
}

void Compositor::ScheduleFullDraw()
{
}

bool Compositor::ReadPixels(SkBitmap* bitmap,
                            const gfx::Rect& bounds_in_pixel) {
  if (bounds_in_pixel.right() > size().width() ||
      bounds_in_pixel.bottom() > size().height())
    return false;
  bitmap->setConfig(SkBitmap::kARGB_8888_Config,
                    bounds_in_pixel.width(), bounds_in_pixel.height());
  bitmap->allocPixels();
  SkAutoLockPixels lock_image(*bitmap);
  unsigned char* pixels = static_cast<unsigned char*>(bitmap->getPixels());
  CancelCompositorLock();
  PendingSwap pending_swap(READPIXELS_SWAP, posted_swaps_.get());
  return true;
}

void Compositor::SetScaleAndSize(float scale, const gfx::Size& size_in_pixel) {
  DCHECK_GT(scale, 0);
  if (!size_in_pixel.IsEmpty()) {
    size_ = size_in_pixel;
  }
  if (device_scale_factor_ != scale) {
    device_scale_factor_ = scale;
    if (root_layer_)
      root_layer_->OnDeviceScaleFactorChanged(scale);
  }
}

void Compositor::AddObserver(CompositorObserver* observer) {
  observer_list_.AddObserver(observer);
}

void Compositor::RemoveObserver(CompositorObserver* observer) {
  observer_list_.RemoveObserver(observer);
}

bool Compositor::HasObserver(CompositorObserver* observer) {
  return observer_list_.HasObserver(observer);
}

void Compositor::OnSwapBuffersPosted() {
  posted_swaps_->PostSwap();
}

void Compositor::OnSwapBuffersComplete() {
  DCHECK(posted_swaps_->AreSwapsPosted());
  DCHECK_GE(1, posted_swaps_->NumSwapsPosted(DRAW_SWAP));
  if (posted_swaps_->NextPostedSwap() == DRAW_SWAP)
    NotifyEnd();
  posted_swaps_->EndSwap();
}

void Compositor::OnSwapBuffersAborted() {
  DCHECK_GE(1, posted_swaps_->NumSwapsPosted(DRAW_SWAP));

  // We've just lost the context, so unwind all posted_swaps.
  while (posted_swaps_->AreSwapsPosted()) {
    if (posted_swaps_->NextPostedSwap() == DRAW_SWAP)
      NotifyEnd();
    posted_swaps_->EndSwap();
  }

  FOR_EACH_OBSERVER(CompositorObserver,
                    observer_list_,
                    OnCompositingAborted(this));
}

void Compositor::OnUpdateVSyncParameters(base::TimeTicks timebase,
                                         base::TimeDelta interval) {
  FOR_EACH_OBSERVER(CompositorObserver,
                    observer_list_,
                    OnUpdateVSyncParameters(this, timebase, interval));
}

scoped_refptr<CompositorLock> Compositor::GetCompositorLock() {
  if (!compositor_lock_) {
    compositor_lock_ = new CompositorLock(this);
    FOR_EACH_OBSERVER(CompositorObserver,
                      observer_list_,
                      OnCompositingLockStateChanged(this));
  }
  return compositor_lock_;
}

void Compositor::UnlockCompositor() {
  DCHECK(compositor_lock_);
  compositor_lock_ = NULL;
  FOR_EACH_OBSERVER(CompositorObserver,
                    observer_list_,
                    OnCompositingLockStateChanged(this));
}

void Compositor::CancelCompositorLock() {
  if (compositor_lock_)
    compositor_lock_->CancelLock();
}

void Compositor::NotifyEnd() {
  last_ended_frame_++;
  FOR_EACH_OBSERVER(CompositorObserver,
                    observer_list_,
                    OnCompositingEnded(this));
}

}  // namespace ui
