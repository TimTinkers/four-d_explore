# 4D Visualizer

## Overview

The fourth spatial dimension is inherently difficult to visualize. Leveraging the performance of Vulkan, we will implement an application for users to explore the fourth dimension with full freedom of movement. Additionally, the user will have a field of view into the fourth dimension similar to the field of view one would normally get in the X and Y directions on a screen. Finally, we will offer the user some way to interact with the world through collision detection. All of this together will go a long way to making the user feel embedded in a true four-dimensional world. Such an application would be a great improvement over existing visualizers.

The implementation relies on compute shaders because vertex and fragment shaders do not readily support the math required to extend into extra dimensions. For this project’s framework we will be using Anvil, a wrapper for Vulkan in development by AMD. We could use CUDA but felt that the graphical nature of this visualizer makes a graphics API more appealing.

The application environment will be procedurally generated. We will start using a Minecraft-like system of (hyper)cubes. We will also be able to manually create simple polychora such a hypercube/tesseract, 4-simplex (related to tetrahedra), or duoprisms. One interesting example would be an actual non-intersecting klein bottle.

Once we have a working renderer, there are some stretch goals we would like to achieve. We would like add better interactivity to the demo. We would also like to extend the marching tetrahedra algorithm to transform noise into nicer polygons. Finally, depending on the speed of the simulation, we would like to add some sort of acceleration structure such as a k-d tree or a hexadecitree to help improve collision detection performance.

## Milestone Goals

Milestone One (11/20): Rewrite matrix code for five by five transformation matrices and five-deep vectors for tracking points and directions; support moving camera in new setup.
Begin rasterization work to display simple 4D objects; shape changes with view.

Milestone Two (11/27): Generate 4D perlin noise with desired characteristics. Implement collision detection between user (camera) and environment.

Milestone Three (12/04): Implement interesting visualization quirks unique to the 4D environment, such as a lighting model which supports 4D shadows.

Milestone Four (12/11): Structure to optimize physics performance and non-player physics objects.

## References:
https://github.com/smcameron/open-simplex-noise-in-c

https://en.wikipedia.org/wiki/4-polytope

https://github.com/GPUOpen-LibrariesAndSDKs/Anvil

https://en.wikipedia.org/wiki/Marching_tetrahedra
