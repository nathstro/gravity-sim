# C++ Gravity Simulator

## What is this?
This is a real-time N-body gravity simulation created in C++ using OpenGL.

## How does it work?
* The Renderer class handles all rendering and physics.
* Each frame, the acceleration of each body is calculated using the gravitational acceleration formula:
	* a = GM / r^2
	* This formula uses the mass and the distance between each body from all other bodies.
* Verlet integration is used to mostly accurately calculate the change in position each frame.
* Each body is rendered as a sphere.

## Controls
* WASD to move forward/left/backward/right
* Q to move up; E to move down
* P to cause lag

# This project is unfinished.
