# 4D Visualizer

The fourth spatial dimension is inherently difficult to visualize. Leveraging the performance of Vulkan, we have implemented an application for users to explore the fourth dimension with full freedom of movement. The user has a properly-modeled field of view into the fourth dimension similar to the field of view one would normally get in the X, Y, and Z directions on a screen. The user can also collide with portions of the world to demonstrate itneresting properties of four-dimensional motion. We hope that this application helps the user feel embedded in a true four-dimensional world.

## Overview

|![Observing some rotations with a 4D Grid](img/4D_Grid_Solid.gif)|
|:-:|
|Observing some motion for an interesting 4D Grid.|

The implementation relies on compute shaders because vertex and fragment shaders do not readily support the math required to extend into extra dimensions. For this project’s framework we will be using [Anvil](https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/), a wrapper for Vulkan in development by AMD. The application can generate and view voxel scenes made out of tesseracts.

## Operation

Currently, this application is only expected to operate properly on Linux platforms. Important Windows paths are currently hardcoded and some functionality is not enabled on the platform.

To run the visualizer, supply the following mandatory arguments to the executable binary:
```
<Width> <Height> <Scene File | PERLIN | OPENSIMPLEX>
```

`<Width>` specifies the width of the visualizer window to launch.

`<Height>` specifies the width of the visualizer window to launch.

`<Scene File | PERLIN | OPENSIMPLEX>` specifies where to obtain mesh data for visualizing. This parameter can be the path to a file containing a series of individual tesseract coordinates listed in the space-delimited format "x y z w" with one set of coordinates per line. Example scenes are provided in the "scenes" folder. 

This parameter can also be provided as "PERLIN" or "OPENSIMPLEX" to have the visualizer generate its own coordinate data. If either of these options are chosen, the remaining arguments will alter additional parameters of the generation algorithm. If "PERLIN" is specified, our own four-dimensional Perlin implementation will be used to generate a contiguous piece of terrain. If "OPENSIMPLEX" is specified, Stephen Cameron's [open-simplex-noise-in-c](https://github.com/smcameron/open-simplex-noise-in-c) implementation is used to generate the terrain.

`[persistence]` is an optional parameter for the "PERLIN" setting which specifies the [persistence value](http://libnoise.sourceforge.net/tutorials/tutorial4.html) for shaping the terrain. Under the "OPENSIMPLEX" scheme, this parameter is used to alter the seed used for generating terrain.

`[frequency]` is an optional parameter for the "PERLIN" setting which specifies the [frequency value](http://libnoise.sourceforge.net/tutorials/tutorial4.html) for shaping the terrain. Under the "OPENSIMPLEX" scheme, this parameter is used to alter the seed used for generating terrain.

`[x size]` is an optional parameter specifying the length along the x-direction of the generated piece of terrain.

`[y size]` is an optional parameter specifying the length along the y-direction of the generated piece of terrain.

`[z size]` is an optional parameter specifying the length along the z-direction of the generated piece of terrain.

`[w size]` is an optional parameter specifying the length along the w-direction of the generated piece of terrain. This is the "fourth" axis which we have added to space in this visualizer.

## Controls

## Features

This visualizer application includes the following features to make it a more interactive and useful exploration tool:

### Compute Shader

A Vulkan compute shader is used to generate the scene data given a buffer of coordinates. For every set of coordinates in the list provided to the compute shader, a GPU thread is used to generate a unit-[tesseract](https://en.wikipedia.org/wiki/Tesseract) centered about that set. The compute shader then reads the camera's view matrix information to appropriately transform the scene. Depending on the rendering mode

### Translation and Rotation in Four Dimensions

We have implemented some rewritten matrix code for five by five transformation matrices in order to support moving and rotating the view in this new visualizer setup.

### Procedural Generation

The application supports procedural generation. We will start using a Minecraft-like system of (hyper)cubes. We will also be able to manually create simple polychora such a hypercube/tesseract, 4-simplex (related to tetrahedra), or duoprisms. One interesting example would be an actual non-intersecting klein bottle.

### Collisions

### User-specified Scenes


Once we have a working renderer, there are some stretch goals we would like to achieve. We would like add better interactivity to the demo. We would also like to extend the marching tetrahedra algorithm to transform noise into nicer polygons. Finally, depending on the speed of the simulation, we would like to add some sort of acceleration structure such as a k-d tree or a hexadecitree to help improve collision detection performance.

Milestone One (11/20): Rewrite matrix code for five by five transformation matrices and five-deep vectors for tracking points and directions; support moving camera in new setup.
Begin rasterization work to display simple 4D objects; shape changes with view.

Milestone Two (11/27): Generate 4D perlin noise with desired characteristics. Implement collision detection between user (camera) and environment.

Milestone Three (12/04): Implement interesting visualization quirks unique to the 4D environment, such as a lighting model which supports 4D shadows.

Milestone Four (12/11): Structure to optimize physics performance and non-player physics objects.

## Benchmarks

By far the slowest part of our visualizer is the baking compute manager baking process. This is an Anvil-added process where the buffer of mesh coordinates is processed by the compute shader. While generating the mesh coordinates is very fast, as the number of meshes increases it takes progressively longer to initialize the scene. The following benchmarks were taken on a Windows 10, i5-4590 @ 3.30GHz 8GB, GTX 970 4GB desktop computer.

<p align="center">
  <img src="img/bakeTimes.png"/>
</p>

It is worth noting that this baking process only needs to happen once upon scene initialization. In all cases tested, performance was well above the monitor refresh rate and the visualizer was not impacted. The difference between lines and triangles is in line with the amount of data that must be produced: a solid mesh must generate 144 output data points whereas a wireframe mesh generates only 64. Improving the performance of this baking process might be possible by implementing some sort of chunking mechanism to combine neighboring meshes into larger triangles. However, this is difficult to conceptualize in all dimensions so we stuck with the approach of constructing larger meshes out of discrete tesseracts.

## Conclusions

## Libraries and Supporting Code

This visualizer application would not have been possible without support from the following:
- [Anvil](https://github.com/GPUOpen-LibrariesAndSDKs/Anvil/), AMD's Vulkan wrapper.
- [open-simplex-noise-in-c](https://github.com/smcameron/open-simplex-noise-in-c), Stephen Cameron's C implementation of Kurt Spencer's Java implementation of open simplex, a free alternative to the simplex noise algorithm.

## References:

We also found the following resources useful in understanding the fourth spatial dimension and how to implement this application:
- [4-polytope](https://en.wikipedia.org/wiki/4-polytope), for an overview of four-dimensional shapes.
- [libnoise](http://libnoise.sourceforge.net/tutorials/tutorial4.html), for a description of Perlin noise parameters.
