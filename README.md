Blend2D JS Bindings
-------------------

Blend2D bindings for node.js.

Disclaimer
----------

This is a proof-of-concept implementation that uses NJS-API to bind Blend2D classes into JS. The bindings match some JS and HTML5 conventions (for example using string representations of enumerations).

Wrapped Classes
---------------

  * `Path`
  * `Image`
  * `Pattern`
  * `Gradient`
  * `Font`
  * `FontFace`
  * `Context`
  * `ContextCookie`
  * `Runtime`

Building
--------

These bindings require Blend2D library to be installed and available including public Blend2D C++ headers. NJS-API that is used to create bindings should be installed by using `npm install` as it's specified in package dependencies.
