// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GL_CONTEXT_CGL_H_
#define UI_GL_GL_CONTEXT_CGL_H_

#include <OpenGL/CGLTypes.h>

#include "base/memory/scoped_generic_obj.h"
#include "ui/gl/gl_context.h"

namespace gfx {

class GLSurface;

// Encapsulates a CGL OpenGL context.
class GLContextCGL : public GLContext {
 public:
  explicit GLContextCGL(GLShareGroup* share_group);

  // Implement GLContext.
  virtual bool Initialize(GLSurface* compatible_surface,
                          GpuPreference gpu_preference) OVERRIDE;
  virtual void Destroy() OVERRIDE;
  virtual bool MakeCurrent(GLSurface* surface) OVERRIDE;
  virtual void ReleaseCurrent(GLSurface* surface) OVERRIDE;
  virtual bool IsCurrent(GLSurface* surface) OVERRIDE;
  virtual void* GetHandle() OVERRIDE;
  virtual void SetSwapInterval(int interval) OVERRIDE;
  virtual bool GetTotalGpuMemory(size_t* bytes) OVERRIDE;

 protected:
  virtual ~GLContextCGL();

 private:
  GpuPreference GetGpuPreference();

  void* context_;
  GpuPreference gpu_preference_;

  CGLPixelFormatObj discrete_pixelformat_;

  int screen_;
  int renderer_id_;

  DISALLOW_COPY_AND_ASSIGN(GLContextCGL);
};

class ScopedCGLDestroyRendererInfo {
 public:
  void operator()(CGLRendererInfoObj x) const;
};

typedef ScopedGenericObj<CGLRendererInfoObj, ScopedCGLDestroyRendererInfo>
    ScopedCGLRendererInfoObj;

}  // namespace gfx

#endif  // UI_GL_GL_CONTEXT_CGL_H_
