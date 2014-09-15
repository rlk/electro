# Electro


## Running on PCs

The run scripts take a list of Lua files as parameters.  Each file
is executed in turn.  Here, the demo menu is used as an example:

To run the demos under Linux or OS X:

    $ electro demo.lua

To run the demos under Windows:

    C:\> electro.exe demo.lua

Alternatively, drag and drop the demo.lua icon onto ELECTRO.BAT.


## Running on clusters

Each cluster has an associated run script.  To run the demos on nico,
make sure that "/DEMO/evl/Electro/bin" is included in your PATH,
change to the Electro examples directory, and run it as follows:

    $ cd /DEMO/evl/Electro/examples
    $ electro-nico11x5 demo.lua

On yorda:

    $ cd /DEMO/evl/Electro/examples
    $ electro-yorda11x5 demo.lua

Vortex works best when Electro is allowed to capture the mouse pointer:

    $ electro-nico11x5 -m vortex.lua
or
    $ electro-yorda11x5 -m vortex.lua


## Usage

    ESC    - Exit
    F1     - Toggle server console display
    F2     - Toggle server full screen display
    F3     - Decrease server window resolution
    F4     - Increase server window resolution
    PrtScr - Screenshot "snap.png"
