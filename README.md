Octopus Engine
-----------

A cross-platform 2D game engine used in [Octopus City Blues](http://octopuscityblues.com). Octopus Engine is an OpenGL engine written in C++11 and based on the [XD framework](https://github.com/firas-assaad/xd). It was developed to meet the needs of Octopus City Blues in particular, but it might be useful for other adventure games or RPGs.

### Features

  * Partial support for [TMX](https://github.com/bjorn/tiled/wiki/TMX-Map-Format) maps ([Custom Properties](https://docs.google.com/document/d/1Y_l-yU-Zg7KF5-RJbVpVyhJKy6W4WEt6V1TbZNigI7Y/edit?usp=sharing)).
  * Lua scripting features ([Reference](https://docs.google.com/document/d/1GTJ0rVu4J4hg0B49IqWwUqUE9--KCmZ0tMyc6UBsYsE/edit?usp=sharing)).
  * A flexible XML sprite format. See [SpriteEditor](https://bitbucket.org/firas_assaad/spriteeditor).
  * Octopus City Blues-specific features such as NPC scheduling and game time.
  * Tile-based collision detection.
  * Player movement and interaction with various object types.
  * Support for custom GLSL shaders.

### License

See LICENSE.text file.

### Building

** Dependencies **

  * [XD](https://github.com/firas-assaad/xd) - Should be placed in vendor/ directory
  * [FreeType](http://www.freetype.org/index.html)
  * [Lua 5.2](http://www.lua.org/)
  * [Luabind](https://bitbucket.org/uso/luabind) - with Lua 5.2 support
  * [Boost](http://www.boost.org/) - Regex (Required), Unit Test Framework (Optional)
  * [FMOD Studio Programmer's API](http://www.fmod.org/download/)
  * [DevIL](http://openil.sourceforge.net/)
  * [GLFW3](http://www.glfw.org/)
  * [GLEW](http://glew.sourceforge.net/)
  
Start by cloning my XD fork into the vendor/ directory. There are project files for Visual Studio 2012 and Xcode, although I never tested them on another machine. There's a basic Makefile in the linux/ directory for GCC. You can test the executable by placing it in the win/ directory or try it with the [Octopus City Blues](http://ghost-in-a-bottle.itch.io/octopus-city-blues) demo.
 