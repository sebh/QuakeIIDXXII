# QuakeIIDXXII

Quake II on Dx12 (Started from https://github.com/oraoto/quake2-vs2015)

Build the solution
1. Open the solution in Visual Studio
2. Make sure you select a windows SDK and platform toolset you have locally for each projects
3. Select Quake2 as the startup project
4. Select the _x86_ platform and either _Debug_ or _Release_ configuration.
5. Change _Quake 2_ project the _Application_ project _Working Directory_ from `$(ProjectDir)` to your local full game folder. For instance: D:\Apps\Steam\steamapps\common\Quake 2
6. Open *dx_shader.cpp* and update the full path to the shader file. (this really needs to be fixed)
7. Hit F5

The dx12 implementation is far from perfect and lots could be done, (especially around data upload to GPU and batching). But it works and allowed me to check my [Dx12 basic thin abstraction layer](https://github.com/sebh/Dx12Base).

Only the win32 platform compiles and run correctly for now. So dx12 is limited to 4GB of GPU video memory and no ray tracing is available.

Seb
