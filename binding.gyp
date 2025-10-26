{
  "targets": [
    {
      "target_name": "infinitecanvas",
      "sources": [
        "src/node_addon.cpp",
        "src/Canvas.cpp",
        "src/Stroke.cpp",
        "src/BezierSmoother.cpp",
        "src/VectorRenderer.cpp",
        "src/ToolWheel.cpp",
        "imgui/imgui.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_tables.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/backends/imgui_impl_glfw.cpp",
        "imgui/backends/imgui_impl_opengl3.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include",
        "/usr/include",
        "/usr/include/glm",
        "imgui",
        "imgui/backends"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "libraries": [
        "-lGL",
        "-lGLEW",
        "-lglfw",
        "-lpthread"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ 
        "NAPI_DISABLE_CPP_EXCEPTIONS"
      ],
      "cflags": [ 
        "-std=c++17",
        "-pthread"
      ],
      "cflags_cc": [ 
        "-std=c++17",
        "-pthread"
      ],
      "ldflags": [
        "-pthread"
      ]
    }
  ]
}
