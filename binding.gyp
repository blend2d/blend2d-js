{
  "target_defaults": {
    "default_configuration": "Release",
    "configurations": {
      "Debug"  : { "defines": ["_DEBUG"] },
      "Release": { "defines": ["NDEBUG"] }
    },
    "conditions": [
      ["OS=='linux' or OS=='mac'", {
        "cflags": ["-fvisibility=hidden"]
      }]
    ]
  },
  "targets": [{
    "target_name": "b2djs",
    "sources": [
      "b2djs.cpp",
      "b2djs.h"
    ],

    "include_dirs": [
      "../b2d/src"
    ],

    "conditions": [
      ["OS == 'win'", {
        "library_dirs": [
          "../b2d/build_vs2017_x64/Release",
          "../b2d/build_vs2017_x64/Debug"
        ],
        "libraries": [
          "-lb2d"
        ]
      }, {
        "libraries": [
          "-L../../b2d/build_rel",
          "-L../../b2d/build_dbg",
          "-lb2d"
        ]
      }]
    ]
  }]
}
