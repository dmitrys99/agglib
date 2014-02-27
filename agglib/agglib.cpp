// agglib.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "agglib.h"

/*
draw_line
---------

Рисует линию на поверхности.
Задаются начальные и конечные координаты, толщина линии и ее цвет.
Если толщина линии равна 0, линия рисуется толщиной в 0.3 пикселя.
*/
int draw_line(surface* surface, float x0, float y0, float x1, float y1, float w, unsigned color, bool use_opacity)
{
	if (surface) {
		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;

		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
	
        pix_format pixf(rbuf);
        ren_base ren(pixf);

        agg::scanline_u8 sl;

        agg::rasterizer_scanline_aa<> ras;

        ras.gamma(agg::gamma_none());

        agg::path_storage ps;
        agg::conv_stroke<agg::path_storage> pg(ps);

		if (abs(w) < eps) w = 0.3f;
		pg.width(w);
		pg.line_cap(agg::round_cap);

		ps.remove_all();
		ps.move_to(x0+0.5f, y0+0.5f);
		ps.line_to(x1+0.5f, y1+0.5f);
		ras.reset();
		ras.add_path(pg);
		agg::render_scanlines_aa_solid(ras, sl, ren, fill_color);
		
		return 0;
	} else {
		return 1;
	}
}

/*
draw_rect
---------

Рисует прямоугольник на поверхности.
Прямоугольник располагается строго внутри координат (x0, y0) -- (x1,y1)
Если толщина линии равна 0, прямоугольник рисуется толщиной в 0.3 пикселя.
Если толщина меньше нуля, прямоугольник заливается цветом, если больше нуля, 
рисуется рамка заданной толщины и цвета.
*/
int draw_rect(surface* surface, float x0, float y0, float x1, float y1, float w, unsigned color, bool use_opacity)
{
	if (surface) {
		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;

		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
	
        pix_format pixf(rbuf);
        ren_base ren(pixf);
		ren_prim renp(ren);
		outline_rasterizer rasp(renp);
        agg::scanline_u8 sl;
        agg::rasterizer_scanline_aa<> ras;
		ras.gamma(agg::gamma_none());

        agg::path_storage ps;
        agg::conv_stroke<agg::path_storage> pg(ps);

		float d = w/2;
		x0 = floor(x0 + d) + 0.5f;
		x1 =  ceil(x1 - d) + 0.5f;
		y0 = floor(y0 + d) + 0.5f;
		y1 =  ceil(y1 - d) + 0.5f;
		
		pg.width(w);
        ps.remove_all();
        ps.move_to(x0, y0);
		ps.line_to(x1, y0);
		ps.line_to(x1, y1);
		ps.line_to(x0, y1);
		ps.line_to(x0, y0);
        ps.close_polygon();
        ras.reset();
		
		if (abs(w) < eps) {
			renp.line_color(fill_color);
			rasp.add_path(ps);
		} else {
			if (w < -eps) {
				renp.line_color(fill_color);
				renp.fill_color(fill_color);
				renp.solid_rectangle(
					int(floor(x0)+1), 
					int(floor(y0)+1), 
					int(ceil(x1)-1), 
					int(ceil(y1)-1));
			} else {
				ras.add_path(pg);
				agg::render_scanlines_aa_solid(ras, sl, ren, fill_color);
			}
		}
		return 0;
	} else {
		return 1;
	}
}

int draw_oval(surface* surface, float l, float t, float r, float b, float w, unsigned color, bool use_opacity)
{
	if (surface) {
		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;

		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
	
        typedef agg::renderer_base<agg::pixfmt_bgr24> ren_base;

        agg::pixfmt_bgr24 pixf(rbuf);
        ren_base ren(pixf);
		ren_prim renp(ren);
		outline_rasterizer rasp(renp);
        agg::scanline_u8 sl;

        agg::rasterizer_scanline_aa<> ras;

        ras.gamma(agg::gamma_none());

        agg::ellipse ell;

		float d = (w > eps) ? w/2 : 0.0f;
		ell.init(l+(r-l)/2+0.5f, t+(b-t)/2+0.5f, (r-l)/2-d, (b-t)/2-d);
			
		agg::conv_stroke<agg::ellipse> pg(ell);
		if (abs(w) < eps) w = 0.3f;
		pg.width(w);

        ras.reset();
		if (w < -eps) {ras.add_path(ell);} else {ras.add_path(pg);}
		agg::render_scanlines_aa_solid(ras, sl, ren, fill_color);
		return 0;
	} else {
		return 1;
	}
}

unsigned internal_draw_text(
	wchar_t* text, 
	wchar_t* typeface, 
	float height, 
	int font_weight, 
	float stroke_weight,
	float x0, float y0,
	bool italic, bool underline, bool strikeout,
	agg::rgba8& color, 
	agg::rasterizer_scanline_aa<>& ras, 
	agg::scanline_u8& sl, 
    renderer_solid& ren_solid, 
	renderer_bin& ren_bin, 
	font_engine_type& m_feng,
	font_manager_type& m_fman, 
	conv_curve_type& m_curves,
	conv_contour_type& m_contour)
{
    agg::glyph_rendering gren = agg::glyph_ren_native_mono;
	gren = agg::glyph_ren_agg_gray8;

    unsigned num_glyphs = 0;

    m_contour.width(stroke_weight);

    m_feng.hinting(true);
    m_feng.height(height);
	int res = GetDeviceCaps(m_feng.dc(), LOGPIXELSX);
	m_feng.resolution(res);

	/*
		Ширина шрифта в Windows - это что-то странное. MSDN говорит,
		«требуется указать среднюю ширину», но нет даже намека на то, 
		что считать «средней шириной». Логично было бы указывать ширину
		в зависимости от высоты шрифта, как это сделано во FreeType. 
		То есть, равенство высота == ширина должно приводить к «натуральным»,
		неискаженным глифам. В Windows необходимо указывать абсолютное 
		значение ширины, что весьма глупо и неудобно на практике.
	*/
    //-------------------------
    m_feng.width(0.0);
    m_feng.flip_y(true);

	// Матрица преобразований пока не используется
	// agg::trans_affine mtx;
	// mtx *= agg::trans_affine_skewing(-0.3, 0);
	// mtx *= agg::trans_affine_rotation(agg::deg2rad(-4.0));
	// m_feng.transform(mtx);

    if(m_feng.create_font(typeface, gren, height, 0.0, font_weight, italic, underline, strikeout, ANSI_CHARSET, FF_DONTCARE))
    {
        m_fman.precache(' ', 127);
			
        double x = x0;
        double y = y0;
        const wchar_t* p = text;

        while(*p)
        {
            const agg::glyph_cache* glyph = m_fman.glyph(*p);
            if(glyph)
            {
                m_fman.add_kerning(&x, &y);
 
                m_fman.init_embedded_adaptors(glyph, x, y);

                switch(glyph->data_type)
                {
                case agg::glyph_data_mono:
                    ren_bin.color(color);
                    agg::render_scanlines(m_fman.mono_adaptor(), 
                                            m_fman.mono_scanline(), 
                                            ren_bin);
                    break;

                case agg::glyph_data_gray8:
                    ren_solid.color(color);
                    agg::render_scanlines(m_fman.gray8_adaptor(), 
                                            m_fman.gray8_scanline(), 
                                            ren_solid);
                    break;

                case agg::glyph_data_outline:
                    ras.reset();
                    if(fabs(stroke_weight) <= 0.01)
                    {
						/* 
							Из соображений эффективности не используем
							контурный конвертор, если толщина контура 
							близка к нулю.
						*/
                        //-----------------------
                        ras.add_path(m_curves);
                    }
                    else
                    {
                        ras.add_path(m_contour);
                    }
                    ren_solid.color(color);
						
                    agg::render_scanlines(ras, sl, ren_solid);
                    break;
                }
                    
                // Меняем позицию пера
                x += glyph->advance_x;
                y += glyph->advance_y;
                ++num_glyphs;
            }
            ++p;
        }

		/*
			Подчеркивание и зачеркивание не является частью глифа.
			Это декоративные элементы, информация о которых содержится в шрифте:
			ширина линий подчеркивания и зачеркивания и расположение линий 
			отностительно базовой линии.
		*/

		if (m_feng.metrics_valid() && (m_feng.underline() || m_feng.strikeout())) {
			agg::path_storage ps;
			agg::conv_stroke<agg::path_storage> pg(ps);

			ras.reset();
			if (m_feng.underline()) {
				pg.width(m_feng.underscore_size());
				pg.line_cap(agg::square_cap);
				ps.remove_all();
				ps.move_to(x0+0.5f, y0-m_feng.underscore_position()+0.5f);
				ps.line_to(x+0.5f, y-m_feng.underscore_position()+0.5f);
				ras.add_path(pg);
			}
			if (m_feng.strikeout()) {
				pg.width(m_feng.strikeout_size());
				pg.line_cap(agg::square_cap);
				ps.remove_all();
				ps.move_to(x0+0.5f, y0-m_feng.strikeout_position()+0.5f);
				ps.line_to(x+0.5f, y-m_feng.strikeout_position()+0.5f);
				ras.add_path(pg);
			}
            agg::render_scanlines(ras, sl, ren_solid);
		}
    }
    return num_glyphs;
}

int draw_sstring(surface* surface, font_engine* fe, 
			     float x, float y,
			 	 char* s, 
				 unsigned color, bool use_opacity,
				 wchar_t* fontname, float height, int font_weight,
				 bool italic, bool underline, bool strikeout) {
					 if (surface && fe && s) {
						int buflen = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
						wchar_t* buf = new wchar_t[buflen];
						memset(buf, 0, buflen);
						MultiByteToWideChar(CP_ACP, 0, s, -1, buf, buflen);
						draw_string(surface, fe, x, y, buf, color, use_opacity, fontname, height, font_weight, italic, underline, strikeout);
						delete buf;
						return 0;
					 } 
					 else 
					 { 
						 return 1; 
					 }
}


int draw_string(surface* surface, font_engine* fe, 
			    float x, float y,
				wchar_t* s, 
				unsigned color, bool use_opacity,
				wchar_t* fontname, float height, int font_weight,
				bool italic, bool underline, bool strikeout)
{
	if (surface && fe) {
		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;
		
		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
	    pix_format pixf(rbuf);

		ren_base ren_base(pixf);
		renderer_solid ren_solid(ren_base);
		renderer_bin ren_bin(ren_base);

		agg::scanline_u8 sl;
		agg::rasterizer_scanline_aa<> ras;
	
		internal_draw_text(s, fontname, height, font_weight, 0.0, x, y, italic, underline, strikeout, fill_color, ras, sl, ren_solid, ren_bin, 
			fe->feng, fe->fman, fe->curves, fe->contour);

		return 0;
	} else {
		return 1;
	}
}

void internal_invert_rect(int x0, int y0, int x1, int y1, agg::pixfmt_bgr24& pixf) {
	int x, y;
	int w = x1-x0;
	for(y = y0; y < y0+w; y++) {
		for (x = x0; x < x0+w; x++) {
			agg::rgba8 c = pixf.pixel(x, y);
			agg::rgba8 c1 = agg::rgba8(~c.r, ~c.g, ~c.b, c.a);
			pixf.copy_pixel(x, y, c1);
		}
	}
}

void internal_xor_rect(int x0, int y0, int x1, int y1, agg::pixfmt_bgr24& pixf, agg::rgba8& xr) {
	int x, y;
	int w = x1-x0;
	for(y = y0; y < y0+w; y++) {
		for (x = x0; x < x0+w; x++) {
			agg::rgba8 c = pixf.pixel(x, y);
			agg::rgba8 c1 = agg::rgba8(c.r ^ xr.r, c.g ^ xr.g, c.b ^ xr.b, c.a);
			pixf.copy_pixel(x, y, c1);
		}
	}
}

int invert_rect(surface* surface, int x0, int y0, int x1, int y1) {
	if (surface) {
		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
		pix_format pixf(rbuf);

		agg::rect_i	r1(0, 0, pixf.width()-1, pixf.height()-1);
		agg::rect_i	r2(x0, y0, x1, y1);

		bool b = r1.clip(r2);
		if (!b) return 2;

		internal_invert_rect(r1.x1, r1.y1, r1.x2, r1.x2, pixf);

		return 0;
	} else {
		return 1;
	}
}

int xor_rect(surface* surface, int x0, int y0, int x1, int y1, unsigned color, bool use_opacity) {
	if (surface) {

		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;

		agg::rendering_buffer rbuf;
        rbuf.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
		pix_format pixf(rbuf);

		agg::rect_i	r1(0, 0, pixf.width()-1, pixf.height()-1);
		agg::rect_i	r2(x0, y0, x1, y1);

		bool b = r1.clip(r2);
		if (!b) return 2;

		internal_xor_rect(r1.x1, r1.y1, r1.x2, r1.x2, pixf, fill_color);

		return 0;
	} else {
		return 1;
	}
}


surface* create_surface(int pixel_width, int pixel_height, int fill_color, HDC screenDC)
{
	surface* sf = new surface;
	sf->pmap = new agg::pixel_map();
	sf->pmap->create(pixel_width, pixel_height, agg::org_color24, fill_color);
	return sf;	
}

int delete_surface(surface* surface)
{
	if (surface) {
		delete surface->pmap;
		delete surface;
		surface = NULL;
	}
	return 0;
}

int save_surface(surface* surface, char* filename)
{
	if (surface) {
		surface->pmap->save_as_bmp(filename);
	}
	return 0;
}

font_engine* create_font_engine(HDC dc){
	unsigned int nDPI = GetDeviceCaps(dc, LOGPIXELSX);
	return new font_engine(dc, nDPI);
}

int delete_font_engine(font_engine* fe) {
	if (fe) {
		delete fe;
		return 0;
	} else {
		return 1;
	}
}

int get_text_advice(font_engine* fe, wchar_t* s, double* advicex, double* advicey){
	if (fe) {
		//  предполагается, что при входе сюда шрифт уже выбран,
		// поэтому сразу начинаем считать.
		double x = 0.0;
        double y = 0.0;

		wchar_t* p = s;
		double* ix = advicex;
		double* iy = advicey;
		while (*p){
			const agg::glyph_cache* glyph = fe->fman.glyph(*p);
            if(glyph)
            {
				fe->fman.add_kerning(&x, &y);
				fe->fman.init_embedded_adaptors(glyph, x, y);
                x += glyph->advance_x;
                y += glyph->advance_y;

				*ix = x;
				*iy = y;
			}
			++ix;
			++iy;
			++p;
		}
		return 0;
	} else {
		return 1;
	}
}

int get_stext_advice(font_engine* fe, char* s, double* advicex, double* advicey){
	if (fe && s) {
		int buflen = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
		wchar_t* buf = new wchar_t[buflen];
		memset(buf, 0, buflen);
		MultiByteToWideChar(CP_ACP, 0, s, -1, buf, buflen);
		get_text_advice(fe, buf, advicex, advicey);
		delete buf;
		return 0;
	} else { 
		return 1; 
	}
}

int choose_font(font_engine* fe, 
				wchar_t* fontname, float height, int font_weight, 
				bool italic, bool underline, bool strikeout){
	if (fe) {
		fe->feng.create_font(fontname, agg::glyph_ren_outline, height, 0.0, font_weight, italic, underline, strikeout, ANSI_CHARSET, FF_DONTCARE);
		return 0;
	} else {
		return 1;
	}
}

int scroll(surface* surface, int dx, int dy, unsigned color, bool use_opacity) {
	if (surface) {
		agg::rgba8 fill_color = agg::bgr8_packed(color);
		if (use_opacity) fill_color.a = (color >> 24) & 0xFF; else fill_color.a = 0xFF;
		
		agg::rendering_buffer rsrc;
		agg::rendering_buffer rdst;
        rsrc.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
        rdst.attach(surface->pmap->buf(), surface->pmap->width(), surface->pmap->height(), -surface->pmap->stride());
	    pix_format pixf(rdst);

		agg::rect_i rect_src = agg::rect_i(0, 0, pixf.width() - 1, pixf.height() - 1);
		agg::rect_i rect_dst = agg::rect_i(rect_src);
		rect_dst.x1 += dx;
		rect_dst.x2 += dx;
		rect_dst.y1 += dy;
		rect_dst.y2 += dy;
		bool clip_valid = rect_dst.clip(rect_src);
		rect_src.x1 = rect_dst.x1 - dx;
		rect_src.x2 = rect_dst.x2 - dx;
		rect_src.y1 = rect_dst.y1 - dy;
		rect_src.y2 = rect_dst.y2 - dy;
		
		// Если переменная clip_valid равна false, то заливаем все одним цветом - далеко проскроллились.
		if (clip_valid) {
			int len = rect_dst.x2-rect_dst.x1+1;
			int ystart, ystop;
			ystart = (dy < 0)? rect_src.y1 : rect_src.y2;
			ystop = (dy < 0)? rect_src.y2 : rect_src.y1;

			int delta = (dy < 0)? 1 : -1;
		
			for (int y = ystart; y != ystop; y+=delta) {
				pixf.copy_from(rsrc, rect_dst.x1, y+dy, rect_src.x1, y, len);
			}

			ren_base rbase(pixf);
			typedef agg::renderer_primitives<ren_base> ren_prim;
			ren_prim renp(rbase);

			renp.fill_color(fill_color);

			agg::rect_i r1, r2;
			if (dy > 0) {
				r1.init(0, 0, pixf.width()-1, rect_dst.y1-1);
				r2.y1 = r1.y1;
				r2.y2 = pixf.height();
			} else {
				r1.init(0, rect_dst.y2+1, pixf.width()-1, pixf.height()-1); 
				r2.y1 = 0;
				r2.y2 = r1.y1;
			}

			// Расчет второго прямоугольника
			if (dx > 0)	{
				r2.x1 = 0;
				r2.x2 = rect_dst.x1-1;
			} else {
				r2.x1 = rect_dst.x2+1;
				r2.x2 = pixf.width()-1;
			}

			if (r1.is_valid())
				renp.solid_rectangle(r1.x1, r1.y1, r1.x2, r1.y2);
			if (r2.is_valid())
				renp.solid_rectangle(r2.x1, r2.y1, r2.x2, r2.y2);

		} else {
			ren_base rbase(pixf);
			rbase.clear(fill_color);
		} 

		return 0;
	} else {
		return 1;
	}
}

int show_surface(surface* surface, HDC dc, int sl, int st, int sr, int sb, int dl, int dt, int dr, int db)
{
    RECT rs, rd;
    rs.right  = sr;
    rs.top    = st;
    rs.left   = sl;
    rs.bottom = sb;
    rd.right  = dr;
    rd.top    = dt;
    rd.left   = dl;
    rd.bottom = db;
    if (surface) {
        surface->pmap->draw(dc, &rd, &rs);
        return 0;
	} else {
		return 1;
	}

}

// Необходимости в этой процедуре нет, но она должна быть объявлена, т.к. без нее проект не собирается.
int agg_main(int argc, char* argv[])
{
	return 0;
}