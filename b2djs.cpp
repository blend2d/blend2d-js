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

#define ARRAY_SIZE(X) uint32_t(sizeof(X) / sizeof(X[0]))

namespace b2djs {

// ============================================================================
// [Globals]
// ============================================================================

NJS_ENUM(Enum_ContextType, 0, BL_CONTEXT_TYPE_COUNT - 1,
  "null\0"
  "dummy\0"
  "raster\0"
  "raster-async\0");

NJS_ENUM(Enum_FontTag_Category,
  0, 2,
  "table\0"
  "script\0"
  "feature\0");

NJS_ENUM(Enum_CompOp, 0, BL_COMP_OP_COUNT - 1,
  "src-over\0"
  "src-copy\0" "@src\0"
  "src-in\0"
  "src-out\0"
  "src-atop\0"
  "dst-over\0"
  "dst-copy\0" "@dst\0"
  "dst-in\0"
  "dst-out\0"
  "dst-atop\0"
  "xor\0"
  "clear\0"
  "plus\0"
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

NJS_ENUM(Enum_StyleType, 0, BL_STYLE_TYPE_COUNT - 1,
  "none\0"
  "solid\0"
  "gradient\0"
  "pattern\0");

NJS_ENUM(Enum_Format, 0, BL_FORMAT_COUNT - 1,
  "none\0"
  "prgb32\0"
  "xrgb32\0"
  "a8\0");

NJS_ENUM(Enum_FontFaceType, 0, BL_FONT_FACE_TYPE_COUNT - 1,
  "none\0"
  "opentype\0");

NJS_ENUM(Enum_ImageScaleFilter, 0, BL_IMAGE_SCALE_FILTER_COUNT - 1,
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

NJS_ENUM(Enum_FillRule, 0, BL_FILL_RULE_COUNT - 1,
  "non-zero\0"
  "even-odd\0");

NJS_ENUM(Enum_StrokeCap, 0, BL_STROKE_CAP_COUNT - 1,
  "butt\0"
  "square\0"
  "round\0"
  "round-rev\0"
  "triangle\0"
  "triangle-rev\0");

NJS_ENUM(Enum_StrokeJoin, 0, BL_STROKE_JOIN_COUNT - 1,
  "miter-clip\0" "@miter\0"
  "miter-bevel\0"
  "miter-round\0"
  "bevel\0"
  "round\0");

NJS_ENUM(Enum_StrokeTransformOrder, 0, BL_STROKE_TRANSFORM_ORDER_COUNT - 1,
  "after\0"
  "before\0");

NJS_ENUM(Enum_PathReverseMode, 0, BL_PATH_REVERSE_MODE_COUNT - 1,
  "complete\0"
  "separate\0");

NJS_ENUM(Enum_GradientType, 0, BL_GRADIENT_TYPE_COUNT - 1,
  "linear\0"
  "radial\0"
  "conical\0");

NJS_ENUM(Enum_GradientExtendMode, 0, BL_EXTEND_MODE_SIMPLE_COUNT - 1,
  "pad\0"
  "repeat\0"
  "reflect\0");

NJS_ENUM(Enum_PatternQuality, 0, BL_PATTERN_QUALITY_COUNT - 1,
  "nearest\0"
  "bilinear\0");

NJS_ENUM(Enum_PatternExtendMode, 0, BL_EXTEND_MODE_COMPLEX_COUNT - 1,
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

// ============================================================================
// [MemBuffer]
// ============================================================================

//! Memory buffer used as a temporary storage.
class MemBuffer {
public:
  void* _mem;
  void* _buf;
  size_t _capacity;

  MemBuffer(const MemBuffer&) = delete;
  MemBuffer& operator=(const MemBuffer&) = delete;

  BL_INLINE MemBuffer() noexcept
    : _mem(nullptr),
      _buf(nullptr),
      _capacity(0) {}

  BL_INLINE ~MemBuffer() noexcept {
    _reset();
  }

protected:
  BL_INLINE MemBuffer(void* mem, void* buf, size_t capacity) noexcept
    : _mem(mem),
      _buf(buf),
      _capacity(capacity) {}

public:
  BL_INLINE void* get() const noexcept { return _mem; }
  BL_INLINE size_t capacity() const noexcept { return _capacity; }

  BL_INLINE void* alloc(size_t size) noexcept {
    if (size <= _capacity)
      return _mem;

    if (_mem != _buf)
      free(_mem);

    _mem = malloc(size);
    _capacity = size;

    return _mem;
  }

  BL_INLINE void _reset() noexcept {
    if (_mem != _buf)
      free(_mem);
  }

  BL_INLINE void reset() noexcept {
    _reset();

    _mem = nullptr;
    _capacity = 0;
  }
};

//! Memory buffer (temporary).
//!
//! This template is for fast routines that need to use memory  allocated on
//! the stack, but the memory requirement is not known at compile time. The
//! number of bytes allocated on the stack is described by `N` parameter.
template<size_t N>
class MemBufferTmp : public MemBuffer {
public:
  uint8_t _storage[N];

  BL_INLINE MemBufferTmp() noexcept
    : MemBuffer(_storage, _storage, N) {}

  BL_INLINE ~MemBufferTmp() noexcept {}

  using MemBuffer::alloc;

  BL_INLINE void reset() noexcept {
    _reset();
    _mem = _buf;
    _capacity = N;
  }
};

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
  static NJS_INLINE unsigned int stringify(CharT* out, uint32_t rgba) noexcept {
    unsigned int i = 1;
    out[0] = '#';

    if (!BLRgba32(rgba).isOpaque()) {
      out[1] = static_cast<CharT>(intToHex((rgba >> 28) & 0xFu));
      out[2] = static_cast<CharT>(intToHex((rgba >> 24) & 0xFu));
      i += 2;
    }

    out[i + 0] = static_cast<CharT>(intToHex((rgba >> 20) & 0xFu));
    out[i + 1] = static_cast<CharT>(intToHex((rgba >> 16) & 0xFu));
    out[i + 2] = static_cast<CharT>(intToHex((rgba >> 12) & 0xFu));
    out[i + 3] = static_cast<CharT>(intToHex((rgba >>  8) & 0xFu));
    out[i + 4] = static_cast<CharT>(intToHex((rgba >>  4) & 0xFu));
    out[i + 5] = static_cast<CharT>(intToHex((rgba >>  0) & 0xFu));

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
      out = static_cast<uint32_t>(ctx.doubleValue(in));
      return njs::Globals::kResultOk;
    }

    if (in.isString()) {
      uint16_t content[kMaxColorSize];
      int size = ctx.stringLength(in);

      if (size <= 0 || size > kMaxColorSize || ctx.readUtf16(in, content, size) < size)
        return njs::Globals::kResultInvalidValue;

      out = parse<uint16_t>(content, size);
      return njs::Globals::kResultOk;
    }

    return njs::Globals::kResultInvalidValue;
  }
};

// ============================================================================
// [b2djs::B2DUtils]
// ============================================================================

struct B2DUtils {
  union GeometryData {
    BL_INLINE GeometryData() noexcept {}
    BL_INLINE ~GeometryData() noexcept {}

    BLBox box;
    BLRect rect;
    BLRoundRect round;
    BLCircle circle;
    BLEllipse ellipse;
    BLArc arc;
    BLTriangle triangle;
    BLArrayView<BLPoint> poly;
    BLPath path;
  };

  static njs::Result unpackMatrixArg(njs::FunctionCallContext& ctx, uint32_t& op, BLMatrix2D& m) noexcept;
  static njs::Result unpackGeometryArg(njs::FunctionCallContext& ctx, uint32_t& type, GeometryData& data, MemBuffer& mem) noexcept;
};

njs::Result B2DUtils::unpackMatrixArg(
  njs::FunctionCallContext& ctx, uint32_t& op, BLMatrix2D& m) noexcept {

  unsigned int argc = ctx.argumentsLength();
  switch (op) {
    case BL_MATRIX2D_OP_TRANSLATE:
    case BL_MATRIX2D_OP_POST_TRANSLATE:
    case BL_MATRIX2D_OP_SKEW:
    case BL_MATRIX2D_OP_POST_SKEW:
      NJS_CHECK(ctx.verifyArgumentsLength(2));

L_UnpackXY:
      NJS_CHECK(ctx.unpackArgument(0, m.m[0]));
      NJS_CHECK(ctx.unpackArgument(1, m.m[1]));
      return njs::Globals::kResultOk;

    case BL_MATRIX2D_OP_SCALE:
    case BL_MATRIX2D_OP_POST_SCALE:
      if (argc == 2)
        goto L_UnpackXY;

      NJS_CHECK(ctx.verifyArgumentsLength(1));
      NJS_CHECK(ctx.unpackArgument(0, m.m[0]));
      m.m[1] = m.m[0];
      return njs::Globals::kResultOk;

    case BL_MATRIX2D_OP_ROTATE:
    case BL_MATRIX2D_OP_POST_ROTATE:
      if (argc != 1 && argc != 3)
        return ctx.invalidArgumentsLength();

      NJS_CHECK(ctx.unpackArgument(0, m.m[0]));
      if (argc == 3) {
        NJS_CHECK(ctx.unpackArgument(1, m.m[1]));
        NJS_CHECK(ctx.unpackArgument(2, m.m[2]));
        op += (BL_MATRIX2D_OP_POST_ROTATE - BL_MATRIX2D_OP_ROTATE);
      }

      return njs::Globals::kResultOk;

    case BL_MATRIX2D_OP_TRANSFORM:
    case BL_MATRIX2D_OP_POST_TRANSFORM:
      NJS_CHECK(ctx.verifyArgumentsLength(6));
      NJS_CHECK(ctx.unpackArgument(0, m.m[0]));
      NJS_CHECK(ctx.unpackArgument(1, m.m[1]));
      NJS_CHECK(ctx.unpackArgument(2, m.m[2]));
      NJS_CHECK(ctx.unpackArgument(3, m.m[3]));
      NJS_CHECK(ctx.unpackArgument(4, m.m[4]));
      NJS_CHECK(ctx.unpackArgument(5, m.m[5]));
      return njs::Globals::kResultOk;

    default:
      return njs::Globals::kResultInvalidState;
  }
}

njs::Result B2DUtils::unpackGeometryArg(njs::FunctionCallContext& ctx, uint32_t& type, GeometryData& data, MemBuffer& mem) noexcept {
  unsigned int argc = ctx.argumentsLength();
  switch (type) {
    case BL_GEOMETRY_TYPE_BOXD:
    case BL_GEOMETRY_TYPE_LINE: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.box.x0));
      NJS_CHECK(ctx.unpackArgument(1, data.box.y0));
      NJS_CHECK(ctx.unpackArgument(2, data.box.x1));
      NJS_CHECK(ctx.unpackArgument(3, data.box.y1));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_RECTD: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.rect.x));
      NJS_CHECK(ctx.unpackArgument(1, data.rect.y));
      NJS_CHECK(ctx.unpackArgument(2, data.rect.w));
      NJS_CHECK(ctx.unpackArgument(3, data.rect.h));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_CIRCLE: {
      NJS_CHECK(ctx.verifyArgumentsLength(3));
      NJS_CHECK(ctx.unpackArgument(0, data.circle.center.x));
      NJS_CHECK(ctx.unpackArgument(1, data.circle.center.y));
      NJS_CHECK(ctx.unpackArgument(2, data.circle.r));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_ELLIPSE: {
      NJS_CHECK(ctx.verifyArgumentsLength(4));
      NJS_CHECK(ctx.unpackArgument(0, data.ellipse.center.x));
      NJS_CHECK(ctx.unpackArgument(1, data.ellipse.center.y));
      NJS_CHECK(ctx.unpackArgument(2, data.ellipse.radius.x));
      NJS_CHECK(ctx.unpackArgument(3, data.ellipse.radius.y));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_ROUND_RECT: {
      NJS_CHECK(ctx.verifyArgumentsLength(5, 6));

      NJS_CHECK(ctx.unpackArgument(0, data.round.x));
      NJS_CHECK(ctx.unpackArgument(1, data.round.y));
      NJS_CHECK(ctx.unpackArgument(2, data.round.w));
      NJS_CHECK(ctx.unpackArgument(3, data.round.h));

      NJS_CHECK(ctx.unpackArgument(4, data.round.rx));
      data.round.ry = data.round.rx;

      if (argc == 6)
        NJS_CHECK(ctx.unpackArgument(4, data.round.ry));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_ARC:
    case BL_GEOMETRY_TYPE_CHORD:
    case BL_GEOMETRY_TYPE_PIE: {
      NJS_CHECK(ctx.verifyArgumentsLength(5, 6));

      NJS_CHECK(ctx.unpackArgument(0, data.arc.cx));
      NJS_CHECK(ctx.unpackArgument(1, data.arc.cy));

      NJS_CHECK(ctx.unpackArgument(2, data.arc.rx));
      data.arc.ry = data.arc.rx;

      if (argc == 6)
        NJS_CHECK(ctx.unpackArgument(3, data.arc.ry));

      NJS_CHECK(ctx.unpackArgument(argc - 2, data.arc.start));
      NJS_CHECK(ctx.unpackArgument(argc - 1, data.arc.sweep));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_TRIANGLE: {
      NJS_CHECK(ctx.verifyArgumentsLength(6));
      NJS_CHECK(ctx.unpackArgument(0, data.triangle.x0));
      NJS_CHECK(ctx.unpackArgument(1, data.triangle.y0));
      NJS_CHECK(ctx.unpackArgument(2, data.triangle.x1));
      NJS_CHECK(ctx.unpackArgument(3, data.triangle.y1));
      NJS_CHECK(ctx.unpackArgument(4, data.triangle.x2));
      NJS_CHECK(ctx.unpackArgument(5, data.triangle.y2));

      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_POLYGOND:
    case BL_GEOMETRY_TYPE_POLYLINED: {
      if (argc < 2 || (argc & 1) == 1)
        return ctx.invalidArgumentsLength();

      size_t nPoly = static_cast<size_t>(argc) / 2;
      double* poly = static_cast<double*>(mem.alloc(static_cast<size_t>(argc) * sizeof(double)));

      if (!poly)
        return njs::Globals::kResultOutOfMemory;

      for (unsigned int i = 0; i < argc; i++)
        NJS_CHECK(ctx.unpackArgument(i, poly[i]));

      data.poly.reset(reinterpret_cast<BLPoint*>(poly), nPoly);
      return njs::Globals::kResultOk;
    }

    case BL_GEOMETRY_TYPE_PATH: {
      PathWrap* path;

      NJS_CHECK(ctx.verifyArgumentsLength(1));
      NJS_CHECK(ctx.unwrapArgument<PathWrap>(0, &path));

      data.path.impl = path->_obj.impl;
      return njs::Globals::kResultOk;
    }

    default:
      return njs::Globals::kResultInvalidState;
  }
}

// ============================================================================
// [b2djs::RuntimeWrap]
// ============================================================================

struct RuntimeWrap {
  NJS_BIND_STATIC(memoryInfo) {
    BLRuntimeMemoryInfo info;
    blRuntimeQueryInfo(BL_RUNTIME_INFO_TYPE_MEMORY, &info);

    njs::Value obj = ctx.newObject();
    NJS_CHECK(obj);

    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("vmUsed"              )), ctx.newValue(info.vmUsed)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("vmReserved"          )), ctx.newValue(info.vmReserved)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("vmOverhead"          )), ctx.newValue(info.vmOverhead)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("vmBlockCount"        )), ctx.newValue(info.vmBlockCount)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("zmUsed"              )), ctx.newValue(info.zmUsed)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("zmReserved"          )), ctx.newValue(info.zmReserved)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("zmOverhead"          )), ctx.newValue(info.zmOverhead)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("zmBlockCount"        )), ctx.newValue(info.zmBlockCount)));
    NJS_CHECK(ctx.setProperty(obj, ctx.newInternalizedString(njs::Latin1Ref("dynamicPipelineCount")), ctx.newValue(info.dynamicPipelineCount)));

    return ctx.returnValue(obj);
  }
};

// ============================================================================
// [b2djs::ContextCookieWrap]
// ============================================================================

NJS_BIND_CLASS(ContextCookieWrap) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 1) {
      ContextCookieWrap* cookie;
      NJS_CHECK(ctx.unwrapArgument<ContextCookieWrap>(0, &cookie));
      return ctx.returnNew<ContextCookieWrap>(cookie->_obj);
    }
    else {
      return ctx.returnNew<ContextCookieWrap>();
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
// [b2djs::ImageWrap]
// ============================================================================

NJS_BIND_CLASS(ImageWrap) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 1) {
      ImageWrap* image;
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));
      return ctx.returnNew<ImageWrap>(image->_obj);
    }
    else if (argc == 3) {
      int w, h;
      uint32_t format;

      NJS_CHECK(ctx.unpackArgument(0, w, njs::Range<int>(1, BL_RUNTIME_MAX_IMAGE_SIZE)));
      NJS_CHECK(ctx.unpackArgument(1, h, njs::Range<int>(1, BL_RUNTIME_MAX_IMAGE_SIZE)));
      NJS_CHECK(ctx.unpackArgument(2, format, Enum_Format));

      if (format == BL_FORMAT_NONE)
        return ctx.invalidArgumentCustom(2, "Pixel format cannot be 'none'");

      BLImage img;
      BLResult err = img.create(w, h, format);

      if (err) {
        // TODO: Throw out of memory exception.
      }

      return ctx.returnNew<ImageWrap>(img);
    }
    else {
      return ctx.returnNew<ImageWrap>();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(width) {
    return ctx.returnValue(self->_obj.width());
  }

  NJS_BIND_GET(height) {
    return ctx.returnValue(self->_obj.height());
  }

  NJS_BIND_GET(format) {
    return ctx.returnValue(self->_obj.format(), Enum_Format);
  }

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.empty());
  }
};

// ============================================================================
// [b2djs::PathWrap]
// ============================================================================

NJS_BIND_CLASS(PathWrap) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 0) {
      return ctx.returnNew<PathWrap>();
    }
    else if (argc == 1) {
      PathWrap* other;

      NJS_CHECK(ctx.unwrapArgument<PathWrap>(0, &other));
      return ctx.returnNew<PathWrap>(other->_obj);
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
  // NJS_BIND_METHOD(moveToRel) { return moveToInternal(ctx, self, true); }

  NJS_BIND_METHOD(lineTo) { return lineToInternal(ctx, self, false); }
  // NJS_BIND_METHOD(lineToRel) { return lineToInternal(ctx, self, true); }

  static njs::Result moveToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
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

    //if (!isRelative)
      self->_obj.moveTo(x, y);
    //else
    //  self->_obj.moveToRel(x, y);

    return ctx.returnValue(ctx.This());
  }

  static njs::Result lineToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
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
        int n = std::min<int>(argc - a, static_cast<int>(ARRAY_SIZE(points)));

        do {
          double x, y;
          NJS_CHECK(ctx.unpackArgument(a + 0, x));
          NJS_CHECK(ctx.unpackArgument(a + 1, y));

          points[i + 0] = x;
          points[i + 1] = y;

          a += 2;
          i += 2;
        } while (i < n);

        //if (!isRelative)
          self->_obj.polyTo(reinterpret_cast<BLPoint*>(points), n >> 1);
        //else
        //  self->_obj.polyToRel(reinterpret_cast<BLPoint*>(points), n >> 1);
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
  // NJS_BIND_METHOD(quadToRel) { return quadToInternal(ctx, self, true); }

  NJS_BIND_METHOD(smoothQuadTo) { return smoothQuadToInternal(ctx, self, false); }
  // NJS_BIND_METHOD(smoothQuadToRel) { return smoothQuadToInternal(ctx, self, true); }

  static njs::Result quadToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 4) {
      double x1, y1, x2, y2;

      NJS_CHECK(ctx.unpackArgument(0, x1));
      NJS_CHECK(ctx.unpackArgument(1, y1));
      NJS_CHECK(ctx.unpackArgument(2, x2));
      NJS_CHECK(ctx.unpackArgument(3, y2));

      //if (!isRelative)
        self->_obj.quadTo(x1, y1, x2, y2);
      //else
      //  self->_obj.quadToRel(x1, y1, x2, y2);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  static njs::Result smoothQuadToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 2) {
      double x2, y2;

      NJS_CHECK(ctx.unpackArgument(0, x2));
      NJS_CHECK(ctx.unpackArgument(1, y2));

      //if (!isRelative)
        self->_obj.smoothQuadTo(x2, y2);
      //else
      //  self->_obj.smoothQuadToRel(x2, y2);
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
  // NJS_BIND_METHOD(cubicToRel) { return cubicToInternal(ctx, self, true); }

  NJS_BIND_METHOD(smoothCubicTo) { return smoothCubicToInternal(ctx, self, false); }
  // NJS_BIND_METHOD(smoothCubicToRel) { return smoothCubicToInternal(ctx, self, true); }

  static njs::Result cubicToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();
    if (argc == 6) {
      double x1, y1, x2, y2, x3, y3;

      NJS_CHECK(ctx.unpackArgument(0, x1));
      NJS_CHECK(ctx.unpackArgument(1, y1));
      NJS_CHECK(ctx.unpackArgument(2, x2));
      NJS_CHECK(ctx.unpackArgument(3, y2));
      NJS_CHECK(ctx.unpackArgument(4, x3));
      NJS_CHECK(ctx.unpackArgument(5, y3));

      //if (!isRelative)
        self->_obj.cubicTo(x1, y1, x2, y2, x3, y3);
      //else
      //  self->_obj.cubicToRel(x1, y1, x2, y2, x3, y3);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  static njs::Result smoothCubicToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 4) {
      double x2, y2, x3, y3;

      NJS_CHECK(ctx.unpackArgument(0, x2));
      NJS_CHECK(ctx.unpackArgument(1, y2));
      NJS_CHECK(ctx.unpackArgument(2, x3));
      NJS_CHECK(ctx.unpackArgument(3, y3));

      //if (!isRelative)
        self->_obj.smoothCubicTo(x2, y2, x3, y3);
      //else
      //  self->_obj.smoothCubicToRel(x2, y2, x3, y3);
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
  // NJS_BIND_METHOD(arcToRel) { return arcToInternal(ctx, self, true); }

  static njs::Result arcToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
    unsigned int argc = ctx.argumentsLength();

    if (argc >= 5) {
      double cx, cy, rx, ry, start, sweep;
      bool forceMoveTo = false;

      if (ctx.unpackArgument(argc - 1, forceMoveTo) == njs::Globals::kResultOk && --argc < 5)
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

      //if (!isRelative)
        self->_obj.arcTo(cx, cy, rx, ry, start, sweep, forceMoveTo);
      //else
      //  self->_obj.arcToRel(BLPoint(cx, cy), BLPoint(rx, ry), start, sweep, startPath);
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(ellipticArcTo) { return ellipticArcToInternal(ctx, self, false); }
  // NJS_BIND_METHOD(ellipticArcToRel) { return ellipticArcToInternal(ctx, self, true); }

  static njs::Result ellipticArcToInternal(njs::FunctionCallContext& ctx, PathWrap* self, bool isRelative) noexcept {
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

      //if (!isRelative)
        self->_obj.ellipticArcTo(BLPoint(rx, ry), angle, largeArcFlag, sweepFlag, BLPoint(x, y));
      //else
      //  self->_obj.svgArcToRel(BLPoint(rx, ry), angle, largeArcFlag, sweepFlag, BLPoint(x, y));
    }
    else {
      return ctx.invalidArgumentsLength();
    }

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(arcQuadrantTo) {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 4) {
      double x1, y1, x2, y2;

      NJS_CHECK(ctx.unpackArgument(0, x1));
      NJS_CHECK(ctx.unpackArgument(1, y1));
      NJS_CHECK(ctx.unpackArgument(2, x2));
      NJS_CHECK(ctx.unpackArgument(3, y2));

      self->_obj.arcQuadrantTo(x1, y1, x2, y2);
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

  NJS_BIND_METHOD(addBox      ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_BOXD      ); }
  NJS_BIND_METHOD(addRect     ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_RECTD     ); }
  NJS_BIND_METHOD(addCircle   ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_CIRCLE    ); }
  NJS_BIND_METHOD(addEllipse  ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_ELLIPSE   ); }
  NJS_BIND_METHOD(addRoundRect) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_ROUND_RECT); }
  NJS_BIND_METHOD(addChord    ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_CHORD     ); }
  NJS_BIND_METHOD(addPie      ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_PIE       ); }
  NJS_BIND_METHOD(addTriangle ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_TRIANGLE  ); }
  NJS_BIND_METHOD(addPolygon  ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_POLYGOND  ); }
  NJS_BIND_METHOD(addPath     ) { return addGeometry(ctx, self, BL_GEOMETRY_TYPE_PATH      ); }

  NJS_BIND_METHOD(addReversedPath) {
    uint32_t argc = ctx.argumentsLength();
    if (argc < 1 || argc > 2)
      return ctx.invalidArgumentsLength();

    PathWrap* path;
    uint32_t reverseMode = BL_PATH_REVERSE_MODE_COMPLETE;

    NJS_CHECK(ctx.unwrapArgument<PathWrap>(0, &path));
    if (argc == 2) {
      NJS_CHECK(ctx.unpackArgument(1, reverseMode, Enum_PathReverseMode));
    }

    self->_obj.addReversedPath(path->_obj, reverseMode);
    return ctx.returnValue(ctx.This());
  }

  static njs::Result addGeometry(njs::FunctionCallContext& ctx, PathWrap* self, uint32_t geometryType) noexcept {
    B2DUtils::GeometryData geometryData;
    MemBufferTmp<1024> mem;

    NJS_CHECK(B2DUtils::unpackGeometryArg(ctx, geometryType, geometryData, mem));
    self->_obj.addGeometry(geometryType, &geometryData, nullptr, BL_GEOMETRY_DIRECTION_CW);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::PatternWrap]
// ============================================================================

NJS_BIND_CLASS(PatternWrap) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();
    if (argc != 1 && argc != 5)
      return ctx.invalidArgumentsLength();

    ImageWrap* image;
    NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));

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

    BLPattern pattern(image->_obj, BLRectI(x, y, w, h));
    return ctx.returnNew<PatternWrap>(pattern);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(extendMode) {
    return ctx.returnValue(self->_obj.extendMode(), Enum_PatternExtendMode);
  }

  NJS_BIND_SET(extendMode) {
    uint32_t value;
    NJS_CHECK(ctx.unpackValue(value, Enum_PatternExtendMode));
    self->_obj.setExtendMode(value);
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

    self->_obj.setMatrix(BLMatrix2D(a, b, c, d, e, f));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(resetMatrix) {
    self->_obj.resetMatrix();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(translate    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSLATE     ); }
  NJS_BIND_METHOD(postTranslate) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSLATE); }
  NJS_BIND_METHOD(scale        ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SCALE         ); }
  NJS_BIND_METHOD(postScale    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SCALE    ); }
  NJS_BIND_METHOD(skew         ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SKEW          ); }
  NJS_BIND_METHOD(postSkew     ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SKEW     ); }
  NJS_BIND_METHOD(rotate       ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_ROTATE        ); }
  NJS_BIND_METHOD(postRotate   ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_ROTATE   ); }
  NJS_BIND_METHOD(transform    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSFORM     ); }
  NJS_BIND_METHOD(postTransform) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSFORM); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, PatternWrap* self, uint32_t op) noexcept {
    BLMatrix2D m;

    NJS_CHECK(B2DUtils::unpackMatrixArg(ctx, op, m));
    self->_obj._applyMatrixOp(op, &m);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::GradientWrap]
// ============================================================================

NJS_BIND_CLASS(GradientWrap) {
  NJS_BIND_CONSTRUCTOR() {
    unsigned int argc = ctx.argumentsLength();

    if (argc == 0) {
      return ctx.returnNew<GradientWrap>();
    }
    else {
      uint32_t type;
      NJS_CHECK(ctx.unpackArgument(0, type, Enum_GradientType));

      double values[6] {};
      uint32_t count = 0;

      switch (type) {
        case BL_GRADIENT_TYPE_LINEAR: count = 4; break;
        case BL_GRADIENT_TYPE_RADIAL: count = 5; break;
        case BL_GRADIENT_TYPE_CONICAL: count = 3; break;
      }

      if (argc != 1) {
        if (argc - 1 != count)
          return ctx.invalidArgumentsLength();

        for (uint32_t i = 0; i < count; i++) {
          NJS_CHECK(ctx.unpackArgument(i + 1, values[i]));
        }
      }

      return ctx.returnNew<GradientWrap>(type, values);
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(type) {
    uint32_t value = self->_obj.type();
    return ctx.returnValue(value, Enum_GradientType);
  }

  NJS_BIND_SET(type) {
    uint32_t value;
    NJS_CHECK(ctx.unpackValue(value, Enum_GradientType));
    self->_obj.setType(value);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_METHOD(empty) {
    return ctx.returnValue(self->_obj.size() == 0);
  }

  NJS_BIND_GET(length) {
    return ctx.returnValue(self->_obj.size());
  }

  NJS_BIND_GET(extendMode) {
    return ctx.returnValue(self->_obj.extendMode(), Enum_GradientExtendMode);
  }

  NJS_BIND_SET(extendMode) {
    uint32_t value;
    NJS_CHECK(ctx.unpackValue(value, Enum_GradientExtendMode));
    self->_obj.setExtendMode(value);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(x0) {
    return ctx.returnValue(self->_obj.x0());
  }

  NJS_BIND_SET(x0) {
    double x0;
    NJS_CHECK(ctx.unpackValue(x0));
    self->_obj.setX0(x0);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(y0) {
    return ctx.returnValue(self->_obj.y0());
  }

  NJS_BIND_SET(y0) {
    double y0;
    NJS_CHECK(ctx.unpackValue(y0));
    self->_obj.setY0(y0);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(x1) {
    return ctx.returnValue(self->_obj.x1());
  }

  NJS_BIND_SET(x1) {
    double x1;
    NJS_CHECK(ctx.unpackValue(x1));
    self->_obj.setX1(x1);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(y1) {
    return ctx.returnValue(self->_obj.y1());
  }

  NJS_BIND_SET(y1) {
    double y1;
    NJS_CHECK(ctx.unpackValue(y1));
    self->_obj.setY1(y1);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(r0) {
    return ctx.returnValue(self->_obj.r0());
  }

  NJS_BIND_SET(r0) {
    double r0;
    NJS_CHECK(ctx.unpackValue(r0));
    self->_obj.setR0(r0);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(angle) {
    return ctx.returnValue(self->_obj.angle());
  }

  NJS_BIND_SET(angle) {
    double angle;
    NJS_CHECK(ctx.unpackValue(angle));
    self->_obj.setAngle(angle);
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

    self->_obj.addStop(offset, BLRgba32(color));
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

    self->_obj.setMatrix(BLMatrix2D(a, b, c, d, e, f));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(resetMatrix) {
    self->_obj.resetMatrix();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(translate    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSLATE     ); }
  NJS_BIND_METHOD(postTranslate) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSLATE); }
  NJS_BIND_METHOD(scale        ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SCALE         ); }
  NJS_BIND_METHOD(postScale    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SCALE    ); }
  NJS_BIND_METHOD(skew         ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SKEW          ); }
  NJS_BIND_METHOD(postSkew     ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SKEW     ); }
  NJS_BIND_METHOD(rotate       ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_ROTATE        ); }
  NJS_BIND_METHOD(postRotate   ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_ROTATE   ); }
  NJS_BIND_METHOD(transform    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSFORM     ); }
  NJS_BIND_METHOD(postTransform) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSFORM); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, GradientWrap* self, uint32_t op) noexcept {
    BLMatrix2D m;

    NJS_CHECK(B2DUtils::unpackMatrixArg(ctx, op, m));
    self->_obj._applyMatrixOp(op, &m);

    return ctx.returnValue(ctx.This());
  }
};

// ============================================================================
// [b2djs::FontFaceWrap]
// ============================================================================

static njs::Result storeDesignMetrics(njs::ExecutionContext& ctx, njs::Value& dst, const BLFontDesignMetrics& dm) noexcept {
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("ascent"                )), ctx.newValue(dm.ascent)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("descent"               )), ctx.newValue(dm.descent)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("lineGap"               )), ctx.newValue(dm.lineGap)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("xHeight"               )), ctx.newValue(dm.xHeight)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("capHeight"             )), ctx.newValue(dm.capHeight)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vAscent"               )), ctx.newValue(dm.vAscent)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vDescent"              )), ctx.newValue(dm.vDescent)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("hMinLSB"               )), ctx.newValue(dm.hMinLSB)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vMinLSB"               )), ctx.newValue(dm.vMinLSB)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("hMinTSB"               )), ctx.newValue(dm.hMinTSB)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vMinTSB"               )), ctx.newValue(dm.vMinTSB)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("hMaxAdvance"           )), ctx.newValue(dm.hMaxAdvance)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("vMaxAdvance"           )), ctx.newValue(dm.hMaxAdvance)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("underlinePosition"     )), ctx.newValue(dm.underlinePosition)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("underlineThickness"    )), ctx.newValue(dm.underlineThickness)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("strikethroughPosition" )), ctx.newValue(dm.strikethroughPosition)));
  NJS_CHECK(ctx.setProperty(dst, ctx.newInternalizedString(njs::Latin1Ref("strikethroughThickness")), ctx.newValue(dm.strikethroughThickness)));

  return njs::Globals::kResultOk;
}

static njs::Result storeDiagInfo(njs::ExecutionContext& ctx, njs::Value& dst, uint32_t diagFlags) noexcept {
  #define ADD_DIAG_FLAG(NAME, FLAG)                                                   \
    ctx.setProperty(dst,                                                              \
                    ctx.newInternalizedString(njs::Latin1Ref(NAME)),                  \
                    ctx.newValue((diagFlags & FLAG) != 0))

  NJS_CHECK(ADD_DIAG_FLAG("wrongNameData", BL_FONT_FACE_DIAG_WRONG_NAME_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("fixedNameData", BL_FONT_FACE_DIAG_FIXED_NAME_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("wrongKernData", BL_FONT_FACE_DIAG_WRONG_KERN_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("fixedKernData", BL_FONT_FACE_DIAG_FIXED_KERN_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("wrongCMapData", BL_FONT_FACE_DIAG_WRONG_CMAP_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("fixedCMapData", BL_FONT_FACE_DIAG_WRONG_CMAP_FORMAT));
  NJS_CHECK(ADD_DIAG_FLAG("wrongGDefData", BL_FONT_FACE_DIAG_WRONG_GDEF_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("wrongGPosData", BL_FONT_FACE_DIAG_WRONG_GPOS_DATA));
  NJS_CHECK(ADD_DIAG_FLAG("wrongGSubData", BL_FONT_FACE_DIAG_WRONG_GSUB_DATA));

  #undef ADD_DIAG_FLAG

  return njs::Globals::kResultOk;
}

static njs::Result tagsToJSArray(njs::ExecutionContext& ctx, njs::Value& dst, const BLArray<uint32_t>& tags) noexcept {
  uint32_t count = unsigned(tags.size());
  dst = ctx.newArray();
  NJS_CHECK(dst);

  for (uint32_t i = 0; i < count; i++) {
    njs::Value s = ctx.newUint32(tags[i]);
    NJS_CHECK(s);
    NJS_CHECK(ctx.setPropertyAt(dst, i, s));
  }

  return njs::Globals::kResultOk;
}

NJS_BIND_CLASS(FontFaceWrap) {
  NJS_BIND_CONSTRUCTOR() {
    return ctx.returnNew<FontFaceWrap>();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(faceType) {
    return ctx.returnValue(self->_obj.faceType(), Enum_FontFaceType);
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

  NJS_BIND_GET(diagInfo) {
    njs::Value dmObj = ctx.newObject();
    NJS_CHECK(dmObj);
    NJS_CHECK(storeDiagInfo(ctx, dmObj, self->_obj.diagFlags()));
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
    int size = ctx.readUtf8(arg, fileName, sizeof(fileName) - 1);
    fileName[size] = '\0';

    BLResult err = self->_obj.createFromFile(fileName);
    return ctx.returnValue(err);
  }

  /*
  NJS_BIND_METHOD(listTags) {
    NJS_CHECK(ctx.verifyArgumentsLength(1));

    uint32_t category;
    NJS_CHECK(ctx.unpackArgument(0, category, Enum_FontTag_Category));

    BLArray<uint32_t> tags;
    self->_obj.listTags(category, tags);

    njs::Value out;
    NJS_CHECK(tagsToJSArray(ctx, out, tags));
    return ctx.returnValue(out);
  }
  */
};

// ============================================================================
// [b2djs::FontWrap]
// ============================================================================

NJS_BIND_CLASS(FontWrap) {
  NJS_BIND_CONSTRUCTOR() {
    return ctx.returnNew<FontWrap>();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(face) {
    njs::Value FontFaceClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("FontFace"));
    njs::Value obj = ctx.newInstance(FontFaceClass);

    FontFaceWrap* wrap = ctx.unwrapUnsafe<FontFaceWrap>(obj);
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
    const BLFontMetrics& m = self->_obj.metrics();

    njs::Value mObj = ctx.newObject();
    NJS_CHECK(mObj);
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("ascent"                )), ctx.newValue(m.ascent)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("descent"               )), ctx.newValue(m.descent)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("lineGap"               )), ctx.newValue(m.lineGap)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("xHeight"               )), ctx.newValue(m.xHeight)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("capHeight"             )), ctx.newValue(m.capHeight)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("vAscent"               )), ctx.newValue(m.vAscent)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("vDescent"              )), ctx.newValue(m.vDescent)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("underlinePosition"     )), ctx.newValue(m.underlinePosition)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("underlineThickness"    )), ctx.newValue(m.underlineThickness)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("strikethroughPosition" )), ctx.newValue(m.strikethroughPosition)));
    NJS_CHECK(ctx.setProperty(mObj, ctx.newInternalizedString(njs::Latin1Ref("strikethroughThickness")), ctx.newValue(m.strikethroughThickness)));

    return ctx.returnValue(mObj);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(createFromFace) {
    int argc = ctx.argumentsLength();
    if (argc != 2)
      return ctx.invalidArgumentsLength(2);

    FontFaceWrap* face;
    NJS_CHECK(ctx.unwrapArgument<FontFaceWrap>(0, &face));

    double size;
    NJS_CHECK(ctx.unpackArgument(1, size));

    BLResult err = self->_obj.createFromFace(face->_obj, size);
    return ctx.returnValue(err);
  }
};

// ============================================================================
// [b2djs::ContextWrap]
// ============================================================================

NJS_BIND_CLASS(ContextWrap) {
  NJS_BIND_CONSTRUCTOR() {
    NJS_CHECK(ctx.verifyArgumentsLength(0, 1));
    unsigned int argc = ctx.argumentsLength();

    if (argc == 1) {
      ImageWrap* image;
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));
      return ctx.returnNew<ContextWrap>(image->_obj);
    }
    else {
      return ctx.returnNew<ContextWrap>();
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(isNone) {
    return ctx.returnValue(self->_obj.isNone());
  }

  NJS_BIND_GET(contextType) {
    return ctx.returnValue(self->_obj.contextType(), Enum_ContextType);
  }

  NJS_BIND_GET(targetWidth) {
    return ctx.returnValue(self->_obj.targetWidth());
  }

  NJS_BIND_GET(targetHeight) {
    return ctx.returnValue(self->_obj.targetHeight());
  }

  // --------------------------------------------------------------------------
  // [Begin / End]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(begin) {
    NJS_CHECK(ctx.verifyArgumentsLength(1));

    ImageWrap* image;
    NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));

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

  NJS_BIND_GET(flattenTolerance) {
    double tolerance = self->_obj.flattenTolerance();
    return ctx.returnValue(tolerance);
  }

  NJS_BIND_SET(flattenTolerance) {
    double tolerance;
    NJS_CHECK(ctx.unpackValue(tolerance));
    self->_obj.setFlattenTolerance(tolerance);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(compOp) {
    uint32_t compOp = self->_obj.compOp();
    return ctx.returnValue(compOp, Enum_CompOp);
  }

  NJS_BIND_SET(compOp) {
    uint32_t compOp;
    NJS_CHECK(ctx.unpackValue(compOp, Enum_CompOp));
    self->_obj.setCompOp(compOp);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(alpha) {
    double alpha = self->_obj.globalAlpha();
    return ctx.returnValue(alpha);
  }

  NJS_BIND_SET(alpha) {
    double alpha;
    NJS_CHECK(ctx.unpackValue(alpha));
    self->_obj.setGlobalAlpha(alpha);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(patternQuality) {
    uint32_t quality = self->_obj.hints().patternQuality;
    return ctx.returnValue(quality, Enum_PatternQuality);
  }

  NJS_BIND_SET(patternQuality) {
    uint32_t quality;
    NJS_CHECK(ctx.unpackValue(quality, Enum_PatternQuality));
    self->_obj.setPatternQuality(quality);
    return njs::Globals::kResultOk;
  }

  // --------------------------------------------------------------------------
  // [Fill & Stroke Alpha]
  // --------------------------------------------------------------------------

  NJS_BIND_GET(fillAlpha) {
    double alpha = self->_obj.fillAlpha();
    return ctx.returnValue(alpha);
  }

  NJS_BIND_SET(fillAlpha) {
    double alpha;
    NJS_CHECK(ctx.unpackValue(alpha));
    self->_obj.setFillAlpha(alpha);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeAlpha) {
    double alpha = self->_obj.strokeAlpha();
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

  NJS_BIND_GET(fillStyle) { return getOpStyle(ctx, self, BL_CONTEXT_OP_TYPE_FILL); }
  NJS_BIND_SET(fillStyle) { return setOpStyle(ctx, self, BL_CONTEXT_OP_TYPE_FILL); }

  NJS_BIND_GET(strokeStyle) { return getOpStyle(ctx, self, BL_CONTEXT_OP_TYPE_STROKE); }
  NJS_BIND_SET(strokeStyle) { return setOpStyle(ctx, self, BL_CONTEXT_OP_TYPE_STROKE); }

  static njs::Result getOpStyle(njs::GetPropertyContext& ctx, ContextWrap* self, uint32_t slot) noexcept {
    // TODO:
    /*
    BLRgba32 rgba;
    if (self->_obj.getStyle(slot, rgba) == BL_SUCCESS)
      return ctx.returnValue(rgba.value, ColorConcept());
    */

    return ctx.returnValue(njs::Undefined);
  }

  static njs::Result setOpStyle(njs::SetPropertyContext& ctx, ContextWrap* self, uint32_t opType) noexcept {
    njs::Value value = ctx.propertyValue();

    if (value.isNumber()) {
      double d = ctx.doubleValue(value);
      uint32_t rgba = static_cast<uint32_t>(d);
      self->_obj.setOpStyle(opType, BLRgba32(rgba));
    }
    else if (value.isString()) {
      uint32_t rgba;
      NJS_CHECK(ctx.unpack(value, rgba, ColorConcept()));
      self->_obj.setOpStyle(opType, BLRgba32(rgba));
    }
    else if (ctx.isWrapped<GradientWrap>(value)) {
      GradientWrap* gradient = ctx.unwrapUnsafe<GradientWrap>(value);
      self->_obj.setOpStyle(opType, gradient->_obj);
    }
    else if (ctx.isWrapped<PatternWrap>(value)) {
      PatternWrap* pattern = ctx.unwrapUnsafe<PatternWrap>(value);
      self->_obj.setOpStyle(opType, pattern->_obj);
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
    uint32_t fillRule = self->_obj.fillRule();
    return ctx.returnValue(fillRule, Enum_FillRule);
  }

  NJS_BIND_SET(fillRule) {
    uint32_t fillRule;
    NJS_CHECK(ctx.unpackValue(fillRule, Enum_FillRule));
    self->_obj.setFillRule(fillRule);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeWidth) {
    double strokeWidth = self->_obj.strokeWidth();
    return ctx.returnValue(strokeWidth);
  }

  NJS_BIND_SET(strokeWidth) {
    double strokeWidth;
    NJS_CHECK(ctx.unpackValue(strokeWidth));
    self->_obj.setStrokeWidth(strokeWidth);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeMiterLimit) {
    double miterLimit = self->_obj.strokeMiterLimit();
    return ctx.returnValue(miterLimit);
  }

  NJS_BIND_SET(strokeMiterLimit) {
    double miterLimit;
    NJS_CHECK(ctx.unpackValue(miterLimit));
    self->_obj.setStrokeMiterLimit(miterLimit);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeDashOffset) {
    double dashOffset = self->_obj.strokeDashOffset();
    return ctx.returnValue(dashOffset);
  }

  NJS_BIND_SET(strokeDashOffset) {
    double dashOffset;
    NJS_CHECK(ctx.unpackValue(dashOffset));
    self->_obj.setStrokeDashOffset(dashOffset);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeJoin) {
    uint32_t strokeJoin = self->_obj.strokeJoin();
    return ctx.returnValue(strokeJoin, Enum_StrokeJoin);
  }

  NJS_BIND_SET(strokeJoin) {
    uint32_t strokeJoin;
    NJS_CHECK(ctx.unpackValue(strokeJoin, Enum_StrokeJoin));
    self->_obj.setStrokeJoin(strokeJoin);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeStartCap) {
    uint32_t startCap = self->_obj.strokeStartCap();
    return ctx.returnValue(startCap, Enum_StrokeCap);
  }

  NJS_BIND_SET(strokeStartCap) {
    uint32_t startCap;
    NJS_CHECK(ctx.unpackValue(startCap, Enum_StrokeCap));
    self->_obj.setStrokeStartCap(startCap);
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeEndCap) {
    uint32_t endCap = self->_obj.strokeEndCap();
    return ctx.returnValue(endCap, Enum_StrokeCap);
  }

  NJS_BIND_SET(strokeEndCap) {
    uint32_t endCap;
    NJS_CHECK(ctx.unpackValue(endCap, Enum_StrokeCap));
    self->_obj.setStrokeEndCap(static_cast<uint32_t>(endCap));
    return njs::Globals::kResultOk;
  }

  NJS_BIND_GET(strokeTransformOrder) {
    uint32_t order = self->_obj.strokeTransformOrder();
    return ctx.returnValue(order, Enum_StrokeTransformOrder);
  }

  NJS_BIND_SET(strokeTransformOrder) {
    uint32_t order;
    NJS_CHECK(ctx.unpackValue(order, Enum_StrokeTransformOrder));
    self->_obj.setStrokeTransformOrder(order);
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

    self->_obj.setMatrix(BLMatrix2D(a, b, c, d, e, f));
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

  NJS_BIND_METHOD(translate    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSLATE     ); }
  NJS_BIND_METHOD(postTranslate) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSLATE); }
  NJS_BIND_METHOD(scale        ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SCALE         ); }
  NJS_BIND_METHOD(postScale    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SCALE    ); }
  NJS_BIND_METHOD(skew         ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_SKEW          ); }
  NJS_BIND_METHOD(postSkew     ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_SKEW     ); }
  NJS_BIND_METHOD(rotate       ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_ROTATE        ); }
  NJS_BIND_METHOD(postRotate   ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_ROTATE   ); }
  NJS_BIND_METHOD(transform    ) { return matrixOp(ctx, self, BL_MATRIX2D_OP_TRANSFORM     ); }
  NJS_BIND_METHOD(postTransform) { return matrixOp(ctx, self, BL_MATRIX2D_OP_POST_TRANSFORM); }

  static njs::Result matrixOp(njs::FunctionCallContext& ctx, ContextWrap* self, uint32_t op) noexcept {
    BLMatrix2D m;

    NJS_CHECK(B2DUtils::unpackMatrixArg(ctx, op, m));
    self->_obj._applyMatrixOp(op, &m);

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
      ContextCookieWrap* cookie = nullptr;
      NJS_CHECK(ctx.unwrapArgument<ContextCookieWrap>(0, &cookie));
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
      ContextCookieWrap* cookie = nullptr;
      NJS_CHECK(ctx.unwrapArgument<ContextCookieWrap>(0, &cookie));
      self->_obj.restore(cookie->_obj);
    }
    else {
      return ctx.invalidArgumentsLength(0, 1);
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Clip]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(clipToRect) {
    BLRect rect;

    NJS_CHECK(ctx.verifyArgumentsLength(4));
    NJS_CHECK(ctx.unpackArgument(0, rect.x));
    NJS_CHECK(ctx.unpackArgument(1, rect.y));
    NJS_CHECK(ctx.unpackArgument(2, rect.w));
    NJS_CHECK(ctx.unpackArgument(3, rect.h));

    self->_obj.clipToRect(rect);
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(restoreClipping) {
    NJS_CHECK(ctx.verifyArgumentsLength(0));
    self->_obj.restoreClipping();
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(clearAll) {
    self->_obj.clearAll();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(clearRect) {
    BLRect rect;

    NJS_CHECK(ctx.verifyArgumentsLength(4));
    NJS_CHECK(ctx.unpackArgument(0, rect.x));
    NJS_CHECK(ctx.unpackArgument(1, rect.y));
    NJS_CHECK(ctx.unpackArgument(2, rect.w));
    NJS_CHECK(ctx.unpackArgument(3, rect.h));

    self->_obj.clearRect(rect);
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Fill]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(fillAll) {
    self->_obj.fillAll();
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillBox      ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_BOXD      ); }
  NJS_BIND_METHOD(fillRect     ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_RECTD     ); }
  NJS_BIND_METHOD(fillCircle   ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_CIRCLE    ); }
  NJS_BIND_METHOD(fillEllipse  ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_ELLIPSE   ); }
  NJS_BIND_METHOD(fillRoundRect) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_ROUND_RECT); }
  NJS_BIND_METHOD(fillChord    ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_CHORD     ); }
  NJS_BIND_METHOD(fillPie      ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_PIE       ); }
  NJS_BIND_METHOD(fillTriangle ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_TRIANGLE  ); }
  NJS_BIND_METHOD(fillPolygon  ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_POLYGOND  ); }
  NJS_BIND_METHOD(fillPath     ) { return fillGeometry(ctx, self, BL_GEOMETRY_TYPE_PATH      ); }

  static njs::Result fillGeometry(njs::FunctionCallContext& ctx, ContextWrap* self, uint32_t geometryType) noexcept {
    B2DUtils::GeometryData geometryData;
    MemBufferTmp<1024> mem;

    NJS_CHECK(B2DUtils::unpackGeometryArg(ctx, geometryType, geometryData, mem));
    self->_obj.fillGeometry(geometryType, &geometryData);

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillText) {
    NJS_CHECK(ctx.verifyArgumentsLength(4));

    double x, y;
    FontWrap* font;
    NJS_CHECK(ctx.unpackArgument(0, x));
    NJS_CHECK(ctx.unpackArgument(1, y));
    NJS_CHECK(ctx.unwrapArgument(2, &font));

    njs::Value text = ctx.argumentAt(3);
    if (!text.isString())
      return ctx.invalidArgument(3);

    MemBufferTmp<2048> buf;
    size_t len = ctx.stringLength(text);

    uint16_t* content = static_cast<uint16_t*>(buf.alloc(len * sizeof(uint16_t)));
    if (!content)
      return ctx.returnValue(ctx.This());

    int size = ctx.readUtf16(text, content, len);
    self->_obj.fillUtf16Text(BLPoint(x, y), font->_obj, content, size_t(size));
    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(fillGlyphRun) {
    NJS_CHECK(ctx.verifyArgumentsLength(5));

    double x, y;
    FontWrap* font;
    NJS_CHECK(ctx.unpackArgument(0, x));
    NJS_CHECK(ctx.unpackArgument(1, y));
    NJS_CHECK(ctx.unwrapArgument(2, &font));

    njs::Value srcGlyphs = ctx.argumentAt(3);
    njs::Value srcPositions = ctx.argumentAt(4);

    if (!srcGlyphs.isArray() || !srcPositions.isArray())
      return ctx.invalidArgument(1);

    bool posIsRawArray = false;
    size_t count = ctx.arrayLength(srcGlyphs);
    size_t posCount = ctx.arrayLength(srcPositions);

    posIsRawArray = (count * 2 == posCount);
    if (count != posCount && !posIsRawArray)
      return ctx.invalidArgument();

    if (count) {
      MemBufferTmp<1024> buf;
      void* p = buf.alloc(count * (sizeof(BLPoint) + sizeof(BLGlyphId)));
      if (p) {
        BLPoint* tmpOffsets = static_cast<BLPoint*>(p);
        BLGlyphId* tmpGlyphs = reinterpret_cast<BLGlyphId*>(tmpOffsets + count);

        if (posIsRawArray) {
          for (size_t i = 0, posIndex = 0; i < count; i++, posIndex += 2) {
            njs::Value gVal = ctx.propertyAt(srcGlyphs, uint32_t(i));
            njs::Value xVal = ctx.propertyAt(srcPositions, uint32_t(posIndex + 0));
            njs::Value yVal = ctx.propertyAt(srcPositions, uint32_t(posIndex + 1));

            if (!gVal.isUint32() || !xVal.isNumber() || !yVal.isNumber())
              return ctx.invalidArgument();

            uint32_t glyphId = ctx.uint32Value(gVal);
            if (glyphId > 65535)
              return ctx.invalidArgument();

            tmpGlyphs[i] = BLGlyphId(glyphId);
            tmpOffsets[i].reset(ctx.doubleValue(xVal), ctx.doubleValue(yVal));
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

            uint32_t glyphId = ctx.uint32Value(gVal);
            njs::Value xVal = ctx.propertyOf(pVal, xStr);
            njs::Value yVal = ctx.propertyOf(pVal, yStr);

            if (glyphId > 65535 || !xVal.isNumber() || !yVal.isNumber())
              return ctx.invalidArgument();

            tmpGlyphs[i] = BLGlyphId(glyphId);
            tmpOffsets[i].reset(ctx.doubleValue(xVal), ctx.doubleValue(yVal));
          }
        }

        BLGlyphRun gr {};
        gr.glyphIdData = tmpGlyphs;
        gr.placementData = tmpOffsets;
        gr.size = count;
        gr.glyphIdSize = 2;
        gr.glyphIdAdvance = int8_t(sizeof(BLGlyphId));
        gr.placementAdvance = int8_t(sizeof(BLPoint));
        gr.placementType = BL_GLYPH_PLACEMENT_TYPE_USER_UNITS;
        self->_obj.fillGlyphRun(BLPoint(x, y), font->_obj, gr);
      }
    }

    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Stroke]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(strokeLine     ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_LINE      ); }
  NJS_BIND_METHOD(strokeBox      ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_BOXD      ); }
  NJS_BIND_METHOD(strokeRect     ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_RECTD     ); }
  NJS_BIND_METHOD(strokeCircle   ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_CIRCLE    ); }
  NJS_BIND_METHOD(strokeEllipse  ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_ELLIPSE   ); }
  NJS_BIND_METHOD(strokeRoundRect) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_ROUND_RECT); }
  NJS_BIND_METHOD(strokeChord    ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_CHORD     ); }
  NJS_BIND_METHOD(strokePie      ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_PIE       ); }
  NJS_BIND_METHOD(strokeTriangle ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_TRIANGLE  ); }
  NJS_BIND_METHOD(strokePolygon  ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_POLYGOND  ); }
  NJS_BIND_METHOD(strokePolyline ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_POLYLINED ); }
  NJS_BIND_METHOD(strokePath     ) { return strokeGeometry(ctx, self, BL_GEOMETRY_TYPE_PATH      ); }


  static njs::Result strokeGeometry(njs::FunctionCallContext& ctx, ContextWrap* self, uint32_t geometryType) noexcept {
    B2DUtils::GeometryData geometryData;
    MemBufferTmp<1024> mem;

    NJS_CHECK(B2DUtils::unpackGeometryArg(ctx, geometryType, geometryData, mem));
    self->_obj.strokeGeometry(geometryType, &geometryData);

    return ctx.returnValue(ctx.This());
  }

  NJS_BIND_METHOD(strokeText) {
    NJS_CHECK(ctx.verifyArgumentsLength(4));

    double x, y;
    FontWrap* font;
    NJS_CHECK(ctx.unpackArgument(0, x));
    NJS_CHECK(ctx.unpackArgument(1, y));
    NJS_CHECK(ctx.unwrapArgument(2, &font));

    njs::Value text = ctx.argumentAt(3);
    if (!text.isString())
      return ctx.invalidArgument(3);

    MemBufferTmp<2048> buf;
    size_t len = ctx.stringLength(text);

    uint16_t* content = static_cast<uint16_t*>(buf.alloc(len * sizeof(uint16_t)));
    if (!content)
      return ctx.returnValue(ctx.This());

    int size = ctx.readUtf16(text, content, len);
    self->_obj.strokeUtf16Text(BLPoint(x, y), font->_obj, content, size_t(size));
    return ctx.returnValue(ctx.This());
  }

  // --------------------------------------------------------------------------
  // [Blit]
  // --------------------------------------------------------------------------

  NJS_BIND_METHOD(blitImage) {
    unsigned int argc = ctx.argumentsLength();
    ImageWrap* image = nullptr;

    if (argc == 1) {
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));
      self->_obj.blitImage(BLPointI(0, 0), image->_obj);
    }
    else if (argc == 3) {
      double x, y;

      NJS_CHECK(ctx.unpackArgument(0, x));
      NJS_CHECK(ctx.unpackArgument(1, y));
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(2, &image));

      self->_obj.blitImage(BLPoint(x, y), image->_obj);
    }
    else if (argc == 5) {
      double x, y, w, h;

      NJS_CHECK(ctx.unpackArgument(0, x));
      NJS_CHECK(ctx.unpackArgument(1, y));
      NJS_CHECK(ctx.unpackArgument(2, w));
      NJS_CHECK(ctx.unpackArgument(3, h));
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(4, &image));

      self->_obj.blitImage(BLRect(x, y, w, h), image->_obj);
    }
    else if (argc == 7) {
      double dx, dy;
      int sx, sy, sw, sh;

      NJS_CHECK(ctx.unpackArgument(0, dx));
      NJS_CHECK(ctx.unpackArgument(1, dy));
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(2, &image));
      NJS_CHECK(ctx.unpackArgument(3, sx));
      NJS_CHECK(ctx.unpackArgument(4, sy));
      NJS_CHECK(ctx.unpackArgument(5, sw));
      NJS_CHECK(ctx.unpackArgument(6, sh));

      self->_obj.blitImage(BLPoint(dx, dy), image->_obj, BLRectI(sx, sy, sw, sh));
    }
    else if (argc == 9) {
      double dx, dy, dw, dh;
      int sx, sy, sw, sh;

      NJS_CHECK(ctx.unpackArgument(0, dx));
      NJS_CHECK(ctx.unpackArgument(1, dy));
      NJS_CHECK(ctx.unpackArgument(2, dw));
      NJS_CHECK(ctx.unpackArgument(3, dh));
      NJS_CHECK(ctx.unwrapArgument<ImageWrap>(4, &image));
      NJS_CHECK(ctx.unpackArgument(5, sx));
      NJS_CHECK(ctx.unpackArgument(6, sy));
      NJS_CHECK(ctx.unpackArgument(7, sw));
      NJS_CHECK(ctx.unpackArgument(8, sh));

      self->_obj.blitImage(BLRect(dx, dy, dw, dh), image->_obj, BLRectI(sx, sy, sw, sh));
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
      _error(BL_SUCCESS) {}

  virtual void onWork() noexcept {
    BLImageCodec codec;
    _error = codec.findByData(BLImageCodec::builtInCodecs(), _inputData.data(), _inputData.size());
    if (_error != BL_SUCCESS)
      return;

    BLImageDecoder decoder;
    _error = codec.createDecoder(&decoder);
    if (_error != BL_SUCCESS)
      return;

    _error = decoder.readFrame(_outputImage, _inputData.data(), _inputData.size());
  }

  virtual void onDone(njs::Context& ctx, njs::Value data) noexcept {
    njs::Value exports = ctx.propertyAt(data, kIndexExports);
    njs::Value cb      = ctx.propertyAt(data, kIndexCallback);

    if (_error != BL_SUCCESS) {
      ctx.call(cb, ctx.null(), ctx.newValue(_error), ctx.null());
    }
    else {
      njs::Value ImageClass = ctx.propertyOf(exports, njs::Utf8Ref("Image"));
      njs::Value obj = ctx.newInstance(ImageClass);

      ImageWrap* img = ctx.unwrapUnsafe<ImageWrap>(obj);
      img->_obj.assign(_outputImage);

      ctx.call(cb, ctx.null(), ctx.null(), obj);
    }
  }

  njs::StrRef<const uint8_t> _inputData;
  BLImage _outputImage;
  BLResult _error;
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

    BLImageCodec codec;
    BLResult result = codec.findByData(BLImageCodec::builtInCodecs(), data, size);
    if (result != BL_SUCCESS)
      return ctx.returnValue(result);

    BLImageDecoder decoder;
    result = codec.createDecoder(&decoder);
    if (result != BL_SUCCESS)
      return ctx.returnValue(result);

    BLImage img;
    result = decoder.readFrame(img, data, size);
    if (result != BL_SUCCESS)
      return ctx.returnValue(result);

    njs::Value ImageClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("Image"));
    njs::Value obj = ctx.newInstance(ImageClass);

    ImageWrap* wrap = ctx.unwrapUnsafe<ImageWrap>(obj);
    wrap->_obj.assign(img);
    return ctx.returnValue(obj);
  }

  NJS_BIND_STATIC(scaleSync) {
    NJS_CHECK(ctx.verifyArgumentsLength(4, 5));

    ImageWrap* image;
    int w, h;

    uint32_t filter;
    BLImageScaleOptions options;
    options.resetToDefaults();

    NJS_CHECK(ctx.unwrapArgument<ImageWrap>(0, &image));
    NJS_CHECK(ctx.unpackArgument(1, w, njs::Range<int>(1, BL_RUNTIME_MAX_IMAGE_SIZE)));
    NJS_CHECK(ctx.unpackArgument(2, h, njs::Range<int>(1, BL_RUNTIME_MAX_IMAGE_SIZE)));
    NJS_CHECK(ctx.unpackArgument(3, filter, Enum_ImageScaleFilter));

    if (filter == BL_IMAGE_SCALE_FILTER_NONE ||
        filter == BL_IMAGE_SCALE_FILTER_USER)
      return ctx.invalidArgument(3);

    if (ctx.argumentsLength() >= 5) {
      njs::Value obj = ctx.argumentAt(4);
      if (!obj.isObject())
        return ctx.invalidArgument(4);

      switch (filter) {
        case BL_IMAGE_SCALE_FILTER_SINC    :
        case BL_IMAGE_SCALE_FILTER_LANCZOS :
        case BL_IMAGE_SCALE_FILTER_BLACKMAN: {
          njs::Value radius = ctx.propertyOf(obj, njs::Utf8Ref("radius"));
          if (radius.isNumber())
            options.radius = ctx.doubleValue(radius);
          break;
        }

        case BL_IMAGE_SCALE_FILTER_MITCHELL: {
          njs::Value b = ctx.propertyOf(obj, njs::Utf8Ref("b"));
          njs::Value c = ctx.propertyOf(obj, njs::Utf8Ref("c"));

          if (b.isNumber())
            options.mitchell.b = ctx.doubleValue(b);

          if (c.isNumber())
            options.mitchell.c = ctx.doubleValue(c);
          break;
        }
      }
    }

    BLImage dst;
    BLResult err = BLImage::scale(dst, image->_obj, BLSizeI(w, h), filter, &options);

    if (err != BL_SUCCESS) {
      return ctx.returnValue(err);
    }
    else {
      njs::Value ImageClass = ctx.propertyOf(njs::Value(ctx.v8CallbackInfo().Data()), njs::Utf8Ref("Image"));
      njs::Value obj = ctx.newInstance(ImageClass);

      ImageWrap* wrap = ctx.unwrapUnsafe<ImageWrap>(obj);
      wrap->_obj.assign(dst);
      return ctx.returnValue(obj);
    }
  }

  NJS_BIND_STATIC(toRaw) {
    ImageWrap* image;

    NJS_CHECK(ctx.verifyArgumentsLength(1));
    NJS_CHECK(ctx.unwrapArgument(0, &image));

    BLPixelConverter pc;
    BLFormatInfo srcFmt {};

    srcFmt.flags |= BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_PREMULTIPLIED;
    srcFmt.depth = 32;
    srcFmt.setSizes(8, 8, 8, 8);
    srcFmt.setShifts(0, 8, 16, 24);

    BLResult err = pc.create(blFormatInfo[image->_obj.format()], srcFmt);
    if (err)
      return ctx.returnValue(ctx.undefined());

    BLImageData imageData;
    err = image->_obj.getData(&imageData);
    if (err)
      return ctx.returnValue(ctx.undefined());

    unsigned int w = imageData.size.w;
    unsigned int h = imageData.size.h;
    unsigned int bpl = w * 4;
    size_t size = 8 + h * bpl;

    njs::Value buf = njs::Node::newBuffer(ctx, size);
    uint8_t* bufData = static_cast<uint8_t*>(njs::Node::bufferData(buf));

    uint8_t* pixels = static_cast<uint8_t*>(imageData.pixelData);
    intptr_t stride = imageData.stride;

    dd(bufData + 0, w);
    dd(bufData + 4, h);
    pc.convertRect(bufData + 8, bpl, pixels, stride, w, h);

    return ctx.returnValue(buf);
  }
};
#endif

B2DJS_API NJS_MODULE(b2d) {
  typedef v8::Local<v8::FunctionTemplate> FunctionSpec;

  FunctionSpec ImageSpec         = NJS_INIT_CLASS(ImageWrap        , exports);
  FunctionSpec PathSpec          = NJS_INIT_CLASS(PathWrap         , exports);
  FunctionSpec GradientSpec      = NJS_INIT_CLASS(GradientWrap     , exports);
  FunctionSpec PatternSpec       = NJS_INIT_CLASS(PatternWrap      , exports);
  FunctionSpec FontFaceSpec      = NJS_INIT_CLASS(FontFaceWrap     , exports);
  FunctionSpec FontSpec          = NJS_INIT_CLASS(FontWrap         , exports);
  FunctionSpec ContextSpec       = NJS_INIT_CLASS(ContextWrap      , exports);
  FunctionSpec ContextCookieSpec = NJS_INIT_CLASS(ContextCookieWrap, exports);

  njs::Value RuntimeObject = ctx.newObject();
  ctx.setProperty(exports, ctx.newInternalizedString(njs::Latin1Ref("Runtime")), RuntimeObject);

  RuntimeObject.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("memoryInfo")).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), RuntimeWrap::StaticEntry_memoryInfo, exports.v8Handle())->GetFunction());

  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("decode"    )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_decode    , exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("decodeSync")).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_decodeSync, exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("toRaw"     )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_toRaw     , exports.v8Handle())->GetFunction());
  exports.v8HandleAs<v8::Object>()->Set(ctx.newInternalizedString(njs::Latin1Ref("scaleSync" )).v8Handle(), v8::FunctionTemplate::New(ctx.v8Isolate(), ImageIO::StaticEntry_scaleSync , exports.v8Handle())->GetFunction());
}

} // b2djs namespace
