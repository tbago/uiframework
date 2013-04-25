// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_TEXTURE_LAYER_CLIENT_H_
#define CC_LAYERS_TEXTURE_LAYER_CLIENT_H_

namespace cc {
class ResourceUpdateQueue;

class TextureLayerClient {
 public:
  // Called to prepare this layer's texture for compositing. The client may
  // queue a texture upload or copy on the ResourceUpdateQueue.
  // Returns the texture ID to be used for compositing.
  virtual unsigned PrepareTexture(ResourceUpdateQueue* queue) = 0;
 protected:
  virtual ~TextureLayerClient() {}
};

}  // namespace cc

#endif  // CC_LAYERS_TEXTURE_LAYER_CLIENT_H_
