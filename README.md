# bit
Low-end 2D animation system

# building
Create a subdirectory callled 'build' under the bit directory.
Type cmake .. -DCMAKE_BUILD_TYPE=Debug

# Dependencies
Most of the dependencies are configured in the top-level CMakeLists.txt file. The project is designed to build
as a Linux native hosted system, so bring in the dependencies using your package system.

GTK2, pango, cairo, gtkgl, SDL2, XCB, JSMN

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

# JSMN usage
JSMN is cloned from https://github.com/zserge/jsmn.git in as the jsmn subdirectory of the bit directory. I build a CMakeLists.txt file there that builds JSMN as a 
library, which is pretty easy. Instead of building a proper DOM, the parsing code traverses the array of tokens that JSMN provides
and creates the arrays as it goes.
