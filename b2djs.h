// [B2D-JS]
// Blend2D javascript bindings.
//
// [License]
// Public Domain <http://unlicense.org>

// [Guard]
#ifndef _B2DJS_H
#define _B2DJS_H

// [Dependencies]
#include <b2d/b2d.h>

#include "../njs/njs.h"
#include "../njs/njs-extension-enum.h"

namespace b2djs {

// ============================================================================
// [b2djs::Begin]
// ============================================================================

#define B2DJS_API

// ============================================================================
// [b2djs::JSModule]
// ============================================================================

struct JSModule {
  enum : uint32_t { kBaseTag = 0x3FFFAA00u };
};

// ============================================================================
// [b2djs::JSCookie]
// ============================================================================

struct JSCookie {
  NJS_BASE_CLASS(JSCookie, "Cookie", JSModule::kBaseTag + 0)

  NJS_INLINE JSCookie() noexcept : _obj() {}
  NJS_INLINE JSCookie(const b2d::Cookie& other) noexcept : _obj(other) {}
  NJS_INLINE ~JSCookie() noexcept {}

  b2d::Cookie _obj;
};

// ============================================================================
// [b2djs::JSImage]
// ============================================================================

struct JSImage {
  NJS_BASE_CLASS(JSImage, "Image", JSModule::kBaseTag + 1)

  NJS_INLINE JSImage() noexcept : _obj() {}
  NJS_INLINE JSImage(const b2d::Image& img) noexcept : _obj(img) {}
  NJS_INLINE ~JSImage() noexcept {}

  b2d::Image _obj;
};

// ============================================================================
// [b2djs::JSPath2D]
// ============================================================================

struct JSPath2D {
  NJS_BASE_CLASS(JSPath2D, "Path2D", JSModule::kBaseTag + 2)

  NJS_INLINE JSPath2D() noexcept {}
  NJS_INLINE explicit JSPath2D(const b2d::Path2D& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~JSPath2D() noexcept {}

  b2d::Path2D _obj;
};

// ============================================================================
// [b2djs::JSPattern]
// ============================================================================

struct JSPattern {
  NJS_BASE_CLASS(JSPattern, "Pattern", JSModule::kBaseTag + 3)

  NJS_INLINE JSPattern() noexcept {}
  NJS_INLINE explicit JSPattern(const b2d::Pattern& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~JSPattern() noexcept {}

  b2d::Pattern _obj;
};

// ============================================================================
// [b2djs::JSGradient]
// ============================================================================

struct JSGradient {
  NJS_BASE_CLASS(JSGradient, "Gradient", JSModule::kBaseTag + 4)

  NJS_INLINE JSGradient(uint32_t typeId, double v0, double v1, double v2, double v3, double v4 = 0.0, double v5 = 0.0) noexcept
    : _obj(typeId, v0, v1, v2, v3, v4, v5) {}
  NJS_INLINE ~JSGradient() noexcept {}

  b2d::Gradient _obj;
};

// ============================================================================
// [b2djs::JSLinearGradient]
// ============================================================================

struct JSLinearGradient : public JSGradient {
  NJS_INHERIT_CLASS(JSLinearGradient, JSGradient, "LinearGradient")

  NJS_INLINE JSLinearGradient(double x0 = 0.0, double y0 = 0.0, double x1 = 0.0, double y1 = 0.0) noexcept
    : JSGradient(b2d::Any::kTypeIdLinearGradient, x0, y0, x1, y1) {}
};

// ============================================================================
// [b2djs::JSRadialGradient]
// ============================================================================

struct JSRadialGradient : public JSGradient {
  NJS_INHERIT_CLASS(JSRadialGradient, JSGradient, "RadialGradient")

  NJS_INLINE JSRadialGradient(double cx = 0.0, double cy = 0.0, double fx = 0.0, double fy = 0.0, double cr = 0.0) noexcept
    : JSGradient(b2d::Any::kTypeIdRadialGradient, cx, cy, fx, fy, cr) {}
};

// ============================================================================
// [b2djs::JSConicalGradient]
// ============================================================================

struct JSConicalGradient : public JSGradient {
  NJS_INHERIT_CLASS(JSConicalGradient, JSGradient, "ConicalGradient")

  NJS_INLINE JSConicalGradient(double cx = 0.0, double cy = 0.0, double angle = 0.0) noexcept
    : JSGradient(b2d::Any::kTypeIdConicalGradient, cx, cy, angle, 0.0) {}
};

// ============================================================================
// [b2djs::JSFontFace]
// ============================================================================

struct JSFontFace {
  NJS_BASE_CLASS(JSFontFace, "FontFace", JSModule::kBaseTag + 5)

  NJS_INLINE JSFontFace() noexcept {}
  NJS_INLINE explicit JSFontFace(b2d::FontFace& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~JSFontFace() noexcept {}

  b2d::FontFace _obj;
};

// ============================================================================
// [b2djs::JSFont]
// ============================================================================

struct JSFont {
  NJS_BASE_CLASS(JSFont, "Font", JSModule::kBaseTag + 6)

  NJS_INLINE JSFont() noexcept {}
  NJS_INLINE explicit JSFont(b2d::Font& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~JSFont() noexcept {}

  b2d::Font _obj;
};

// ============================================================================
// [b2djs::JSContext2D]
// ============================================================================

struct JSContext2D {
  NJS_BASE_CLASS(JSContext2D, "Context2D", JSModule::kBaseTag + 7)

  NJS_INLINE JSContext2D() noexcept {}
  NJS_INLINE explicit JSContext2D(b2d::Image& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~JSContext2D() noexcept {}

  b2d::Context2D _obj;
};

} // b2d namespace

// [Guard]
#endif // _B2DJS_H
