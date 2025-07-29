# COMP 371 - Assignment 1: Interactive 3D Graphics Scene 

- Created by Jeremy Crete, Colton Leblond and Charles Eimer  
- Concordia University - Summer 2025  
- Course: COMP 371 - Computer Graphics

## Table of Contents

- [Badges](#badges)
- [Features Overview](#features-overview)
- [Assignment Requirements Checklist](#assignment-requirements-checklist)
  - [Libraries Used](#libraries-used)
  - [Interactive Camera](#inetractive-camera)
  - [Scene Content](#scene-content)
  - [Advanced Features](#advanced-features-choose-1-for-this-assignment-and-implement-the-other-next-assignment)
- [Demo](#demo)
- [Controls](#controls)
- [Credits](#credits)


## Badges
![OpenGL](https://img.shields.io/badge/OpenGL-4.x-blue?logo=opengl)
![GLFW](https://img.shields.io/badge/GLFW-enabled-brightgreen)
![GLEW](https://img.shields.io/badge/GLEW-included-blueviolet)
![GLM](https://img.shields.io/badge/GLM-vector%20math-orange)
![Platform](https://img.shields.io/badge/Platform-macOS-lightgrey)
![Build](https://img.shields.io/badge/Build-manual%20or%20CMake-yellow)
![License](https://img.shields.io/badge/License-Academic-green)

## Features Overview

This project implements a 3D scene using OpenGL. It features interactive camera movement, multiple textures and dynamic lighting with multiple light sources.

## Assignment Requirements Checklist

### Libraries Used
- [X] OpenGL
- [X] GLFW
- [X] GLM
- [X] GLEW
- [X] stb_image.h
- [ ] OBJLoader (assignment 2)

### Inetractive Camera
- [X] Mouse controlled rotation to change the viewing direction of the scene
- [X] Keyboard controlled translation (WASD style)
- [X] Movement is relative to the camera

### Scene Content
- [X] Multiple Textured surfaces (ex: Skybox, Ground, Cubes)

### Advanced Features (*choose 1 for this assignment and implement the other next assignment*)
- [X] **Dynamic lighting** with multiple moving light sources
- [ ] Hierarchical animation with at least 2 levels of rotation

## Demo

![Assignment 1 Demo](media/demo.gif)

## Controls

| Actions         | Key / Input         |
|-----------------|---------------------|
| Move Forward        | `W`                |
| Move Backward       | `S`                |
| Move Left           | `A`                |
| Move Right          | `D`                |
| Increase Movement Speed          | `SHIFT`                |
| Look Around         | Mouse movement     |
| Exit Program        | `ESC`              |

## Credits
- [Cloud Texture / Skybox](https://polyhaven.com/a/kloofendal_48d_partly_cloudy_puresky)





