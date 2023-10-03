/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/*
 * Copyright (C) 2022 Red Hat, Inc.
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
 */

#include "config.h"

#include <glib.h>

#include "backends/edid.h"

/* GSM 23507, DisplayHDR certified */
unsigned char edid_blob[] = {
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x1e, 0x6d, 0xd3, 0x5b,
  0xf4, 0xe7, 0x1d, 0x00, 0x02, 0x20, 0x01, 0x04, 0xb5, 0x46, 0x27, 0x78,
  0x9d, 0x8c, 0xb5, 0xaf, 0x4f, 0x43, 0xab, 0x26, 0x0e, 0x50, 0x54, 0x21,
  0x08, 0x00, 0xd1, 0xc0, 0x61, 0x40, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x09, 0xec, 0x00, 0xa0, 0xa0, 0xa0,
  0x67, 0x50, 0x30, 0x20, 0x3a, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x1a,
  0x00, 0x00, 0x00, 0xfd, 0x0c, 0x30, 0xa5, 0x03, 0x03, 0x46, 0x01, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x4c,
  0x47, 0x20, 0x55, 0x4c, 0x54, 0x52, 0x41, 0x47, 0x45, 0x41, 0x52, 0x0a,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x32, 0x30, 0x32, 0x4d, 0x41, 0x50, 0x4e,
  0x47, 0x59, 0x46, 0x32, 0x34, 0x0a, 0x02, 0x8d, 0x02, 0x03, 0x2d, 0x71,
  0x23, 0x09, 0x06, 0x07, 0xe2, 0x00, 0x6a, 0xe3, 0x05, 0xc0, 0x00, 0xe6,
  0x06, 0x05, 0x01, 0x61, 0x61, 0x4f, 0x48, 0x10, 0x04, 0x03, 0x01, 0x1f,
  0x13, 0x3f, 0x40, 0x6d, 0x1a, 0x00, 0x00, 0x02, 0x05, 0x30, 0xa5, 0x00,
  0x04, 0x61, 0x4f, 0x61, 0x4f, 0x56, 0x5e, 0x00, 0xa0, 0xa0, 0xa0, 0x29,
  0x50, 0x30, 0x20, 0x35, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x1a, 0x5a,
  0xa0, 0x00, 0xa0, 0xa0, 0xa0, 0x46, 0x50, 0x30, 0x20, 0x3a, 0x00, 0xb9,
  0x88, 0x21, 0x00, 0x00, 0x00, 0x6f, 0xc2, 0x00, 0xa0, 0xa0, 0xa0, 0x55,
  0x50, 0x30, 0x20, 0x3a, 0x00, 0xb9, 0x88, 0x21, 0x00, 0x00, 0x1a, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x2f, 0x70, 0x12, 0x79, 0x03, 0x00, 0x03, 0x00, 0x14,
  0x3d, 0x11, 0x01, 0x84, 0xff, 0x09, 0x9f, 0x00, 0x2f, 0x80, 0x1f, 0x00,
  0x9f, 0x05, 0x76, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf3, 0x90
};
unsigned int edid_blob_len = 384;


int
main (int    argc,
      char **argv)
{
  MetaEdidInfo *edid_info;
  edid_info = meta_edid_info_new_parse (edid_blob,edid_blob_len);

  g_assert (edid_info != NULL);
  g_assert (strcmp (edid_info->manufacturer_code, "GSM") == 0);
  g_assert (edid_info->product_code == 23507);
  g_assert (edid_info->hdr_static_metadata.max_luminance == 408);
  g_assert (edid_info->hdr_static_metadata.tf ==
            (META_EDID_TF_TRADITIONAL_GAMMA_SDR | META_EDID_TF_PQ));
  g_assert (edid_info->colorimetry ==
            (META_EDID_COLORIMETRY_BT2020YCC | META_EDID_COLORIMETRY_BT2020RGB));
}
