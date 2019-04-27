// [Blend2D-JS]
// Blend2D javascript bindings.
//
// [License]
// Public Domain <http://unlicense.org>

#ifndef BLEND2D_JS_H
#define BLEND2D_JS_H

#include <blend2d.h>
#include <njs-api.h>
#include <njs-extension-enum.h>

namespace bljs {

// ============================================================================
// [bljs::Module]
// ============================================================================

struct Module {
  enum : uint32_t { kBaseTag = 0x3FFFAA00u };
};

// ============================================================================
// [bljs::ContextCookieWrap]
// ============================================================================

struct ContextCookieWrap {
  NJS_BASE_CLASS(ContextCookieWrap, "Cookie", Module::kBaseTag + 0)

  NJS_INLINE ContextCookieWrap() noexcept : _obj() {}
  NJS_INLINE ContextCookieWrap(const BLContextCookie& other) noexcept : _obj(other) {}
  NJS_INLINE ~ContextCookieWrap() noexcept {}

  BLContextCookie _obj;
};

// ============================================================================
// [bljs::ImageWrap]
// ============================================================================

struct ImageWrap {
  NJS_BASE_CLASS(ImageWrap, "Image", Module::kBaseTag + 1)

  NJS_INLINE ImageWrap() noexcept : _obj() {}
  NJS_INLINE ImageWrap(const BLImage& img) noexcept : _obj(img) {}
  NJS_INLINE ~ImageWrap() noexcept {}

  BLImage _obj;
};

// ============================================================================
// [bljs::PathWrap]
// ============================================================================

struct PathWrap {
  NJS_BASE_CLASS(PathWrap, "Path", Module::kBaseTag + 2)

  NJS_INLINE PathWrap() noexcept {}
  NJS_INLINE explicit PathWrap(const BLPath& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~PathWrap() noexcept {}

  BLPath _obj;
};

// ============================================================================
// [bljs::PatternWrap]
// ============================================================================

struct PatternWrap {
  NJS_BASE_CLASS(PatternWrap, "Pattern", Module::kBaseTag + 3)

  NJS_INLINE PatternWrap() noexcept {}
  NJS_INLINE explicit PatternWrap(const BLPattern& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~PatternWrap() noexcept {}

  BLPattern _obj;
};

// ============================================================================
// [bljs::GradientWrap]
// ============================================================================

struct GradientWrap {
  NJS_BASE_CLASS(GradientWrap, "Gradient", Module::kBaseTag + 4)

  NJS_INLINE GradientWrap() noexcept
    : _obj() {}
  NJS_INLINE GradientWrap(uint32_t type, const double* values) noexcept
    : _obj(type, values) {}
  NJS_INLINE ~GradientWrap() noexcept {}

  BLGradient _obj;
};

// ============================================================================
// [bljs::FontFaceWrap]
// ============================================================================

struct FontFaceWrap {
  NJS_BASE_CLASS(FontFaceWrap, "FontFace", Module::kBaseTag + 5)

  NJS_INLINE FontFaceWrap() noexcept {}
  NJS_INLINE explicit FontFaceWrap(BLFontFace& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~FontFaceWrap() noexcept {}

  BLFontFace _obj;
};

// ============================================================================
// [bljs::FontWrap]
// ============================================================================

struct FontWrap {
  NJS_BASE_CLASS(FontWrap, "Font", Module::kBaseTag + 6)

  NJS_INLINE FontWrap() noexcept {}
  NJS_INLINE explicit FontWrap(BLFont& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~FontWrap() noexcept {}

  BLFont _obj;
};

// ============================================================================
// [bljs::ContextWrap]
// ============================================================================

struct ContextWrap {
  NJS_BASE_CLASS(ContextWrap, "Context", Module::kBaseTag + 7)

  NJS_INLINE ContextWrap() noexcept {}
  NJS_INLINE explicit ContextWrap(BLImage& obj) noexcept : _obj(obj) {}
  NJS_INLINE ~ContextWrap() noexcept {}

  BLContext _obj;
};

} // {bljs}

#endif // BLEND2D_JS
