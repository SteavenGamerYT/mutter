/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (C) 2017 Red Hat
 * Copyright (c) 2018 DisplayLink (UK) Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#define GL_GLEXT_PROTOTYPES

#include "backends/native/meta-renderer-native-gles3.h"

#include <GLES3/gl3.h>
#include <drm_fourcc.h>
#include <errno.h>
#include <gio/gio.h>
#include <string.h>

#include "backends/meta-egl-ext.h"
#include "backends/meta-gles3.h"
#include "backends/meta-gles3-table.h"

/*
 * GL/gl.h being included may conflict with gl3.h on some architectures.
 * Make sure that hasn't happened on any architecture.
 */
#ifdef GL_VERSION_1_1
#error "Somehow included OpenGL headers when we shouldn't have"
#endif

static GQuark shader_program_quark = 0;

static GLuint
load_shader (const char *src,
             GLenum      type)
{
  GLuint shader = glCreateShader (type);

  if (shader)
    {
      GLint compiled;

      glShaderSource (shader, 1, &src, NULL);
      glCompileShader (shader);
      glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);
      if (!compiled)
        {
          GLchar log[1024];

          glGetShaderInfoLog (shader, sizeof (log) - 1, NULL, log);
          log[sizeof (log) - 1] = '\0';
          g_warning ("load_shader compile failed: %s", log);
          glDeleteShader (shader);
          shader = 0;
        }
    }

  return shader;
}

static void
ensure_shader_program (MetaGles3 *gles3)
{
  static const char vertex_shader_source[] =
    "#version 100\n"
    "attribute vec2 position;\n"
    "attribute vec2 texcoord;\n"
    "varying vec2 v_texcoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "  gl_Position = vec4(position, 0.0, 1.0);\n"
    "  v_texcoord = texcoord;\n"
    "}\n";

  static const char fragment_shader_source[] =
    "#version 100\n"
    "#extension GL_OES_EGL_image_external : require\n"
    "precision mediump float;\n"
    "uniform samplerExternalOES s_texture;\n"
    "varying vec2 v_texcoord;\n"
    "\n"
    " void main()\n"
    "{\n"
    "  gl_FragColor = texture2D(s_texture, v_texcoord);\n"
    "}\n";

  static const GLfloat box[] =
    { // position    texcoord
      -1.0f,  1.0f, 0.0f, 0.0f,
       1.0f,  1.0f, 1.0f, 0.0f,
       1.0f, -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 1.0f,
    };
  GLint linked;
  GLuint vertex_shader, fragment_shader;
  GLint position_attrib, texcoord_attrib;
  GLuint shader_program;

  if (!shader_program_quark)
    shader_program_quark = g_quark_from_static_string ("shader program");

  if (g_object_get_qdata (G_OBJECT (gles3), shader_program_quark))
    return;

  shader_program = glCreateProgram ();
  g_return_if_fail (shader_program);
  g_object_set_qdata_full (G_OBJECT (gles3),
                           shader_program_quark,
                           GUINT_TO_POINTER (shader_program),
                           NULL);

  vertex_shader = load_shader (vertex_shader_source, GL_VERTEX_SHADER);
  g_return_if_fail (vertex_shader);
  fragment_shader = load_shader (fragment_shader_source, GL_FRAGMENT_SHADER);
  g_return_if_fail (fragment_shader);

  GLBAS (gles3, glAttachShader, (shader_program, vertex_shader));
  GLBAS (gles3, glAttachShader, (shader_program, fragment_shader));
  GLBAS (gles3, glLinkProgram, (shader_program));
  GLBAS (gles3, glGetProgramiv, (shader_program, GL_LINK_STATUS, &linked));
  if (!linked)
    {
      GLchar log[1024];

      glGetProgramInfoLog (shader_program, sizeof (log) - 1, NULL, log);
      log[sizeof (log) - 1] = '\0';
      g_warning ("Link failed: %s", log);
      return;
    }

  GLBAS (gles3, glUseProgram, (shader_program));

  position_attrib = glGetAttribLocation (shader_program, "position");
  GLBAS (gles3, glEnableVertexAttribArray, (position_attrib));
  GLBAS (gles3, glVertexAttribPointer,
    (position_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (GLfloat), box));

  texcoord_attrib = glGetAttribLocation (shader_program, "texcoord");
  GLBAS (gles3, glEnableVertexAttribArray, (texcoord_attrib));
  GLBAS (gles3, glVertexAttribPointer,
    (texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof (GLfloat), box + 2));
}

static void
paint_egl_image (MetaGles3   *gles3,
                 EGLImageKHR  egl_image,
                 int          width,
                 int          height)
{
  GLuint texture;

  meta_gles3_clear_error (gles3);
  ensure_shader_program (gles3);

  GLBAS (gles3, glViewport, (0, 0, width, height));

  GLBAS (gles3, glActiveTexture, (GL_TEXTURE0));
  GLBAS (gles3, glGenTextures, (1, &texture));
  GLBAS (gles3, glBindTexture, (GL_TEXTURE_EXTERNAL_OES, texture));
  GLEXT (gles3, glEGLImageTargetTexture2DOES, (GL_TEXTURE_EXTERNAL_OES,
                                               egl_image));
  GLBAS (gles3, glTexParameteri, (GL_TEXTURE_EXTERNAL_OES,
                                  GL_TEXTURE_MAG_FILTER,
                                  GL_NEAREST));
  GLBAS (gles3, glTexParameteri, (GL_TEXTURE_EXTERNAL_OES,
                                  GL_TEXTURE_MIN_FILTER,
                                  GL_NEAREST));
  GLBAS (gles3, glTexParameteri, (GL_TEXTURE_EXTERNAL_OES,
                                  GL_TEXTURE_WRAP_S,
                                  GL_CLAMP_TO_EDGE));
  GLBAS (gles3, glTexParameteri, (GL_TEXTURE_EXTERNAL_OES,
                                  GL_TEXTURE_WRAP_T,
                                  GL_CLAMP_TO_EDGE));

  GLBAS (gles3, glDrawArrays, (GL_TRIANGLE_FAN, 0, 4));

  GLBAS (gles3, glDeleteTextures, (1, &texture));
}

gboolean
meta_renderer_native_gles3_blit_shared_bo (MetaEgl        *egl,
                                           MetaGles3      *gles3,
                                           EGLDisplay      egl_display,
                                           EGLContext      egl_context,
                                           EGLSurface      egl_surface,
                                           struct gbm_bo  *shared_bo,
                                           GError        **error)
{
  int shared_bo_fd;
  unsigned int width;
  unsigned int height;
  uint32_t i, n_planes;
  uint32_t strides[4] = { 0 };
  uint32_t offsets[4] = { 0 };
  uint64_t modifiers[4] = { 0 };
  int fds[4] = { -1, -1, -1, -1 };
  uint32_t format;
  EGLImageKHR egl_image;
  gboolean use_modifiers;

  shared_bo_fd = gbm_bo_get_fd (shared_bo);
  if (shared_bo_fd < 0)
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                   "Failed to export gbm_bo: %s", strerror (errno));
      return FALSE;
    }

  width = gbm_bo_get_width (shared_bo);
  height = gbm_bo_get_height (shared_bo);
  format = gbm_bo_get_format (shared_bo);

  n_planes = gbm_bo_get_plane_count (shared_bo);
  for (i = 0; i < n_planes; i++)
    {
      strides[i] = gbm_bo_get_stride_for_plane (shared_bo, i);
      offsets[i] = gbm_bo_get_offset (shared_bo, i);
      modifiers[i] = gbm_bo_get_modifier (shared_bo);
      fds[i] = shared_bo_fd;
    }

  /* Workaround for https://gitlab.gnome.org/GNOME/mutter/issues/18 */
  if (modifiers[0] == DRM_FORMAT_MOD_LINEAR ||
      modifiers[0] == DRM_FORMAT_MOD_INVALID)
    use_modifiers = FALSE;
  else
    use_modifiers = TRUE;

  egl_image = meta_egl_create_dmabuf_image (egl,
                                            egl_display,
                                            width,
                                            height,
                                            format,
                                            n_planes,
                                            fds,
                                            strides,
                                            offsets,
                                            use_modifiers ? modifiers : NULL,
                                            error);
  close (shared_bo_fd);

  if (!egl_image)
    return FALSE;

  paint_egl_image (gles3, egl_image, width, height);

  meta_egl_destroy_image (egl, egl_display, egl_image, NULL);

  return TRUE;
}
