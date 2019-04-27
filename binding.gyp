{
  "target_defaults": {
    "default_configuration": "Release",
    "configurations": {
      "Release": { "defines": ["NDEBUG"] }
    },
    "conditions": [
      ["OS=='linux' or OS=='mac'", {
        "cflags_cc": ["-std=c++17", "-fvisibility=hidden"],
        "cflags_cc!": ["-std=gnu++1y"]
      }]
    ]
  },
  "targets": [{
    "target_name": "b2djs",
    "sources": [
      "blend2d-js.cpp",
      "blend2d-js.h"
    ],

    "include_dirs": [
      "<!@(node -p \"require('njs-api').include\")",
      "../blend2d/src"
    ],

    "conditions": [
      ["OS == 'win'", {
        "library_dirs": [
          "../blend2d/build_vs2017_x64/Release",
          "../blend2d/build_vs2017_x64/Debug"
        ],
        "libraries": [
          "-lblend2d"
        ]
      }, {
        "libraries": [
          "-L../../blend2d/build_rel",
          "-L../../blend2d/build_dbg",
          "-lblend2d"
        ]
      }]
    ]
  }]
}
