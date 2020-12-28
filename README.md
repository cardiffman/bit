# bit
Low-end 2D animation system with a few implementations.

# building
Create a subdirectory callled 'build' under the bit directory.
Type

    cmake .. -DCMAKE_BUILD_TYPE=Debug USE_JSMN=1

See below for building and dependencies.

# Animation system theory
The system is based on a stereotype of needing to move things around but not needing to rotate among sprites. So,
There exist containers, which are strictly rectangular. They may be filled with a color, filled with a picture,
or filled with child containers. A container with children will not be drawn itself. The term Asset is used for pictures
and other renderings that go in containers.

# Animation system practice
The tree of containers is translated to a vector and each container has an ID and a parent ID. It is easy to draw the
container tree properly with this system.

The assets are kept as another vector. A container that has a bitmap image refers to it as its asset ID.

Once you get to about 40 containers, it is way too hard to populate the vectors used by the rendering code.
So a JSMN-based parser supports a notation where containers are objects and can contain a member called
containers which is an array of the child containers.


   {
     "containers": [ { "area": { "x":0, "y":0, "width":1280, "height":720 }, "fill":[255,20,40,70] },
                     {"area": { "x":640, "y":360, "width": 640, "height":360},"asset":"bitmap1"}
                ]
     "assets": [ {"label":"bitmap1", "url":"./bitmap.jpg" } ]
   }

# Various rendering engines
The reason there are several programs doing the same thing is to explore the implementation space. We have XCB with and without MIT-SHM, we have SDL code which I'm sure is using OpenGL underneath, we have a GTK setup that uses a little cairo to get things done. There is an abstraction of GraphicsEngine's which can render the scene built by the builder. They have to support the assets the fills and other ways containers get drawn. Right now all these engines use the same code to do this, and only vary in how the graphics get to the screen. I think this could change easily using the present GraphicsEngine abstraction.

# Building
This project can be built in Cygwin or Linux. When more of us, particularly myself, learn how to do X Windows under WSL 2, that will be added.

# Dependencies
Most of the dependencies are configured in the top-level CMakeLists.txt file. The project is designed to build
as a Linux native hosted system, so bring in the dependencies using your package system.

GTK2, freetype2, cairo, gtkgl, SDL2, XCB, and either JSMN, jsoncpp, or cmjson

## JSON dependency
At the time of this writing, the new cmjson JSON parser I'm making is not quite done, even though it is the default
if you don't define USE_JSMN or USE_JSONCPP. Some scenes might fail to parse that JSONCPP can parse.

### JSMN
JSMN is cloned from https://github.com/zserge/jsmn.git in as the jsmn subdirectory of the bit directory. I build a CMakeLists.txt file there that builds JSMN as a
library, which is pretty easy. Instead of building a proper DOM, the parsing code traverses the array of tokens that JSMN provides
and creates the container and asset arrays as it goes.

### JSONCPP
JSONCPP is cloned from https://github.com/open-source-parsers/jsoncpp.git to the jsoncpp subdirectory of the bit directory. It comes with a cmake build system. The cmake build system
will be invoked recursively from our build system, and that build system modifies some global Cmake parameters. This results in some of our executables ending up in bit/build/bin, at least under Cygwin.

# Environmental variances
Under Cygwin, the latest compiler can handle C++20. Under Debian 9, which is still a more-or-less viable OS, the latest G++ can only handle C++14, so that is the current
standard for this project.

Under Cygwin, the win64 directory contains a "Windows" program called 'win'. To launch it properly use cygstart so that it sees your entire command. 
