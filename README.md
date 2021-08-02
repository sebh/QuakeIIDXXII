# QuakeIIDXXII

![dx12appscreenshot](QuakeIIDXXII.png)

Quake II on Dx12. It has been tarted from [https://github.com/oraoto/quake2-vs2015](https://github.com/oraoto/quake2-vs2015) because it is the original code and it was compiling out of the box.

The goal of this project was to verify that my [Dx12 basic thin abstraction layer](https://github.com/sebh/Dx12Base) could work is such a use case. And I always wanted to put my nose in Quake2's code for fun! The dx12 implementation is far from perfect and lots could be improved (especially around data upload to GPU and batching). But it works :)

QuakeIIDXII looks similar to the original Quake 2 but there are likely missing things and bugs. I will not provide any support but I welcome bug fixes or suggestions that I will process when I have the time.
Only the win32 platform compiles and run correctly for now. In this case, dx12 is limited to 4GB of GPU video memory and no ray tracing is available.

Main modifications done to the renderer as compared to OpenGL:
- Drawing is no longer done inline: drawcalls are recorded while updating vertex buffers. All draw call are issued at the end of the frame. More could be done such as preparing static vertex buffers for the world but it does not seem necessary.
- Mesh triangle list, fan and polys are batched in as few draw calls as possible if contiguous drawcall have the same state/parameterisation. Batches are triangle list only now.
- Lightmap upload is also no longer done inline&interleaved with draw calls for subparts of textures. Static/dynamic Lightmaps are maintained CPU side and only the one receiving dynamic lighting are uploaded to GPU memory before the world is finally drawn using them.

New cvars:
- *dx_batchworldtriangles*: whether or not to batch meshes tris list/fan and polys according to render state. 1 by default because it result in less batches and it is a lot faster (roughly at least 4x less drawcalls).
- *dx_showworldbatches*: show drawcalls/batches with single colors. This reveal the impact of *dx_batchworldtriangles* on draw call count.
- *dx_showlightmaps*: show used lightmaps on screen and highlight in green the one being uploaded this frame due to dynamic lighting.


Build the solution
1. Open the solution in Visual Studio
2. Make sure you select a windows SDK and platform toolset you have locally for each projects
3. Select Quake2 as the startup project
4. Select the _x86_ platform and either _Debug_ or _Release_ configuration.
5. Change _Quake 2_ project the _Application_ project _Working Directory_ from `$(ProjectDir)` to your local full game folder. For instance: D:\Apps\Steam\steamapps\common\Quake 2
6. Open *dx_shader.cpp* and update the full path to the shader file. (this really needs to be fixed)
7. Hit F5

Seb
