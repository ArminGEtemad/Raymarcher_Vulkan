# Vulkan Raymarching grapher

> **Status:** Project Under Construction!
>
> **Target:** Having a finished Demo for Digital Dragons 2026

A real-time 3D implicit grapher written in Vulkan + GLSL

In my last two big projects I was using WGPU, [Reaction-Diffusion](https://github.com/ArminGEtemad/reaction_diffusion_wgpu) and [3D implicit grapher](https://github.com/ArminGEtemad/implicit_grapher_wgpu). Which was awesome and I had a lot of fun! But where is the fun if there is no suffering? I would like to implement my 3D implicit grapher in WGPU now in Vulkan. In WGPU project, however, I decided to go with sphere marching. Here, I would like to focus on `Volumetric Integration`. I tried it once in my last projects before moving back to the sphere marching but I did not do a good job. so now here we are trying it again.

In this project, I am not going to build my own parser and to keep the version 1 lean, I have decided to keep the window size constant and I will get back to it later.

## Milestones

### Third Focus

- [ ] Anti-Aliasing
- [ ] Finishing touches

### Second Focus

- [x] Porting Implicit SDF Math from WGPU
- [x] Volumetric Cone Integration
- [ ] Add dynamical scene

### First Focus

- [x] Basic Vulkan setup
- [x] Bufferless Full-Screen Triangle Pipeline
- [x] Basic Raymarcher Prototype
- [x] Camera Setup
