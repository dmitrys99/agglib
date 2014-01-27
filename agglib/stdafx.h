// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // »сключите редко используемые компоненты из заголовков Windows
// ‘айлы заголовков Windows:
#include <windows.h>

// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
/* agg2 components */

#include "agg_arc.h"
#include "agg_ellipse.h"
#ifdef HAVE_FREETYPE2
#include "agg_font_freetype.h"
#else
#include "agg_font_win32_tt.h"
#endif
#include "agg_path_storage.h"
#include "agg_pixfmt_rgb.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_primitives.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_ellipse.h"
#include "util/agg_color_conv_rgb8.h"
#include "util/agg_color_conv_rgb16.h"
#include "agg_pixfmt_rgba.h"
#include "platform/agg_platform_support.h"
#include "platform/win32/agg_win32_bmp.h"
