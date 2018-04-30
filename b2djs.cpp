// [B2D-JS]
// Blend2D javascript bindings.
//
// [License]
// Public Domain <http://unlicense.org>

#if !defined(BUILDING_NODE_EXTENSION)
# define BUILDING_NODE_EXTENSION
#endif

#include <algorithm>
#include "./b2djs.h"

namespace b2djs {

// ============================================================================
// [Globals]
// ============================================================================

NJS_ENUM(Enum_Context2D_ImplType,
  b2d::Context2D::kImplTypeNone,
  b2d::Context2D::kImplTypeCount,
  "null\0"
  "pipe2d\0");

NJS_ENUM(Enum_CompOp_Id,
  b2d::CompOp::kSrc,
  b2d::CompOp::kExclusion,
  "src\0"
  "src-over\0"
  "src-in\0"
  "src-out\0"
  "src-atop\0"
  "dst\0"
  "dst-over\0"
  "dst-in\0"
  "dst-out\0"
  "dst-atop\0"
  "xor\0"
  "clear\0"
  "plus\0"
  "merge\0"
  "minus\0"
  "multiply\0"
  "screen\0"
  "overlay\0"
  "darken\0"
  "lighten\0"
  "color-dodge\0"
  "color-burn\0"
  "linear-burn\0"
  "linear-light\0"
  "pin-light\0"
  "hard-light\0"
  "soft-light\0"
  "difference\0"
  "exclusion\0");

NJS_ENUM(Enum_Context2D_StyleType,
  b2d::Context2D::kStyleTypeNone,
  b2d::Context2D::kStyleTypePattern,
  "none\0"
  "solid\0"
  "gradient\0"
  "pattern\0");

NJS_ENUM(Enum_PixelFormat_Id,
  b2d::PixelFormat::kNone,
  b2d::PixelFormat::kA8,
  "none\0"
  "prgb32\0"
  "xrgb32\0"
  "a8\0");

NJS_ENUM(Enum_FontFace_ImplType,
  b2d::FontFace::kImplTypeNone,
  b2d::FontFace::kImplTypeCount,
  "none\0"
  "truetype\0"
  "opentype-cff\0"
  "opentype-cff2\0");

NJS_ENUM(Enum_Image_ImplType,
  b2d::Image::kImplTypeNone,
  b2d::Image::kImplTypeCount,
  "none\0"
  "memory\0"
  "external\0");

NJS_ENUM(Enum_ImageScaler_Filter,
  b2d::ImageScaler::kFilterNone,
  b2d::ImageScaler::kFilterMitchell,
  "none\0"
  "nearest\0"
  "bilinear\0"
  "bicubic\0"
  "bell\0"
  "gauss\0"
  "hermite\0"
  "hanning\0"
  "catrom\0"
  "bessel\0"
  "sinc\0"
  "lanczos\0"
  "blackman\0"
  "mitchell\0");

NJS_ENUM(Enum_FillRule, 0, b2d::FillRule::kCount - 1,
  "non-zero\0"
  "even-odd\0");

NJS_ENUM(Enum_StrokeCap, 0, b2d::StrokeCap::kCount - 1,
  "butt\0"
  "square\0"
  "round\0"
  "round-rev\0"
  "triangle\0"
  "triangle-rev\0");

NJS_ENUM(Enum_StrokeJoin, 0, b2d::StrokeJoin::kCount - 1,
  "miter\0"
  "round\0"
  "bevel\0"
  "miter-rev\0"
  "miter-round\0");

NJS_ENUM(Enum_StrokeTransformOrder, 0, b2d::StrokeTransformOrder::kCount - 1,
  "after\0"
  "before\0");

NJS_ENUM(Enum_Gradient_Extend, 0, b2d::Gradient::kExtendCount - 1,
  "pad\0"
  "repeat\0"
  "reflect\0");

NJS_ENUM(Enum_Pattern_Extend, 0, b2d::Pattern::kExtendCount - 1,
  "pad-x pad-y\0"          "@pad\0"
  "repeat-x repeat-y\0"    "@repeat\0"
  "reflect-x reflect-y\0"  "@reflect\0"
  "pad-x repeat-y\0"
  "pad-x reflect-y\0"
  "repeat-x pad-y\0"
  "repeat-x reflect-y\0"
  "reflect-x pad-y\0"
  "reflect-x repeat-y\0"
);

NJS_ENUM(Enum_Pattern_Filter, 0, b2d::Pattern::kFilterCount - 1,
  "nearest\0"
  "bilinear\0");

// ============================================================================
// [b2djs::ColorConcept]
// ============================================================================

struct ColorConcept {
  enum { kConceptType = njs::Globals::kConceptSerializer };
  typedef uint32_t Type;

  enum { kMaxColorSize = 256 };

  static NJS_INLINE unsigned int hexToInt(unsigned int c) noexcept {
    if (c >= '0' && c <= '9')
      return c - '0';

    c |= 0x20;
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;

    return 0;
  }

  static NJS_INLINE unsigned int intToHex(unsigned int c) noexcept {
    unsigned int adj = c < 10
      ? static_cast<unsigned int>('0')
      : static_cast<unsigned int>('A' - 10);
    return c + adj;
  }

  template<typename CharT>
  static NJS_INLINE uint32_t parse(const CharT* str, size_t size) noexcept {
    if (size >= 4 && str[0] == '#') {
      uint32_t x = hexToInt(str[1]);
      uint32_t y = hexToInt(str[2]);
      uint32_t z = hexToInt(str[3]);

      // Specified as 4-bit components.
      if (size == 4)
        return 0xFF000000u + ((x << 16) + (y << 8) + z) * 0x11u;

      uint32_t w = hexToInt(str[4]);
      if (size == 5)
        return ((x << 24) + (y << 16) + (z << 8) + w) * 0x11u;

      // Specified as 8-bit components.
      if (size >= 7) {
        x = (x << 4) + y;
        y = (z << 4) + w;
        z = (hexToInt(str[5]) << 4) + hexToInt(str[6]);

        if (size == 7)
          return 0xFF000000u + (x << 16) + (y << 8) + z;

        if (size == 9) {
          w = (hexToInt(str[7]) << 4) + hexToInt(str[8]);
          return (x << 24) + (y << 16) + (z << 8) + w;
        }
      }
    }

    return 0;
  }

  template<typename CharT>
  static NJS_INLINE unsigned int stringify(CharT* out, uint32_t color) noexcept {
    unsigned int i = 1;
    out[0] = '#';

    if (!b2d::ArgbUtil::isOpaque32(color)) {
      out[1] = static_cast<CharT>(intToHex((color >> 28) & 0xFu));
      out[2] = static_cast<CharT>(intToHex((color >> 24) & 0xFu));
      i += 2;
    }

    out[i + 0] = static_cast<CharT>(intToHex((color >> 20) & 0xFu));
    out[i + 1] = static_cast<CharT>(intToHex((color >> 16) & 0xFu));
    out[i + 2] = static_cast<CharT>(intToHex((color >> 12) & 0xFu));
    out[i + 3] = static_cast<CharT>(intToHex((color >>  8) & 0xFu));
    out[i + 4] = static_cast<CharT>(intToHex((color >>  4) & 0xFu));
    out[i + 5] = static_cast<CharT>(intToHex((color >>  0) & 0xFu));

    return i + 6;
  }

public:
  NJS_NOINLINE njs::Result serialize(njs::Context& ctx, const uint32_t& in, njs::Value& out) const noexcept {
    uint16_t content[kMaxColorSize];
    unsigned int size = stringify<uint16_t>(content, in);

    out = ctx.newString(njs::Utf16Ref(content, size));
    return njs::resultOf(out);
  }

  NJS_NOINLINE njs::Result deserialize(njs::Context& ctx, const njs::Value& in, uint32_t& out) const noexcept {
    if (in.isNumber()) {
      out = static_cast<uint32_t>(in.doubleValue());
      return njs::Globals::kResultOk;
    }

    if (in.isString()) {
      uint16_t content[kMaxColorSize];
      int size = in.stringLength();

      if (size <= 0 || size > kMaxColorSize || in.readUtf16(content, size) < size)
        return njs::Globals::kResultInvalidValue;

      out = parse<uint16_t>(content, size);
      return njs::Globals::kResultOk;
    }

    return njs::Globals::kResultInvalidValue;
  }
};

// ============================================================================
// [b2djs::BlendJSUtils]
// ============================================================================

struct BlendJSUtils {
  // --------------------------------------------------------------------------
  // [Geometry]
  // --------------------------------------------------------------------------

  union GeomData {
    b2d::Wrap<b2d::Box> box;
    b2d::Wrap<b2d::Rect> rect;
    b2d::Wrap<b2d::Round> round;
    b2d::Wrap<b2d::Circle> circle;
    b2d::Wrap<b2d::Ellipse> ellipse;
    b2d::Wrap<b2d::Arc> arc;
    b2d::Wrap<b2d::Triangle> triangle;
    b2d::Wrap<b2d::CArray<b2d::Point> > poly;
    b2d::Wrap<b2d::Path2D> path;
  };

  static njs::Result unpackGeomArg(njs::FunctionCallContext& ctx, uint32_t& id, GeomData& data, b2d::MemBuffer& mem) noexcept;

  // --------------------------------------------------------------------------
  // [Matrix]
  // --------------------------------------------------------------------------

  union MatrixData {
    b2d::Wrap<b2d::Matrix2D> matrix;
    double d[6];
  };

  static njs::Result unpackMatrixArg(njs::FunctionCallContext& ctx, uint32_t& op, MatrixData& data) noexcept;
};

njs::Result BlendJSUtils::unpackGeomArg(njs::FunctionCallContext& ctx, uint32_t& id, GeomData& data, b2d::MemBuffer& mem) noexcept {
  unsigned int argc = ctx.argumentsLength();
  switch (id) {
    case b2d::kGeomArgBox:
    case b2d::kGeomArgLine: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.box->_x0));
      NJS_CHECK(ctx.unpackArgument(1, data.box->_y0));
      NJS_CHECK(ctx.unpackArgument(2, data.box->_x1));
      NJS_CHECK(ctx.unpackArgument(3, data.box->_y1));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgRect: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.rect->_x));
      NJS_CHECK(ctx.unpackArgument(1, data.rect->_y));
      NJS_CHECK(ctx.unpackArgument(2, data.rect->_w));
      NJS_CHECK(ctx.unpackArgument(3, data.rect->_h));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgCircle: {
      NJS_CHECK(ctx.verifyArgumentsLength(3));
      NJS_CHECK(ctx.unpackArgument(0, data.circle->_center._x));
      NJS_CHECK(ctx.unpackArgument(1, data.circle->_center._y));
      NJS_CHECK(ctx.unpackArgument(2, data.circle->_radius));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgEllipse: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.ellipse->_center._x));
      NJS_CHECK(ctx.unpackArgument(1, data.ellipse->_center._y));
      NJS_CHECK(ctx.unpackArgument(2, data.ellipse->_radius._x));
      NJS_CHECK(ctx.unpackArgument(3, data.ellipse->_radius._y));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgRound: {
      NJS_CHECK(ctx.verifyArgumentsLength(5, 6));

      NJS_CHECK(ctx.unpackArgument(0, data.round->_rect._x));
      NJS_CHECK(ctx.unpackArgument(1, data.round->_rect._y));
      NJS_CHECK(ctx.unpackArgument(2, data.round->_rect._w));
      NJS_CHECK(ctx.unpackArgument(3, data.round->_rect._h));

      NJS_CHECK(ctx.unpackArgument(4, data.round->_radius._x));
      data.round->_radius._y = data.round->_radius._x;

      if (argc == 6)
        NJS_CHECK(ctx.unpackArgument(4, data.round->_radius._y));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgArc:
    case b2d::kGeomArgChord:
    case b2d::kGeomArgPie: {
      NJS_CHECK(ctx.verifyArgumentsLength(5, 6));

      NJS_CHECK(ctx.unpackArgument(0, data.arc->_center._x));
      NJS_CHECK(ctx.unpackArgument(1, data.arc->_center._y));

      NJS_CHECK(ctx.unpackArgument(2, data.arc->_radius._x));
      data.arc->_radius._y = data.arc->_radius._x;

      if (argc == 6)
        NJS_CHECK(ctx.unpackArgument(3, data.arc->_radius._y));

      NJS_CHECK(ctx.unpackArgument(argc - 2, data.arc->_start));
      NJS_CHECK(ctx.unpackArgument(argc - 1, data.arc->_sweep));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgTriangle: {
      NJS_CHECK(ctx.verifyArgumentsLength(6));
      NJS_CHECK(ctx.unpackArgument(0, data.triangle->_p[0]._x));
      NJS_CHECK(ctx.unpackArgument(1, data.triangle->_p[0]._y));
      NJS_CHECK(ctx.unpackArgument(2, data.triangle->_p[1]._x));
      NJS_CHECK(ctx.unpackArgument(3, data.triangle->_p[1]._y));
      NJS_CHECK(ctx.unpackArgument(4, data.triangle->_p[2]._x));
      NJS_CHECK(ctx.unpackArgument(5, data.triangle->_p[2]._y));

      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgPolygon:
    case b2d::kGeomArgPolyline: {
      if (argc < 2 || (argc & 1) == 1)
        return ctx.invalidArgumentsLength();

      size_t nPoly = static_cast<size_t>(argc) / 2;
      double* poly = static_cast<double*>(mem.alloc(static_cast<size_t>(argc) * sizeof(double)));

      if (!poly)
        return njs::Globals::kResultOutOfMemory;

      for (unsigned int i = 0; i < argc; i++)
        NJS_CHECK(ctx.unpackArgument(i, poly[i]));

      data.poly->reset(reinterpret_cast<b2d::Point*>(poly), nPoly);
      return njs::Globals::kResultOk;
    }

    case b2d::kGeomArgPath2D: {
      JSPath2D* path;

      NJS_CHECK(ctx.verifyArgumentsLength(1));
      NJS_CHECK(ctx.unwrapArgument<JSPath2D>(0, &path));

      data.path->_impl = path->_obj.impl();
      return njs::Globals::kResultOk;
    }

    default:
      return njs::Globals::kResultInvalidState;
  }
}

njs::Result BlendJSUtils::unpackMatrixArg(
  njs::FunctionCallContext& ctx, uint32_t& op, MatrixData& data) noexcept {

  unsigned int argc = ctx.argumentsLength();
  switch (op) {
    case b2d::Matrix2D::kOpTranslateP:
    case b2d::Matrix2D::kOpTranslateA:
    case b2d::Matrix2D::kOpSkewP:
    case b2d::Matrix2D::kOpSkewA:
      NJS_CHECK(ctx.verifyArgumentsLength(2));

L_UnpackXY:
      NJS_CHECK(ctx.unpackArgument(0, data.d[0]));
      NJS_CHECK(ctx.unpackArgument(1, data.d[1]));
      return njs::Globals::kResultOk;

    case b2d::Matrix2D::kOpScaleP:
    case b2d::Matrix2D::kOpScaleA:
      if (argc == 2)
        goto L_UnpackXY;

      NJS_CHECK(ctx.verifyArgumentsLength(1));
      NJS_CHECK(ctx.unpackArgument(0, data.d[0]));
      data.d[1] = data.d[0];
      return njs::Globals::kResultOk;

    case b2d::Matrix2D::kOpRotateP:
    case b2d::Matrix2D::kOpRotateA:
      if (argc != 1 && argc != 3)
        return ctx.invalidArgumentsLength();

      NJS_CHECK(ctx.unpackArgument(0, data.d[0]));
      if (argc == 3) {
        NJS_CHECK(ctx.unpackArgument(1, data.d[1]));
        NJS_CHECK(ctx.unpackArgument(2, data.d[2]));
        op += (b2d::Matrix2D::kOpRotatePtP - b2d::Matrix2D::kOpRotateP);
      }

      return njs::Globals::kResultOk;

    case b2d::Matrix2D::kOpMultiplyP:
    case b2d::Matrix2D::kOpMultiplyA:
      NJS_CHECK(ctx.verifyArgumentsLength(6));
      NJS_CHECK(ctx.unpackArgument(0, data.d[0]));
      NJS_CHECK(ctx.unpackArgument(1, data.d[1]));
      NJS_CHECK(ctx.unpackArgument(2, data.d[2]));
      NJS_CHECK(ctx.unpackArgument(3, data.d[3]));
      NJS_CHECK(ctx.unpackArgument(4, data.d[4]));
      NJS_CHECK(ctx.unpackArgument(5, data.d[5]));
      return njs::Globals::kResultOk;

    default:
      return njs::Globals::kResultInvalidState;
  }
}

// ============================================================================
// [b2djs::JSCookie]
// ============================================================================

NJS_BIND_CLASS(JSCookie) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 1) {
      JSCookie* cookie;
      NJS_CHECK(ctx.unwrapArgument<JSCookie>(0, &cookie));
      return ctx.returnNew<JSCookie>(cookie->_obj);
    }
    else {
      return ctx.returnNew<JSCookie>();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.empty());
  }

  NJS_BIND_METHOD(reset) {
    self->_obj.reset();
    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::JSImage]
// ============================================================================

NJS_BIND_CLASS(JSImage) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 1) {
      JSImage* image;
      NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));
      return ctx.returnNew<JSImage>(image->_obj);
    }
    else if (argc == 3) {
      int w, h;
      uint32_t format;

      NJS_CHECK(ctx.unpackArgument(0, w, njs::Range<int>(1, b2d::Image::kMaxSize)));
      NJS_CHECK(ctx.unpackArgument(1, h, njs::Range<int>(1, b2d::Image::kMaxSize)));
      NJS_CHECK(ctx.unpackArgument(2, format, Enum_PixelFormat_Id));

      if (format == b2d::PixelFormat::kNone)
        return ctx.invalidArgumentCustom(2, "Pixel format cannot be 'none'");

      b2d::Image img;
      b2d::Error err = img.create(w, h, format);

      if (err) {
        // TODO: Throw out of memory exception.
      }

      return ctx.returnNew<JSImage>(img);
    }
    else {
      return ctx.returnNew<JSImage>();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(implType) {
    return ctx.returnValue(self->_obj.implType(), Enum_Image_ImplType);
  }

  NJS_BIND_GET(width) {
    return ctx.returnValue(self->_obj.width());
  }

  NJS_BIND_GET(height) {
    return ctx.returnValue(self->_obj.height());
  }

  NJS_BIND_GET(pixelFormat) {
    return ctx.returnValue(self->_obj.pixelFormat(), Enum_PixelFormat_Id);
  }

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.empty());
  }
};

// ============================================================================
// [b2djs::JSPath2D]
// ============================================================================

NJS_BIND_CLASS(JSPath2D) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 0) {
      return ctx.returnNew<JSPath2D>();
    }
    else if (argc == 1) {
      JSPath2D* other;

      NJS_CHECK(ctx.unwrapArgument<JSPath2D>(0, &other));
      return ctx.returnNew<JSPath2D>(other->_obj);
    }
    else {
      return ctx.invalidArgumentsLength();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.empty());
  }

  NJS_BIND_GET(length) {
    return ctx.returnValue(static_cast<uint32_t>(self->_obj.size()));
  }

  NJS_BIND_GET(capacity) {
    return ctx.returnValue(static_cast<uint32_t>(self->_obj.capacity()));
  }

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(clear) {
    self->_obj.reset();
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [MoveTo / LineTo]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(moveTo) { return moveToInternal(ctx, self, false); }
  NJS_BIND_METHOD(moveToRel) { return moveToInternal(ctx, self, true); }

  NJS_BIND_METHOD(lineTo) { return lineToInternal(ctx, self, false); }
  NJS_BIND_METHOD(lineToRel) { return lineToInternal(ctx, self, true); }

  static njs::Result moveToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();
    double x, y;

    // One argument is an object having `x` and `y` properties.
    if (argc == 1) {
      // TODO:
      x = y = 0;
    }
    else if (argc == 2) {
      NJS_CHECK(ctx.unpackArgument(0, x));
      NJS_CHECK(ctx.unpackArgument(1, y));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    if (!isRelative)
      self->_obj.moveTo(x, y);
    else
      self->_obj.moveToRel(x, y);

    return ctx.returnValue(ctx.This());
  }

  static njs::Result lineToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    // One argument is an object having `x` and `y` properties.
    if (argc == 1) {
      // TODO:
    }
    else if (argc >= 2 && (argc & 1) == 0) {
      unsigned int a = 0;
      double points[32];

      do {
        int i = 0;
        int n = std::min<int>(argc - a, static_cast<int>(B2D_ARRAY_SIZE(points)));

        do {
          double x, y;
          NJS_CHECK(ctx.unpackArgument(a + 0, x));
          NJS_CHECK(ctx.unpackArgument(a + 1, y));

          points[i + 0] = x;
          points[i + 1] = y;

          a += 2;
          i += 2;
        } while (i < n);

        if (!isRelative)
          self->_obj.polyTo(reinterpret_cast<b2d::Point*>(points), n >> 1);
        else
          self->_obj.polyToRel(reinterpret_cast<b2d::Point*>(points), n >> 1);
      } while (a < argc);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [QuadTo]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(quadTo) { return quadToInternal(ctx, self, false); }
  NJS_BIND_METHOD(quadToRel) { return quadToInternal(ctx, self, true); }

  NJS_BIND_METHOD(smoothQuadTo) { return smoothQuadToInternal(ctx, self, false); }
  NJS_BIND_METHOD(smoothQuadToRel) { return smoothQuadToInternal(ctx, self, true); }

  static njs::Result quadToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 4) {
      double x1, y1, x2, y2;

      NJS_CHECK(ctx.unpackArgument(0, x1));
      NJS_CHECK(ctx.unpackArgument(1, y1));
      NJS_CHECK(ctx.unpackArgument(2, x2));
      NJS_CHECK(ctx.unpackArgument(3, y2));

      if (!isRelative)
        self->_obj.quadTo(x1, y1, x2, y2);
      else
        self->_obj.quadToRel(x1, y1, x2, y2);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  static njs::Result smoothQuadToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 2) {
      double x2, y2;

      NJS_CHECK(ctx.unpackArgument(0, x2));
      NJS_CHECK(ctx.unpackArgument(1, y2));

      if (!isRelative)
        self->_obj.smoothQuadTo(x2, y2);
      else
        self->_obj.smoothQuadToRel(x2, y2);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [CubicTo]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(cubicTo) { return cubicToInternal(ctx, self, false); }
  NJS_BIND_METHOD(cubicToRel) { return cubicToInternal(ctx, self, true); }

  NJS_BIND_METHOD(smoothCubicTo) { return smoothCubicToInternal(ctx, self, false); }
  NJS_BIND_METHOD(smoothCubicToRel) { return smoothCubicToInternal(ctx, self, true); }

  static njs::Result cubicToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 6) {
      double x1, y1, x2, y2, x3, y3;

      NJS_CHECK(ctx.unpackArgument(0, x1));
      NJS_CHECK(ctx.unpackArgument(1, y1));
      NJS_CHECK(ctx.unpackArgument(2, x2));
      NJS_CHECK(ctx.unpackArgument(3, y2));
      NJS_CHECK(ctx.unpackArgument(4, x3));
      NJS_CHECK(ctx.unpackArgument(5, y3));

      if (!isRelative)
        self->_obj.cubicTo(x1, y1, x2, y2, x3, y3);
      else
        self->_obj.cubicToRel(x1, y1, x2, y2, x3, y3);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  static njs::Result smoothCubicToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 4) {
      double x2, y2, x3, y3;

      NJS_CHECK(ctx.unpackArgument(0, x2));
      NJS_CHECK(ctx.unpackArgument(1, y2));
      NJS_CHECK(ctx.unpackArgument(2, x3));
      NJS_CHECK(ctx.unpackArgument(3, y3));

      if (!isRelative)
        self->_obj.smoothCubicTo(x2, y2, x3, y3);
      else
        self->_obj.smoothCubicToRel(x2, y2, x3, y3);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [ArcTo / EllipticArcTo]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(arcTo) { return arcToInternal(ctx, self, false); }
  NJS_BIND_METHOD(arcToRel) { return arcToInternal(ctx, self, true); }

  NJS_BIND_METHOD(ellipticArcTo) { return ellipticArcToInternal(ctx, self, false); }
  NJS_BIND_METHOD(ellipticArcToRel) { return ellipticArcToInternal(ctx, self, true); }

  static njs::Result arcToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc >= 5) {
      double cx, cy, rx, ry, start, sweep;
      bool startPath = false;

      if (ctx.unpackArgument(argc - 1, startPath) == njs::Globals::kResultOk && --argc < 5)
        return ctx.invalidArgumentsLength();

      NJS_CHECK(ctx.unpackArgument(0, cx));
      NJS_CHECK(ctx.unpackArgument(1, cy));
      NJS_CHECK(ctx.unpackArgument(2, rx));

      if (argc == 5)
        ry = rx;
      else
        NJS_CHECK(ctx.unpackArgument(3, ry));

      NJS_CHECK(ctx.unpackArgument(argc - 2, start));
      NJS_CHECK(ctx.unpackArgument(argc - 1, sweep));

      if (!isRelative)
        self->_obj.arcTo(b2d::Point(cx, cy), b2d::Point(rx, ry), start, sweep, startPath);
      else
        self->_obj.arcToRel(b2d::Point(cx, cy), b2d::Point(rx, ry), start, sweep, startPath);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  static njs::Result ellipticArcToInternal(njs::FunctionCallContext& ctx, JSPath2D* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 7) {
      double rx, ry, angle, x, y;
      bool largeArcFlag, sweepFlag;

      NJS_CHECK(ctx.unpackArgument(0, rx));
      NJS_CHECK(ctx.unpackArgument(1, ry));
      NJS_CHECK(ctx.unpackArgument(2, angle));
      NJS_CHECK(ctx.unpackArgument(3, largeArcFlag));
      NJS_CHECK(ctx.unpackArgument(4, sweepFlag));
      NJS_CHECK(ctx.unpackArgument(5, x));
      NJS_CHECK(ctx.unpackArgument(6, y));

      if (!isRelative)
        self->_obj.svgArcTo(b2d::Point(rx, ry), angle, largeArcFlag, sweepFlag, b2d::Point(x, y));
      else
        self->_obj.svgArcToRel(b2d::Point(rx, ry), angle, largeArcFlag, sweepFlag, b2d::Point(x, y));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Close]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(close) {
    self->_obj.close();
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(addBox     ) { return addArg(ctx, self, b2d::kGeomArgBox     ); }
  NJS_BIND_METHOD(addRect    ) { return addArg(ctx, self, b2d::kGeomArgRect    ); }
  NJS_BIND_METHOD(addCircle  ) { return addArg(ctx, self, b2d::kGeomArgCircle  ); }
  NJS_BIND_METHOD(addEllipse ) { return addArg(ctx, self, b2d::kGeomArgEllipse ); }
  NJS_BIND_METHOD(addRound   ) { return addArg(ctx, self, b2d::kGeomArgRound   ); }
  NJS_BIND_METHOD(addChord   ) { return addArg(ctx, self, b2d::kGeomArgChord   ); }
  NJS_BIND_METHOD(addPie     ) { return addArg(ctx, self, b2d::kGeomArgPie     ); }
  NJS_BIND_METHOD(addTriangle) { return addArg(ctx, self, b2d::kGeomArgTriangle); }
  NJS_BIND_METHOD(addPolygon ) { return addArg(ctx, self, b2d::kGeomArgPolygon ); }
  NJS_BIND_METHOD(addPath    ) { return addArg(ctx, self, b2d::kGeomArgPath2D  ); }

  static njs::Result addArg(njs::FunctionCallContext& ctx, JSPath2D* self, uint32_t id) noexcept {
    BlendJSUtils::GeomData data;
    b2d::MemBufferTmp<1024> mem;

    NJS_CHECK(BlendJSUtils::unpackGeomArg(ctx, id, data, mem));
    self->_obj._addArg(id, &data, nullptr, b2d::Path2D::kDirCW);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::JSPattern]
// ============================================================================

NJS_BIND_CLASS(JSPattern) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc != 1 && argc != 5)
      return ctx.invalidArgumentsLength();

    JSImage* image;
    NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));

    int iw = image->_obj.width();
    int ih = image->_obj.height();

    if (iw == 0)
      return ctx.returnValue(ctx.This());

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    if (argc == 5) {
      NJS_CHECK(ctx.unpackArgument(1, x, njs::Range<int>(0, iw - 1)));
      NJS_CHECK(ctx.unpackArgument(2, y, njs::Range<int>(0, ih - 1)));
      NJS_CHECK(ctx.unpackArgument(3, w, njs::Range<int>(1, iw - x)));
      NJS_CHECK(ctx.unpackArgument(4, h, njs::Range<int>(1, ih - y)));
    }
    else {
      w = iw;
      h = ih;
    }

    b2d::Pattern pattern(image->_obj, b2d::IntRect(x, y, w, h));
    return ctx.returnNew<JSPattern>(pattern);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(extend) {
    return ctx.returnValue(self->_obj.extend(), Enum_Pattern_Extend);
  }

  NJS_BIND_SET(extend) {
    uint32_t extend;
    NJS_CHECK(ctx.unpackValue(extend, Enum_Pattern_Extend));
    self->_obj.setExtend(static_cast<uint32_t>(extend));
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Matrix]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(setMatrix) {
    unsigned int argc = ctx.argumentsLength();
    double a, b, c, d, e, f;

    if (argc == 6) {
      NJS_CHECK(ctx.unpackArgument(0, a));
      NJS_CHECK(ctx.unpackArgument(1, b));
      NJS_CHECK(ctx.unpackArgument(2, c));
      NJS_CHECK(ctx.unpackArgument(3, d));
      NJS_CHECK(ctx.unpackArgument(4, e));
      NJS_CHECK(ctx.unpackArgument(5, f));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    self->_obj.setMatrix(b2d::Matrix2D(a, b, c, d, e, f));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(resetMatrix) {
    self->_obj.resetMatrix();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(translate      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateP); }
  NJS_BIND_METHOD(translateAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateA); }
  NJS_BIND_METHOD(scale          ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleP    ); }
  NJS_BIND_METHOD(scaleAppend    ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleA    ); }
  NJS_BIND_METHOD(skew           ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewP     ); }
  NJS_BIND_METHOD(skewAppend     ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewA     ); }
  NJS_BIND_METHOD(rotate         ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateP   ); }
  NJS_BIND_METHOD(rotateAppend   ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateA   ); }
  NJS_BIND_METHOD(transform      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyP ); }
  NJS_BIND_METHOD(transformAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyA ); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, JSPattern* self, uint32_t op) noexcept {
    BlendJSUtils::MatrixData data;

    NJS_CHECK(BlendJSUtils::unpackMatrixArg(ctx, op, data));
    self->_obj._matrixOp(op, &data);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::JSGradient]
// ============================================================================

NJS_BIND_CLASS(JSGradient) {
  NJS_ABSTRACT_CONSTRUCTOR()

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.stopCount() == 0);
  }

  NJS_BIND_GET(length) {
    return ctx.returnValue(self->_obj.stopCount());
  }

  NJS_BIND_GET(extend) {
    return ctx.returnValue(self->_obj.extend(), Enum_Gradient_Extend);
  }

  NJS_BIND_SET(extend) {
    uint32_t extend;
    NJS_CHECK(ctx.unpackValue(extend, Enum_Gradient_Extend));
    self->_obj.setExtend(static_cast<uint32_t>(extend));
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(addStop) {
    unsigned int argc = ctx.argumentsLength();

    double offset;
    uint32_t color;

    if (argc == 2) {
      NJS_CHECK(ctx.unpackArgument(0, offset));
      NJS_CHECK(ctx.unpackArgument(1, color, ColorConcept()));
    }
    else {
      return ctx.invalidArgumentsLength(2);
    }

    self->_obj.addStop(offset, b2d::Argb32(color));
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Matrix]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(setMatrix) {
    unsigned int argc = ctx.argumentsLength();
    double a, b, c, d, e, f;

    if (argc == 6) {
      NJS_CHECK(ctx.unpackArgument(0, a));
      NJS_CHECK(ctx.unpackArgument(1, b));
      NJS_CHECK(ctx.unpackArgument(2, c));
      NJS_CHECK(ctx.unpackArgument(3, d));
      NJS_CHECK(ctx.unpackArgument(4, e));
      NJS_CHECK(ctx.unpackArgument(5, f));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    self->_obj.setMatrix(b2d::Matrix2D(a, b, c, d, e, f));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(resetMatrix) {
    self->_obj.resetMatrix();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(translate      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateP); }
  NJS_BIND_METHOD(translateAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateA); }
  NJS_BIND_METHOD(scale          ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleP    ); }
  NJS_BIND_METHOD(scaleAppend    ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleA    ); }
  NJS_BIND_METHOD(skew           ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewP     ); }
  NJS_BIND_METHOD(skewAppend     ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewA     ); }
  NJS_BIND_METHOD(rotate         ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateP   ); }
  NJS_BIND_METHOD(rotateAppend   ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateA   ); }
  NJS_BIND_METHOD(transform      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyP ); }
  NJS_BIND_METHOD(transformAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyA ); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, JSGradient* self, uint32_t op) noexcept {
    BlendJSUtils::MatrixData data;

    NJS_CHECK(BlendJSUtils::unpackMatrixArg(ctx, op, data));
    self->_obj._matrixOp(op, &data);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::JSLinearGradient]
// ============================================================================

NJS_BIND_CLASS(JSLinearGradient) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 0) {
      return ctx.returnNew<JSLinearGradient>();
    }
    else if (argc == 4) {
      double x0, y0, x1, y1;
      NJS_CHECK(ctx.unpackArgument(0, x0));
      NJS_CHECK(ctx.unpackArgument(1, y0));
      NJS_CHECK(ctx.unpackArgument(2, x1));
      NJS_CHECK(ctx.unpackArgument(3, y1));
      return ctx.returnNew<JSLinearGradient>(x0, y0, x1, y1);
    }
    else {
      return ctx.invalidArgumentsLength();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(x0) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdLinearX0));
  }

  NJS_BIND_SET(x0) {
    double x0;
    NJS_CHECK(ctx.unpackValue(x0));
    self->_obj.setScalar(b2d::Gradient::kScalarIdLinearX0, x0);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(y0) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdLinearY0));
  }

  NJS_BIND_SET(y0) {
    double y0;
    NJS_CHECK(ctx.unpackValue(y0));
    self->_obj.setScalar(b2d::Gradient::kScalarIdLinearY0, y0);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(x1) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdLinearX1));
  }

  NJS_BIND_SET(x1) {
    double x1;
    NJS_CHECK(ctx.unpackValue(x1));
    self->_obj.setScalar(b2d::Gradient::kScalarIdLinearX1, x1);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(y1) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdLinearY1));
  }

  NJS_BIND_SET(y1) {
    double y1;
    NJS_CHECK(ctx.unpackValue(y1));
    self->_obj.setScalar(b2d::Gradient::kScalarIdLinearY1, y1);
    return njs::Globals::kResultOk;
  }
};

// ============================================================================
// [b2djs::JSRadialGradient]
// ============================================================================

NJS_BIND_CLASS(JSRadialGradient) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 0) {
      return ctx.returnNew<JSRadialGradient>();
    }
    else if (argc == 5) {
      double cx, cy, fx, fy, cr;
      NJS_CHECK(ctx.unpackArgument(0, cx));
      NJS_CHECK(ctx.unpackArgument(1, cy));
      NJS_CHECK(ctx.unpackArgument(2, fx));
      NJS_CHECK(ctx.unpackArgument(3, fy));
      NJS_CHECK(ctx.unpackArgument(4, cr));
      return ctx.returnNew<JSRadialGradient>(cx, cy, fx, fy, cr);
    }
    else {
      return ctx.invalidArgumentsLength();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(cx) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdRadialCx));
  }

  NJS_BIND_SET(cx) {
    double cx;
    NJS_CHECK(ctx.unpackValue(cx));
    self->_obj.setScalar(b2d::Gradient::kScalarIdRadialCx, cx);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(cy) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdRadialCy));
  }

  NJS_BIND_SET(cy) {
    double cy;
    NJS_CHECK(ctx.unpackValue(cy));
    self->_obj.setScalar(b2d::Gradient::kScalarIdRadialCy, cy);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(fx) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdRadialFx));
  }

  NJS_BIND_SET(fx) {
    double fx;
    NJS_CHECK(ctx.unpackValue(fx));
    self->_obj.setScalar(b2d::Gradient::kScalarIdRadialFx, fx);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(fy) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdRadialFy));
  }

  NJS_BIND_SET(fy) {
    double fy;
    NJS_CHECK(ctx.unpackValue(fy));
    self->_obj.setScalar(b2d::Gradient::kScalarIdRadialFy, fy);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(cr) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdRadialCr));
  }

  NJS_BIND_SET(cr) {
    double cr;
    NJS_CHECK(ctx.unpackValue(cr));
    self->_obj.setScalar(b2d::Gradient::kScalarIdRadialCr, cr);
    return njs::Globals::kResultOk;
  }
};

// ============================================================================
// [b2djs::JSConicalGradient]
// ============================================================================

NJS_BIND_CLASS(JSConicalGradient) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 0) {
      return ctx.returnNew<JSConicalGradient>();
    }
    else if (argc == 3) {
      double cx, cy, angle;
      NJS_CHECK(ctx.unpackArgument(0, cx));
      NJS_CHECK(ctx.unpackArgument(1, cy));
      NJS_CHECK(ctx.unpackArgument(2, angle));
      return ctx.returnNew<JSConicalGradient>(cx, cy, angle);
    }
    else {
      return ctx.invalidArgumentsLength();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(cx) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdConicalCx));
  }

  NJS_BIND_SET(cx) {
    double cx;
    NJS_CHECK(ctx.unpackValue(cx));
    self->_obj.setScalar(b2d::Gradient::kScalarIdConicalCx, cx);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(cy) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdConicalCy));
  }

  NJS_BIND_SET(cy) {
    double cy;
    NJS_CHECK(ctx.unpackValue(cy));
    self->_obj.setScalar(b2d::Gradient::kScalarIdConicalCy, cy);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(angle) {
    return ctx.returnValue(self->_obj.scalar(b2d::Gradient::kScalarIdConicalAngle));
  }

  NJS_BIND_SET(angle) {
    double angle;
    NJS_CHECK(ctx.unpackValue(angle));
    self->_obj.setScalar(b2d::Gradient::kScalarIdConicalAngle, angle);
    return njs::Globals::kResultOk;
  }
};

// ============================================================================
// [b2djs::JSFontFace]
// ============================================================================

static njs::Result storeDesignMetrics(njs::ExecutionContext& ctx, njs::Value& dst, const b2d::FontDesignMetrics& dm) noexcept {
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("ascent"                )), ctx.newValue(dm.ascent())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("descent"               )), ctx.newValue(dm.descent())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("lineGap"               )), ctx.newValue(dm.lineGap())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("xHeight"               )), ctx.newValue(dm.xHeight())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("capHeight"             )), ctx.newValue(dm.capHeight())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vertAscent"            )), ctx.newValue(dm.vertAscent())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vertDescent"           )), ctx.newValue(dm.vertDescent())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("minLeftSideBearing"    )), ctx.newValue(dm.minLeftSideBearing())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("minRightSideBearing"   )), ctx.newValue(dm.minRightSideBearing())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("minTopSideBearing"     )), ctx.newValue(dm.minTopSideBearing())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("minBottomSideBearing"  )), ctx.newValue(dm.minBottomSideBearing())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("maxAdvanceWidth"       )), ctx.newValue(dm.maxAdvanceWidth())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("maxAdvanceHeight"      )), ctx.newValue(dm.maxAdvanceHeight())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("underlinePosition"     )), ctx.newValue(dm.underlinePosition())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("underlineThickness"    )), ctx.newValue(dm.underlineThickness())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("strikethroughPosition" )), ctx.newValue(dm.strikethroughPosition())));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("strikethroughThickness")), ctx.newValue(dm.strikethroughThickness())));

  return njs::Globals::kResultOk;
}

static njs::Result storeDiagnosticsInfo(njs::ExecutionContext& ctx, njs::Value& dst, const b2d::FontDiagnosticsInfo& info) noexcept {
  #define ADD_DIAG_FLAG(NAME, FLAG)                                                   \
    ctx.setProperty(dst,                                                              \
                    ctx.newInternalizedString(njs::Latin1Ref(NAME)),                  \
                    ctx.newValue((info.flags() & b2d::FontDiagnosticsInfo::FLAG) != 0))

  NJS_CHECK(ADD_DIAG_FLAG("cmapDefectiveData"    , kFlagCMapDefectiveData));
  NJS_CHECK(ADD_DIAG_FLAG("cmapUnsupportedFormat", kFlagCMapUnsupportedFormat));
  NJS_CHECK(ADD_DIAG_FLAG("kernUnsortedPairs"    , kFlagKernUnsortedPairs));
  NJS_CHECK(ADD_DIAG_FLAG("kernTruncatedData"    , kFlagKernTruncatedData));
  NJS_CHECK(ADD_DIAG_FLAG("kernUnknownFormat"    , kFlagKernUnknownFormat));
  NJS_CHECK(ADD_DIAG_FLAG("nameDefectiveData"    , kFlagNameDefectiveData));
  NJS_CHECK(ADD_DIAG_FLAG("nameRedundantFamily"  , kFlagNameRedundantSubfamily));

  #undef ADD_DIAG_FLAG

  return njs::Globals::kResultOk;
}

NJS_BIND_CLASS(JSFontFace) {
  NJS_BIND_CONSTRUCTOR() {
    return ctx.returnNew<JSFontFace>();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(implType) {
    return ctx.returnValue(self->_obj.implType(), Enum_FontFace_ImplType);
  }

  NJS_BIND_GET(faceIndex) {
    return ctx.returnValue(self->_obj.faceIndex());
  }

  NJS_BIND_GET(unitsPerEm) {
    return ctx.returnValue(self->_obj.unitsPerEm());
  }

  NJS_BIND_GET(glyphCount) {
    return ctx.returnValue(self->_obj.glyphCount());
  }

  NJS_BIND_GET(weight) {
    return ctx.returnValue(self->_obj.weight());
  }

  NJS_BIND_GET(stretch) {
    return ctx.returnValue(self->_obj.stretch());
  }

  NJS_BIND_GET(fullName) {
    njs::Value str = ctx.newString(njs::Utf8Ref(self->_obj.fullName(), self->_obj.fullNameSize()));
    NJS_CHECK(str);
    return ctx.returnValue(str);
  }

  NJS_BIND_GET(familyName) {
    njs::Value str = ctx.newString(njs::Utf8Ref(self->_obj.familyName(), self->_obj.familyNameSize()));
    NJS_CHECK(str);
    return ctx.returnValue(str);
  }

  NJS_BIND_GET(subfamilyName) {
    njs::Value str = ctx.newString(njs::Utf8Ref(self->_obj.subfamilyName(), self->_obj.subfamilyNameSize()));
    NJS_CHECK(str);
    return ctx.returnValue(str);
  }

  NJS_BIND_GET(postScriptName) {
    njs::Value str = ctx.newString(njs::Utf8Ref(self->_obj.postScriptName(), self->_obj.postScriptNameSize()));
    NJS_CHECK(str);
    return ctx.returnValue(str);
  }

  NJS_BIND_GET(diagnosticsInfo) {
    njs::Value dmObj = ctx.newObject();
    NJS_CHECK(dmObj);
    NJS_CHECK(storeDiagnosticsInfo(ctx, dmObj, self->_obj.diagnosticsInfo()));
    return ctx.returnValue(dmObj);
  }

  NJS_BIND_GET(designMetrics) {
    njs::Value dmObj = ctx.newObject();
    NJS_CHECK(dmObj);
    NJS_CHECK(storeDesignMetrics(ctx, dmObj, self->_obj.designMetrics()));
    return ctx.returnValue(dmObj);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(createFromFile) {
    int argc = ctx.argumentsLength();
    if (argc != 1)
      return ctx.invalidArgumentsLength(1);

    njs::Value arg = ctx.argumentAt(0);
    if (!arg.isString())
      return ctx.invalidArgument(0);

    char fileName[1024];
    int size = arg.readUtf8(fileName, sizeof(fileName) - 1);
    fileName[size] = '\0';

    b2d::Error err = self->_obj.createFromFile(fileName);
    return ctx.returnValue(err);
  }

  NJS_BIND_METHOD(listTags) {
    b2d::Array<uint32_t> tags;
    self->_obj.listTags(tags);

    uint32_t count = unsigned(tags.size());
    char tagData[8] = { 0 };

    njs::Value arr = ctx.newArray();
    NJS_CHECK(arr);

    for (uint32_t i = 0; i < count; i++) {
      std::memcpy(tagData, &tags[i], 4);

      njs::Value s = ctx.newString(njs::Latin1Ref(tagData));
      NJS_CHECK(s);
      NJS_CHECK(ctx.setPropertyAt(arr, i, s));
    }

    return ctx.returnValue(arr);
  }
};

// ============================================================================
// [b2djs::JSFont]
// ============================================================================

NJS_BIND_CLASS(JSFont) {
  NJS_BIND_CONSTRUCTOR() {
    return ctx.returnNew<JSFont>();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(face) {
    njs::Value FontFaceClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("FontFace"));
    njs::Value obj = ctx.newInstance(FontFaceClass);

    JSFontFace* wrap = ctx.unwrapUnsafe<JSFontFace>(obj);
    wrap->_obj = self->_obj.face();
    return ctx.returnValue(obj);
  }

  NJS_BIND_GET(size) {
    return ctx.returnValue(self->_obj.size());
  }

  NJS_BIND_GET(unitsPerEm) {
    return ctx.returnValue(self->_obj.unitsPerEm());
  }

  NJS_BIND_GET(designMetrics) {
    njs::Value dmObj = ctx.newObject();
    NJS_CHECK(dmObj);
    NJS_CHECK(storeDesignMetrics(ctx, dmObj, self->_obj.designMetrics()));
    return ctx.returnValue(dmObj);
  }

  NJS_BIND_GET(metrics) {
    const b2d::FontMetrics& m = self->_obj.metrics();

    njs::Value mObj = ctx.newObject();
    NJS_CHECK(mObj);
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("ascent"                )), ctx.newValue(m.ascent())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("descent"               )), ctx.newValue(m.descent())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("lineGap"               )), ctx.newValue(m.lineGap())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("xHeight"               )), ctx.newValue(m.xHeight())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("capHeight"             )), ctx.newValue(m.capHeight())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("vertAscent"            )), ctx.newValue(m.vertAscent())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("vertDescent"           )), ctx.newValue(m.vertDescent())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("underlinePosition"     )), ctx.newValue(m.underlinePosition())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("underlineThickness"    )), ctx.newValue(m.underlineThickness())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("strikethroughPosition" )), ctx.newValue(m.strikethroughPosition())));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("strikethroughThickness")), ctx.newValue(m.strikethroughThickness())));

    return ctx.returnValue(mObj);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(createFromFace) {
    int argc = ctx.argumentsLength();
    if (argc != 2)
      return ctx.invalidArgumentsLength(2);

    JSFontFace* face;
    NJS_CHECK(ctx.unwrapArgument<JSFontFace>(0, &face));

    double size;
    NJS_CHECK(ctx.unpackArgument(1, size));

    b2d::Error err = self->_obj.createFromFace(face->_obj, size);
    return ctx.returnValue(err);
  }
};

// ============================================================================
// [b2djs::JSContext2D]
// ============================================================================

NJS_BIND_CLASS(JSContext2D) {
  NJS_BIND_CONSTRUCTOR() {
    NJS_CHECK(ctx.verifyArgumentsLength(0, 1));
    unsigned int argc = ctx.argumentsLength();

    if (argc == 1) {
      JSImage* image;
      NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));
      return ctx.returnNew<JSContext2D>(image->_obj);
    }
    else {
      return ctx.returnNew<JSContext2D>();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(isNone) {
    return ctx.returnValue(self->_obj.isNone());
  }

  NJS_BIND_GET(implType) {
    return ctx.returnValue(self->_obj.implType(), Enum_Context2D_ImplType);
  }

  NJS_BIND_GET(targetWidth) {
    return ctx.returnValue(self->_obj.impl()->_targetSizeD._w);
  }

  NJS_BIND_GET(targetHeight) {
    return ctx.returnValue(self->_obj.impl()->_targetSizeD._h);
  }

  // --------------------------------------------------------------------------
  // [Begin / End]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(begin) {
    NJS_CHECK(ctx.verifyArgumentsLength(1));

    JSImage* image;
    NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));

    self->_obj.begin(image->_obj);
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(end) {
    self->_obj.end();
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Global Params]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(tolerance) {
    double tolerance = 0.0;
    self->_obj.getTolerance(tolerance);
    return ctx.returnValue(tolerance);
  }

  NJS_BIND_SET(tolerance) {
    double tolerance;
    NJS_CHECK(ctx.unpackValue(tolerance));
    self->_obj.setTolerance(tolerance);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(compOp) {
    uint32_t compOp;
    self->_obj.getCompOp(compOp);
    return ctx.returnValue(compOp, Enum_CompOp_Id);
  }

  NJS_BIND_SET(compOp) {
    uint32_t compOp;
    NJS_CHECK(ctx.unpackValue(compOp, Enum_CompOp_Id));
    self->_obj.setCompOp(compOp);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(alpha) {
    double alpha = 0.0;
    self->_obj.getAlpha(alpha);
    return ctx.returnValue(alpha);
  }

  NJS_BIND_SET(alpha) {
    double alpha;
    NJS_CHECK(ctx.unpackValue(alpha));
    self->_obj.setAlpha(alpha);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(patternFilter) {
    uint32_t filter;
    self->_obj.getPatternFilter(filter);
    return ctx.returnValue(filter, Enum_Pattern_Filter);
  }

  NJS_BIND_SET(patternFilter) {
    int filter;
    NJS_CHECK(ctx.unpackValue(filter, Enum_Pattern_Filter));
    self->_obj.setPatternFilter(static_cast<uint32_t>(filter));
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Fill & Stroke Alpha]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(fillAlpha) {
    double alpha = 0.0;
    self->_obj.getFillAlpha(alpha);
    return ctx.returnValue(alpha);
  }

  NJS_BIND_SET(fillAlpha) {
    double alpha;
    NJS_CHECK(ctx.unpackValue(alpha));
    self->_obj.setFillAlpha(alpha);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeAlpha) {
    double alpha = 0.0;
    self->_obj.getStrokeAlpha(alpha);
    return ctx.returnValue(alpha);
  }

  NJS_BIND_SET(strokeAlpha) {
    double alpha;
    NJS_CHECK(ctx.unpackValue(alpha));
    self->_obj.setStrokeAlpha(alpha);
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Fill & Stroke Style]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(fillStyle) { return getStyle(ctx, self, b2d::Context2D::kStyleSlotFill); }
  NJS_BIND_SET(fillStyle) { return setStyle(ctx, self, b2d::Context2D::kStyleSlotFill); }

  NJS_BIND_GET(strokeStyle) { return getStyle(ctx, self, b2d::Context2D::kStyleSlotStroke); }
  NJS_BIND_SET(strokeStyle) { return setStyle(ctx, self, b2d::Context2D::kStyleSlotStroke); }

  static njs::Result getStyle(njs::GetPropertyContext& ctx, JSContext2D* self, uint32_t slot) noexcept {
    b2d::Argb32 color;

    if (self->_obj.getStyle(slot, color) == b2d::kErrorOk)
      return ctx.returnValue(color.value(), ColorConcept());

    return ctx.returnValue(njs::Undefined);
  }

  static njs::Result setStyle(njs::SetPropertyContext& ctx, JSContext2D* self, uint32_t slot) noexcept {
    njs::Value value = ctx.propertyValue();

    if (value.isNumber()) {
      double d = value.doubleValue();
      uint32_t color = static_cast<uint32_t>(d);
      self->_obj.setStyle(slot, b2d::Argb32(color));
    }
    else if (value.isString()) {
      uint32_t color;
      NJS_CHECK(ctx.unpack(value, color, ColorConcept()));
      self->_obj.setStyle(slot, b2d::Argb32(color));
    }
    else if (ctx.isWrapped<JSGradient>(value)) {
      JSGradient* style = ctx.unwrapUnsafe<JSGradient>(value);
      self->_obj.setStyle(slot, style->_obj);
    }
    else if (ctx.isWrapped<JSPattern>(value)) {
      JSPattern* style = ctx.unwrapUnsafe<JSPattern>(value);
      self->_obj.setStyle(slot, style->_obj);
    }
    else {
      return njs::Globals::kResultInvalidValue;
    }

    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Fill / Stroke Params]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(fillRule) {
    uint32_t fillRule;
    self->_obj.getFillRule(fillRule);
    return ctx.returnValue(fillRule, Enum_FillRule);
  }

  NJS_BIND_SET(fillRule) {
    uint32_t fillRule;
    NJS_CHECK(ctx.unpackValue(fillRule, Enum_FillRule));
    self->_obj.setFillRule(static_cast<uint32_t>(fillRule));
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeWidth) {
    double strokeWidth = 0.0;
    self->_obj.getStrokeWidth(strokeWidth);
    return ctx.returnValue(strokeWidth);
  }

  NJS_BIND_SET(strokeWidth) {
    double strokeWidth;
    NJS_CHECK(ctx.unpackValue(strokeWidth));
    self->_obj.setStrokeWidth(strokeWidth);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(miterLimit) {
    double miterLimit = 0.0;
    self->_obj.getMiterLimit(miterLimit);
    return ctx.returnValue(miterLimit);
  }

  NJS_BIND_SET(miterLimit) {
    double miterLimit;
    NJS_CHECK(ctx.unpackValue(miterLimit));
    self->_obj.setMiterLimit(miterLimit);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(dashOffset) {
    double dashOffset = 0.0;
    self->_obj.getDashOffset(dashOffset);
    return ctx.returnValue(dashOffset);
  }

  NJS_BIND_SET(dashOffset) {
    double dashOffset;
    NJS_CHECK(ctx.unpackValue(dashOffset));
    self->_obj.setDashOffset(dashOffset);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeJoin) {
    uint32_t strokeJoin;
    self->_obj.getStrokeJoin(strokeJoin);
    return ctx.returnValue(strokeJoin, Enum_StrokeJoin);
  }

  NJS_BIND_SET(strokeJoin) {
    uint32_t strokeJoin;
    NJS_CHECK(ctx.unpackValue(strokeJoin, Enum_StrokeJoin));
    self->_obj.setStrokeJoin(strokeJoin);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(startCap) {
    uint32_t startCap;
    self->_obj.getStartCap(startCap);
    return ctx.returnValue(startCap, Enum_StrokeCap);
  }

  NJS_BIND_SET(startCap) {
    uint32_t startCap;
    NJS_CHECK(ctx.unpackValue(startCap, Enum_StrokeCap));
    self->_obj.setStartCap(startCap);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(endCap) {
    uint32_t endCap;
    self->_obj.getEndCap(endCap);
    return ctx.returnValue(endCap, Enum_StrokeCap);
  }

  NJS_BIND_SET(endCap) {
    uint32_t endCap;
    NJS_CHECK(ctx.unpackValue(endCap, Enum_StrokeCap));
    self->_obj.setEndCap(static_cast<uint32_t>(endCap));
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeTransformOrder) {
    uint32_t tOrder;
    self->_obj.getEndCap(tOrder);
    return ctx.returnValue(tOrder, Enum_StrokeTransformOrder);
  }

  NJS_BIND_SET(strokeTransformOrder) {
    uint32_t tOrder;
    NJS_CHECK(ctx.unpackValue(tOrder, Enum_StrokeTransformOrder));
    self->_obj.setStrokeTransformOrder(static_cast<uint32_t>(tOrder));
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Matrix]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(setMatrix) {
    unsigned int argc = ctx.argumentsLength();
    double a, b, c, d, e, f;

    if (argc == 6) {
      NJS_CHECK(ctx.unpackArgument(0, a));
      NJS_CHECK(ctx.unpackArgument(1, b));
      NJS_CHECK(ctx.unpackArgument(2, c));
      NJS_CHECK(ctx.unpackArgument(3, d));
      NJS_CHECK(ctx.unpackArgument(4, e));
      NJS_CHECK(ctx.unpackArgument(5, f));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    self->_obj.setMatrix(b2d::Matrix2D(a, b, c, d, e, f));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(resetMatrix) {
    self->_obj.resetMatrix();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(userToMeta) {
    self->_obj.userToMeta();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(translate      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateP); }
  NJS_BIND_METHOD(translateAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpTranslateA); }
  NJS_BIND_METHOD(scale          ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleP    ); }
  NJS_BIND_METHOD(scaleAppend    ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpScaleA    ); }
  NJS_BIND_METHOD(skew           ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewP     ); }
  NJS_BIND_METHOD(skewAppend     ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpSkewA     ); }
  NJS_BIND_METHOD(rotate         ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateP   ); }
  NJS_BIND_METHOD(rotateAppend   ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpRotateA   ); }
  NJS_BIND_METHOD(transform      ) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyP ); }
  NJS_BIND_METHOD(transformAppend) { return matrixOp(ctx, self, b2d::Matrix2D::kOpMultiplyA ); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, JSContext2D* self, uint32_t op) noexcept {
    BlendJSUtils::MatrixData data;

    NJS_CHECK(BlendJSUtils::unpackMatrixArg(ctx, op, data));
    self->_obj._matrixOp(op, &data);

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Save / Restore]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(save) {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 0) {
      self->_obj.save();
    }
    else if (argc == 1) {
      JSCookie* cookie = nullptr;
      NJS_CHECK(ctx.unwrapArgument<JSCookie>(0, &cookie));
      self->_obj.save(cookie->_obj);
    }
    else {
      return ctx.invalidArgumentsLength(0, 1);
    }

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(restore) {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 0) {
      self->_obj.restore();
    }
    else if (argc == 1) {
      JSCookie* cookie = nullptr;
      NJS_CHECK(ctx.unwrapArgument<JSCookie>(0, &cookie));
      self->_obj.restore(cookie->_obj);
    }
    else {
      return ctx.invalidArgumentsLength(0, 1);
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Fill]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(fillAll) {
    self->_obj.fillAll();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillBox     ) { return fillArg(ctx, self, b2d::kGeomArgBox     ); }
  NJS_BIND_METHOD(fillRect    ) { return fillArg(ctx, self, b2d::kGeomArgRect    ); }
  NJS_BIND_METHOD(fillCircle  ) { return fillArg(ctx, self, b2d::kGeomArgCircle  ); }
  NJS_BIND_METHOD(fillEllipse ) { return fillArg(ctx, self, b2d::kGeomArgEllipse ); }
  NJS_BIND_METHOD(fillRound   ) { return fillArg(ctx, self, b2d::kGeomArgRound   ); }
  NJS_BIND_METHOD(fillChord   ) { return fillArg(ctx, self, b2d::kGeomArgChord   ); }
  NJS_BIND_METHOD(fillPie     ) { return fillArg(ctx, self, b2d::kGeomArgPie     ); }
  NJS_BIND_METHOD(fillTriangle) { return fillArg(ctx, self, b2d::kGeomArgTriangle); }
  NJS_BIND_METHOD(fillPolygon ) { return fillArg(ctx, self, b2d::kGeomArgPolygon ); }
  NJS_BIND_METHOD(fillPath    ) { return fillArg(ctx, self, b2d::kGeomArgPath2D  ); }

  static njs::Result fillArg(njs::FunctionCallContext& ctx, JSContext2D* self, uint32_t id) noexcept {
    BlendJSUtils::GeomData data;
    b2d::MemBufferTmp<1024> mem;

    NJS_CHECK(BlendJSUtils::unpackGeomArg(ctx, id, data, mem));
    self->_obj._fillArg(id, &data);

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillText) {
    NJS_CHECK(ctx.verifyArgumentsLength(4));

    double x, y;
    JSFont* font;
    NJS_CHECK(ctx.unpackArgument(0, x));
    NJS_CHECK(ctx.unpackArgument(1, y));
    NJS_CHECK(ctx.unwrapArgument(2, &font));

    njs::Value text = ctx.argumentAt(3);
    if (!text.isString())
      return ctx.invalidArgument(3);

    uint16_t content[1024];
    int size = text.readUtf16(content, 1024);

    self->_obj.fillUtf16Text(b2d::Point(x, y), font->_obj, content, size_t(size));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillGlyphRun) {
    NJS_CHECK(ctx.verifyArgumentsLength(5));

    double x, y;
    JSFont* font;
    NJS_CHECK(ctx.unpackArgument(0, x));
    NJS_CHECK(ctx.unpackArgument(1, y));
    NJS_CHECK(ctx.unwrapArgument(2, &font));

    njs::Value srcGlyphs = ctx.argumentAt(3);
    njs::Value srcPositions = ctx.argumentAt(4);

    if (!srcGlyphs.isArray() || !srcPositions.isArray())
      return ctx.invalidArgument(1);

    bool posIsRawArray = false;
    size_t count = srcGlyphs.arrayLength();
    size_t posCount = srcPositions.arrayLength();

    posIsRawArray = (count * 2 == posCount);
    if (count != posCount && !posIsRawArray)
      return ctx.invalidArgument();

    if (count) {
      b2d::MemBufferTmp<1024> buf;
      void* p = buf.alloc(count * (sizeof(b2d::Point) + sizeof(b2d::GlyphId)));
      if (p) {
        b2d::Point* tmpOffsets = static_cast<b2d::Point*>(p);
        b2d::GlyphId* tmpGlyphs = static_cast<b2d::GlyphId*>(b2d::Support::advancePtr(p, count * sizeof(b2d::Point)));

        if (posIsRawArray) {
          for (size_t i = 0, posIndex = 0; i < count; i++, posIndex += 2) {
            njs::Value gVal = ctx.propertyAt(srcGlyphs, uint32_t(i));
            njs::Value xVal = ctx.propertyAt(srcPositions, uint32_t(posIndex + 0));
            njs::Value yVal = ctx.propertyAt(srcPositions, uint32_t(posIndex + 1));

            if (!gVal.isUint32() || !xVal.isNumber() || !yVal.isNumber())
              return ctx.invalidArgument();

            uint32_t glyphId = gVal.uint32Value();
            if (glyphId > 65535)
              return ctx.invalidArgument();

            tmpGlyphs[i] = b2d::GlyphId(glyphId);
            tmpOffsets[i].reset(xVal.doubleValue(), yVal.doubleValue());
          }
        }
        else {
          njs::Value xStr = ctx.newInternalizedString(njs::Latin1Ref("x"));
          njs::Value yStr = ctx.newInternalizedString(njs::Latin1Ref("y"));

          for (size_t i = 0; i < count; i++) {
            njs::Value gVal = ctx.propertyAt(srcGlyphs, uint32_t(i));
            njs::Value pVal = ctx.propertyAt(srcPositions, uint32_t(i));

            if (!gVal.isUint32() || !pVal.isObject())
              return ctx.invalidArgument();

            uint32_t glyphId = gVal.uint32Value();
            njs::Value xVal = ctx.propertyOf(pVal, xStr);
            njs::Value yVal = ctx.propertyOf(pVal, yStr);

            if (glyphId > 65535 || !xVal.isNumber() || !yVal.isNumber())
              return ctx.invalidArgument();

            tmpGlyphs[i] = b2d::GlyphId(glyphId);
            tmpOffsets[i].reset(xVal.doubleValue(), yVal.doubleValue());
          }
        }

        b2d::GlyphRun glyphRun;
        glyphRun.setFlags(b2d::GlyphRun::kFlagOffsetsAreAbsolute |
                          b2d::GlyphRun::kFlagUserSpaceOffsets);
        glyphRun.setSize(count);
        glyphRun.setGlyphIds(tmpGlyphs);
        glyphRun.setGlyphOffsets(tmpOffsets);
        self->_obj.fillGlyphRun(b2d::Point(x, y), font->_obj, glyphRun);
      }
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Stroke]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(strokeLine    ) { return strokeArg(ctx, self, b2d::kGeomArgLine    ); }
  NJS_BIND_METHOD(strokeBox     ) { return strokeArg(ctx, self, b2d::kGeomArgBox     ); }
  NJS_BIND_METHOD(strokeRect    ) { return strokeArg(ctx, self, b2d::kGeomArgRect    ); }
  NJS_BIND_METHOD(strokeCircle  ) { return strokeArg(ctx, self, b2d::kGeomArgCircle  ); }
  NJS_BIND_METHOD(strokeEllipse ) { return strokeArg(ctx, self, b2d::kGeomArgEllipse ); }
  NJS_BIND_METHOD(strokeRound   ) { return strokeArg(ctx, self, b2d::kGeomArgRound   ); }
  NJS_BIND_METHOD(strokeChord   ) { return strokeArg(ctx, self, b2d::kGeomArgChord   ); }
  NJS_BIND_METHOD(strokePie     ) { return strokeArg(ctx, self, b2d::kGeomArgPie     ); }
  NJS_BIND_METHOD(strokeTriangle) { return strokeArg(ctx, self, b2d::kGeomArgTriangle); }
  NJS_BIND_METHOD(strokePolygon ) { return strokeArg(ctx, self, b2d::kGeomArgPolygon ); }
  NJS_BIND_METHOD(strokePolyline) { return strokeArg(ctx, self, b2d::kGeomArgPolyline); }
  NJS_BIND_METHOD(strokePath    ) { return strokeArg(ctx, self, b2d::kGeomArgPath2D  ); }

  static njs::Result strokeArg(njs::FunctionCallContext& ctx, JSContext2D* self, uint32_t id) noexcept {
    BlendJSUtils::GeomData data;
    b2d::MemBufferTmp<1024> mem;

    NJS_CHECK(BlendJSUtils::unpackGeomArg(ctx, id, data, mem));
    self->_obj._strokeArg(id, &data);

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Blit]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(blitImage) {
    unsigned int argc = ctx.argumentsLength();
    JSImage* image = nullptr;

    if (argc == 1) {
      NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));
      self->_obj.blitImage(b2d::IntPoint(0, 0), image->_obj);
    }
    else if (argc == 3) {
      double x, y;

      NJS_CHECK(ctx.unpackArgument(0, x));
      NJS_CHECK(ctx.unpackArgument(1, y));
      NJS_CHECK(ctx.unwrapArgument<JSImage>(2, &image));

      self->_obj.blitImage(b2d::Point(x, y), image->_obj);
    }
    else if (argc == 5) {
      double x, y, w, h;

      NJS_CHECK(ctx.unpackArgument(0, x));
      NJS_CHECK(ctx.unpackArgument(1, y));
      NJS_CHECK(ctx.unpackArgument(2, w));
      NJS_CHECK(ctx.unpackArgument(3, h));
      NJS_CHECK(ctx.unwrapArgument<JSImage>(4, &image));

      self->_obj.blitImage(b2d::Rect(x, y, w, h), image->_obj);
    }
    else if (argc == 7) {
      double dx, dy;
      int sx, sy, sw, sh;

      NJS_CHECK(ctx.unpackArgument(0, dx));
      NJS_CHECK(ctx.unpackArgument(1, dy));
      NJS_CHECK(ctx.unwrapArgument<JSImage>(2, &image));
      NJS_CHECK(ctx.unpackArgument(3, sx));
      NJS_CHECK(ctx.unpackArgument(4, sy));
      NJS_CHECK(ctx.unpackArgument(5, sw));
      NJS_CHECK(ctx.unpackArgument(6, sh));

      self->_obj.blitImage(b2d::Point(dx, dy), image->_obj, b2d::IntRect(sx, sy, sw, sh));
    }
    else if (argc == 9) {
      double dx, dy, dw, dh;
      int sx, sy, sw, sh;

      NJS_CHECK(ctx.unpackArgument(0, dx));
      NJS_CHECK(ctx.unpackArgument(1, dy));
      NJS_CHECK(ctx.unpackArgument(2, dw));
      NJS_CHECK(ctx.unpackArgument(3, dh));
      NJS_CHECK(ctx.unwrapArgument<JSImage>(4, &image));
      NJS_CHECK(ctx.unpackArgument(5, sx));
      NJS_CHECK(ctx.unpackArgument(6, sy));
      NJS_CHECK(ctx.unpackArgument(7, sw));
      NJS_CHECK(ctx.unpackArgument(8, sh));

      self->_obj.blitImage(b2d::Rect(dx, dy, dw, dh), image->_obj, b2d::IntRect(sx, sy, sw, sh));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [DecodeAsyncTask]
// ============================================================================

struct DecodeAsyncTask : public njs::Task {
  DecodeAsyncTask(njs::Context& ctx, njs::Value data) noexcept
    : njs::Task(ctx, data),
      _inputData(),
      _error(b2d::kErrorOk) {}

  virtual void onWork() noexcept {
    b2d::ImageCodec codec = b2d::ImageCodec::codecByData(
      b2d::ImageCodec::builtinCodecs(),
      _inputData.data(),
      _inputData.size());

    b2d::ImageDecoder dec = codec.createDecoder();
    _error = dec.decode(_outputImage,
      _inputData.data(),
      _inputData.size());
  }

  virtual void onDone(njs::Context& ctx, njs::Value data) noexcept {
    njs::Value exports = ctx.propertyAt(data, kIndexExports);
    njs::Value cb      = ctx.propertyAt(data, kIndexCallback);

    if (_error != b2d::kErrorOk) {
      ctx.call(cb, ctx.null(), ctx.newValue(_error), ctx.null());
    }
    else {
      njs::Value ImageClass = ctx.propertyOf(exports, njs::Utf8Ref("Image"));
      njs::Value obj = ctx.newInstance(ImageClass);

      JSImage* img = ctx.unwrapUnsafe<JSImage>(obj);
      img->_obj.assign(_outputImage);

      ctx.call(cb, ctx.null(), ctx.null(), obj);
    }
  }

  njs::StrRef<const uint8_t> _inputData;
  b2d::Image _outputImage;
  b2d::Error _error;
};

#if 1
struct ImageIO {
  static NJS_INLINE void db(void* p, uint32_t val) {
    static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>(val & 0xFF);
  }

  static NJS_INLINE void dw(void* p, uint32_t val) {
    static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>((val      ) & 0xFF);
    static_cast<uint8_t*>(p)[1] = static_cast<uint8_t>((val >>  8) & 0xFF);
  }

  static NJS_INLINE void dd(void* p, uint32_t val) {
    static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>((val      ) & 0xFF);
    static_cast<uint8_t*>(p)[1] = static_cast<uint8_t>((val >>  8) & 0xFF);
    static_cast<uint8_t*>(p)[2] = static_cast<uint8_t>((val >> 16) & 0xFF);
    static_cast<uint8_t*>(p)[3] = static_cast<uint8_t>((val >> 24) & 0xFF);
  }

  NJS_BIND_STATIC(decode) {
    // TODO: Uses V8.
    // TODO: Doesn't check buffer, cb
    NJS_CHECK(ctx.verifyArgumentsLength(2));

    njs::Value exports = njs::Value(ctx.v8CallbackInfo().Data());
    njs::Value buffer = ctx.argumentAt(0);
    njs::Value cb = ctx.argumentAt(1);

    njs::Value data = ctx.newObject();
    NJS_CHECK(data);

    ctx.setPropertyAt(data, njs::Task::kIndexCallback, cb);
    ctx.setPropertyAt(data, njs::Task::kIndexExports, exports);
    ctx.setPropertyAt(data, njs::Task::kIndexParams, buffer);

    DecodeAsyncTask* task = new(std::nothrow) DecodeAsyncTask(ctx, data);
    NJS_CHECK(task);

    task->_inputData.init(
      static_cast<uint8_t*>(njs::Node::bufferData(buffer)),
      njs::Node::bufferSize(buffer));

    njs::PostTask(task);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_STATIC(decodeSync) {
    // TODO: Doesn't check buffer
    NJS_CHECK(ctx.verifyArgumentsLength(1));

    njs::Value buffer = ctx.argumentAt(0);

    size_t size = njs::Node::bufferSize(buffer);
    const uint8_t* data = static_cast<const uint8_t*>(njs::Node::bufferData(buffer));

    b2d::ImageCodec codec = b2d::ImageCodec::codecByData(b2d::ImageCodec::builtinCodecs(), data, size);
    b2d::ImageDecoder dec = codec.createDecoder();

    b2d::Image img;
    b2d::Error err = dec.decode(img, data, size);

    if (err != b2d::kErrorOk) {
      return ctx.returnValue(err);
    }
    else {
      njs::Value ImageClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("Image"));
      njs::Value obj = ctx.newInstance(ImageClass);

      JSImage* wrap = ctx.unwrapUnsafe<JSImage>(obj);
      wrap->_obj.assign(img);
      return ctx.returnValue(obj);
    }
  }

  NJS_BIND_STATIC(scaleSync) {
    NJS_CHECK(ctx.verifyArgumentsLength(4, 5));

    JSImage* image;
    int w, h;

    uint32_t filter;
    b2d::ImageScaler::Params params;

    NJS_CHECK(ctx.unwrapArgument<JSImage>(0, &image));
    NJS_CHECK(ctx.unpackArgument(1, w, njs::Range<int>(1, b2d::Image::kMaxSize)));
    NJS_CHECK(ctx.unpackArgument(2, h, njs::Range<int>(1, b2d::Image::kMaxSize)));
    NJS_CHECK(ctx.unpackArgument(3, filter, Enum_ImageScaler_Filter));

    if (filter == b2d::ImageScaler::kFilterNone ||
        filter == b2d::ImageScaler::kFilterCustom)
      return ctx.invalidArgument(3);

    if (ctx.argumentsLength() >= 5) {
      njs::Value obj = ctx.argumentAt(4);
      if (!obj.isObject())
        return ctx.invalidArgument(4);

      switch (filter) {
        case b2d::ImageScaler::kFilterSinc    :
        case b2d::ImageScaler::kFilterLanczos :
        case b2d::ImageScaler::kFilterBlackman: {
          njs::Value radius = ctx.propertyOf(obj, njs::Utf8Ref("radius"));
          if (radius.isNumber())
            params.setRadius(radius.doubleValue());
          break;
        }

        case b2d::ImageScaler::kFilterMitchell: {
          njs::Value b = ctx.propertyOf(obj, njs::Utf8Ref("b"));
          njs::Value c = ctx.propertyOf(obj, njs::Utf8Ref("c"));

          if (b.isNumber())
            params.setMitchellB(b.doubleValue());

          if (c.isNumber())
            params.setMitchellC(c.doubleValue());
          break;
        }
      }
    }

    b2d::Image dst;
    b2d::Error err = b2d::Image::scale(dst, image->_obj, b2d::IntSize(w, h), params);

    if (err != b2d::kErrorOk) {
      return ctx.returnValue(err);
    }
    else {
      njs::Value ImageClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("Image"));
      njs::Value obj = ctx.newInstance(ImageClass);

      JSImage* wrap = ctx.unwrapUnsafe<JSImage>(obj);
      wrap->_obj.assign(dst);
      return ctx.returnValue(obj);
    }
  }

  NJS_BIND_STATIC(toRaw) {
    JSImage* image;

    NJS_CHECK(ctx.verifyArgumentsLength(1));
    NJS_CHECK(ctx.unwrapArgument(0, &image));

    b2d::PixelConverter cvt;
    static const uint32_t convertMasks[4] = { 0xFF000000u, 0x000000FFu, 0x0000FF00u, 0x00FF0000u };

    b2d::Error err = cvt.initExport(image->_obj.pixelFormat(), 32, &convertMasks);
    if (err)
      return ctx.returnValue(ctx.undefined());

    b2d::ImageBuffer imageData;
    err = image->_obj.lock(imageData);
    if (err)
      return ctx.returnValue(ctx.undefined());

    unsigned int w = imageData.width();
    unsigned int h = imageData.height();
    unsigned int bpl = w * 4;
    size_t size = 8 + h * bpl;

    njs::Value buf = njs::Node::newBuffer(ctx, size);
    uint8_t* bufData = static_cast<uint8_t*>(njs::Node::bufferData(buf));

    uint8_t* pixels = imageData.pixelData();
    intptr_t stride = imageData.stride();

    dd(bufData + 0, w);
    dd(bufData + 4, h);
    cvt.convertRect(bufData + 8, bpl, pixels, stride, w, h);

    image->_obj.unlock(imageData);
    return ctx.returnValue(buf);
  }
};
#endif

B2DJS_API NJS_MODULE(b2d) {
  typedef v8::Local<v8::FunctionTemplate> FunctionSpec;

  FunctionSpec CookieSpec          = NJS_INIT_CLASS(JSCookie         , exports);
  FunctionSpec ImageSpec           = NJS_INIT_CLASS(JSImage          , exports);
  FunctionSpec Path2DSpec          = NJS_INIT_CLASS(JSPath2D         , exports);
  FunctionSpec PatternSpec         = NJS_INIT_CLASS(JSPattern        , exports);
  FunctionSpec GradientSpec        = NJS_INIT_CLASS(JSGradient       , exports);
  FunctionSpec LinearGradientSpec  = NJS_INIT_CLASS(JSLinearGradient , exports, GradientSpec);
  FunctionSpec RadialGradientSpec  = NJS_INIT_CLASS(JSRadialGradient , exports, GradientSpec);
  FunctionSpec ConicalGradientSpec = NJS_INIT_CLASS(JSConicalGradient, exports, GradientSpec);
  FunctionSpec FontFaceSpec        = NJS_INIT_CLASS(JSFontFace       , exports);
  FunctionSpec FontSpec            = NJS_INIT_CLASS(JSFont           , exports);
  FunctionSpec Context2DSpec       = NJS_INIT_CLASS(JSContext2D      , exports);

  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("decode"    )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_decode    , exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("decodeSync")).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_decodeSync, exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("toRaw"     )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_toRaw     , exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("scaleSync" )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_scaleSync , exports.v8Handle())->GetFunction());
}

} // b2djs namespace
