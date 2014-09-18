# Electro

Electro is an application development environment designed for use on cluster-driven tiled displays, virtual reality systems, and desktop workstations. Electro is based on the MPI process model and is bound to the Lua programming language. With support for 3D graphics, 2D graphics, audio, networking, and input handling, Electro provides an easy-to-use scripting system for interactive applications spanning multiple hosts and a variety of displays.

## Running

The executable takes a list of Lua files as parameters.  Each file
is executed in turn.  Here, the Asteroids game is used as an example:

To run Asteroids under Linux or OS X:

    $ electro asteroids.lua

To run the asteroidss under Windows:

    C:\> electro.exe asteroids.lua

Alternatively, drag and drop the asteroids.lua icon onto ELECTRO.BAT.

## Usage

    ESC    - Exit
    F1     - Toggle server console display
    F2     - Toggle server full screen display
    F3     - Decrease server window resolution
    F4     - Increase server window resolution
    PrtScr - Screenshot "snap.png"
