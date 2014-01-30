// Приведенный ниже блок ifdef - это стандартный метод создания макросов, упрощающий процедуру 
// экспорта из библиотек DLL. Все файлы данной DLL скомпилированы с использованием символа AGGLIB_EXPORTS,
// в командной строке. Этот символ не должен быть определен в каком-либо проекте
// использующем данную DLL. Благодаря этому любой другой проект, чьи исходные файлы включают данный файл, видит 
// функции AGGLIB_API как импортированные из DLL, тогда как данная DLL видит символы,
// определяемые данным макросом, как экспортированные.
#ifdef AGGLIB_EXPORTS
#define AGGLIB_API extern "C" __declspec(dllexport)
#else
#define AGGLIB_API
#endif

#define pix_format agg::pixfmt_bgr24

typedef agg::renderer_base<pix_format> ren_base;
typedef agg::renderer_primitives<ren_base> ren_prim;
typedef agg::rasterizer_outline<ren_prim> outline_rasterizer;

typedef agg::renderer_scanline_aa_solid<ren_base> renderer_solid;
typedef agg::renderer_scanline_bin_solid<ren_base> renderer_bin;

typedef agg::font_engine_win32_tt_int32 font_engine_type;
typedef agg::font_cache_manager<font_engine_type> font_manager_type;
typedef agg::conv_curve<font_manager_type::path_adaptor_type> conv_curve_type;
typedef agg::conv_contour<conv_curve_type> conv_contour_type;

typedef struct {
	agg::pixel_map* pmap;
} surface;

class font_engine {
public:
	font_engine_type feng;
    font_manager_type fman;

    conv_curve_type curves;
    conv_contour_type contour;	

	font_engine(HDC dc, unsigned int res) : 
		
		feng(dc),
		fman(feng),
		curves(fman.path_adaptor()),
		contour(curves)
	{
		feng.resolution(res);
	}
};

float eps = 0.0001f;

typedef struct {
	double x;
	int pos;
} fp;
		/** modes for MarkRect **/
#define invert = 0; 
#define hilite = 1; 
#define dim25  = 2; 
#define dim50  = 3; 
#define dim75  = 4;

// Функция создает графическую поверхность, содержащую пиксельный буфер и механизмы его отрисовки.
AGGLIB_API surface* create_surface(int pixel_width, int pixel_height, int fill_color, HDC screenDC);
AGGLIB_API int delete_surface(surface* surface);
AGGLIB_API int save_surface  (surface* surface, char* filename);
AGGLIB_API int draw_line	 (surface* surface, float x0, float y0, float x1, float y1, float w, unsigned color, bool use_opacity);
AGGLIB_API int draw_oval	 (surface* surface, float l, float t, float r, float b, float w, unsigned color, bool use_opacity);
AGGLIB_API int draw_rect	 (surface* surface, float x0, float y0, float x1, float y1, float w, unsigned color, bool use_opacity);
AGGLIB_API int xor_rect		 (surface* surface, int x0, int y0, int x1, int y1, unsigned color, bool use_opacity);
AGGLIB_API int invert_rect	 (surface* surface, int x0, int y0, int x1, int y1);

AGGLIB_API font_engine* create_font_engine(HDC dc);
AGGLIB_API int delete_font_engine(font_engine* fe);

AGGLIB_API int draw_string(surface* surface, font_engine* fe, 
						   float x, float y,
						   wchar_t* s, 
						   unsigned color, bool use_opacity,
						   wchar_t* fontname, float height, int font_weight,
						   bool italic, bool underline, bool strikeout);

AGGLIB_API int draw_sstring(surface* surface, font_engine* fe, 
						   float x, float y,
						   char* s, 
						   unsigned color, bool use_opacity,
						   wchar_t* fontname, float height, int font_weight,
						   bool italic, bool underline, bool strikeout);

AGGLIB_API int choose_font(font_engine* fe, 
			        	   wchar_t* fontname, float height, int font_weight, 
				           bool italic, bool underline, bool strikeout);

AGGLIB_API int get_stext_advice(font_engine* fe, char* s, double* advicex, double* advicey);
AGGLIB_API int get_text_advice(font_engine* fe, wchar_t* s, double* advicex, double* advicey);

AGGLIB_API int scroll(surface* surface, int dx, int dy, unsigned color, bool use_opacity);
AGGLIB_API int show_surface(surface* surface, HDC dc);
