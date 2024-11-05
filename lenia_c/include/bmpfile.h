/**
 * @file bmpfile.h
 * @brief The BMP library header
 *
 * libbmp - BMP library
 * Copyright (C) 2009 lidaibin(超越无限)
 * mail: lidaibin@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#ifndef __bmpfile_h__
#define __bmpfile_h__

#ifdef __cplusplus
#define BMP_BEGIN_DECLS extern "C" {
#define BMP_END_DECLS }
#else
#define BMP_BEGIN_DECLS
#define BMP_END_DECLS
#endif

BMP_BEGIN_DECLS

#ifndef bool
typedef int bool;
#define FALSE (0)
#define TRUE !FALSE
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

typedef enum {
  BI_RGB = 0,
  BI_RLE8,
  BI_RLE4,
  BI_BITFIELDS,
  BI_JPEG,
  BI_PNG,
} bmp_compression_method_t;

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t alpha;
} rgb_pixel_t;

typedef struct _bmpfile bmpfile_t;

bmpfile_t *bmp_create(uint32_t width, uint32_t height, uint32_t depth);
/* TODO */
/* bmpfile_t *bmp_create_from_file(const char *filename); */
void bmp_destroy(bmpfile_t *bmp);

uint32_t bmp_get_width(bmpfile_t *bmp);
uint32_t bmp_get_height(bmpfile_t *bmp);
uint32_t bmp_get_depth(bmpfile_t *bmp);

bmp_compression_method_t bmp_get_compression_method(bmpfile_t *bmp);
void bmp_set_compression_method(bmpfile_t *bmp, bmp_compression_method_t t);

uint32_t bmp_get_dpi_x(bmpfile_t *bmp);
uint32_t bmp_get_dpi_y(bmpfile_t *bmp);
void bmp_set_dpi(bmpfile_t *bmp, uint32_t x, uint32_t y);

rgb_pixel_t bmp_get_pixel(bmpfile_t *bmp, uint32_t x, uint32_t y);
bool bmp_set_pixel(bmpfile_t *bmp, uint32_t x, uint32_t y, rgb_pixel_t pixel);

bool bmp_save(bmpfile_t *bmp, const char *filename);

BMP_END_DECLS

#endif /* __bmpfile_h__ */
