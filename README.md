# Computer Graphics Project

A computer graphics project focused on recreating a reference scene through three separate stages: scene construction in Blender, a custom Rasteriser, and a custom Ray Tracer. Both rendering systems were developed from scratch in C++ without the use of third-party graphics libraries.

## Overview

This project explored the fundamental principles of computer graphics by recreating a reference image using multiple rendering techniques.

The project began with constructing the scene in Blender to understand the composition, materials, lighting, and environment of the reference image. The scene was then recreated using a custom Rasteriser and Ray Tracer developed from scratch in C++.

The goal was to gain a deeper understanding of modern rendering techniques and how graphics pipelines operate beneath commercial game engines and graphics APIs.

## Screenshots

### Reference Scene Recreation

#### Blender Scene

<img width="1920" height="1080" alt="Cycles Render" src="https://github.com/user-attachments/assets/c9241455-d71c-40c7-b440-9b77c9f99719" />

#### Rasteriser Output

<img width="1920" height="1080" alt="Rastierer" src="https://github.com/user-attachments/assets/d285f3ea-3230-4ba7-871a-d4e2abe5f4a7" />

#### Ray Tracer Output

<img width="1920" height="1080" alt="RayTracer" src="https://github.com/user-attachments/assets/9f0a1691-fcb6-4c05-8751-c36b2a8a122c" />

## Technologies Used

* C++
* Visual Studio
* CMake
* Blender

## Key Features

### Rasteriser

* Custom software rasterisation pipeline.
* Hidden face removal using a Z-buffer.
* Texture mapping support.
* Ambient occlusion implementation.
* Alpha mapping support.
* Multiple light types:

  * Point Lights
  * Directional Lights
  * Spotlights
  * Area Lights

### Ray Tracer

* Recursive ray tracing pipeline.
* Texture mapping support.
* Metal material rendering.
* Anti-aliasing implementation.
* Custom textured Phong shader.
* Alpha-based transparency (cutout shader).
* Multiple light types:

  * Point Lights
  * Directional Lights
  * Spotlights
  * Area Lights

## My Contribution

This project was developed individually as part of a university Computer Graphics module.

My responsibilities included:

* Constructing the reference scene in Blender.
* Developing a custom rasterisation pipeline.
* Developing a custom ray tracing pipeline.
* Implementing lighting and shading systems.
* Creating texture mapping functionality.
* Implementing hidden surface removal.
* Developing material and transparency systems.
* Implementing anti-aliasing techniques.
* Debugging and optimising rendering systems.

## Challenges and Solutions

### Building Rendering Pipelines from Scratch

One of the biggest challenges was understanding and implementing the complete rendering process without relying on graphics engines or rendering libraries.

This required developing the mathematical and graphical systems responsible for transforming scene data into a final rendered image while ensuring correctness and performance.

### Lighting and Material Systems

Supporting multiple light types and material behaviours required implementing different lighting calculations and shading models.

These systems were expanded incrementally and tested against the reference scene to ensure visually accurate results.

### Ray Tracing Performance

Ray tracing is computationally expensive due to the large number of ray-object intersection tests required.

Performance was improved through optimisation of rendering calculations while maintaining image quality through features such as anti-aliasing and material support.

## What I Learned

* Computer graphics fundamentals.
* Rasterisation pipelines.
* Ray tracing techniques.
* Lighting and shading models.
* Texture mapping.
* Material systems.
* Anti-aliasing techniques.
* Hidden surface removal.
* Graphics mathematics.
* Debugging complex rendering systems.
* CMake project management and build systems.

## Repository Information

Project Type: University Project

Status: Complete

Year: 2024

Author: Jack Hodgetts
