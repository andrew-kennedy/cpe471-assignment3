In order to build and run this you will need the following libraries installed:

| Library | Description |
| ------- | ----------- |
`SFML` | Windowing, event handling
`AssImp` | Asset importing and loading
`DevIL` | Powerful image loading and manipulation for textures
`glm` | OpenGL math functions in C
`globjects` | Beautiful and useful Object Oriented C++ wrapper around OpenGL objects/constructs
`glbinding`| OpenGL function bindings, replaces GLEW with less bugs

You can install them on MacOS with
```
brew install assimp sfml devil glm globjects glbinding
```

On MacOS, should simply build with CMake. May require modifying the `CMakeLists.txt` for Windows builds, untested on that platform.

I removed all of the ground plane code and specified it an a wavefront object file because mixing code and data is gross and breaks proper separation of concerns.
