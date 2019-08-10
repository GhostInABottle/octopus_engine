Octopus Engine
-----------

A cross-platform 2D game engine used in [Octopus City Blues](http://octopuscityblues.com). Octopus Engine is an OpenGL engine written in C++17 and based on the [XD framework](https://github.com/rekotiira/xd). It was developed to meet the needs of Octopus City Blues in particular, but it might be useful for other types of games.

### Features

  * Partial support for [TMX](https://github.com/bjorn/tiled/wiki/TMX-Map-Format) maps ([Custom Properties](https://docs.google.com/document/d/1Y_l-yU-Zg7KF5-RJbVpVyhJKy6W4WEt6V1TbZNigI7Y/edit?usp=sharing)).
  * Lua scripting features ([Reference](https://docs.google.com/document/d/1GTJ0rVu4J4hg0B49IqWwUqUE9--KCmZ0tMyc6UBsYsE/edit?usp=sharing)).
  * A flexible XML sprite format. See [SpriteEditor](https://bitbucket.org/firas_assaad/spriteeditor).
  * Tile-based collision detection.
  * Customizable gamepad and keyboard input.
  * Player movement and interaction with various object types.
  * HarfBuzz complex font-shaping support.
  * Support for custom GLSL shaders.
  * A Qt-based editor.

### License

See LICENSE.text file.

### Building

Dependencies:

  * [FreeType](http://www.freetype.org/index.html)
  * [HarfBuzz](https://www.freedesktop.org/wiki/Software/HarfBuzz/)
  * [Lua 5.2](http://www.lua.org/)
  * [Luabind](https://github.com/Oberon00/luabind) - Oberon00 version
  * [Boost](http://www.boost.org/) - lexical cast, string algorithms and unit testing
  * [FMOD Studio Programmer's API](http://www.fmod.org/download/)
  * [GLFW3](http://www.glfw.org/)
  * [GLEW](http://glew.sourceforge.net/)
  * [Qt 5.6](https://www.qt.io/) - For the editor
  
There are project files for Visual Studio 2015 and Xcode, although I never tested them on another machine. There's a basic Makefile in the linux/ directory for GCC. You can test the executable by placing it in the win/ directory.
 