<p align="center">
  <img width="300" alt="Image" src="https://github.com/user-attachments/assets/f2e88d7f-33e2-4a6e-b3a1-d9c4f66cb476">
</p>

![Static Badge](https://img.shields.io/badge/Nuke-12.1%20%7C%2012.2%20%7C%2013.0%20%7C%2013.1%20%7C%2013.2%20%7C%2014.0%20%7C%2014.1%20%7C%2015.0%20-brightgreen?style=flat&logo=nuke&logoColor=white&label=nuke) ![Static Badge](https://img.shields.io/badge/supported-brightgreen?style=flat&logo=github&logoColor=white&label=windows) ![Static Badge](https://img.shields.io/badge/supported-brightgreen?style=flat&logo=github&logoColor=white&label=linux)

**DespillAP** is a native CPU-based plugin for Nuke, currently available for Windows and Linux. It is built using the original algorithms developed by **Adrian Pueyo** for the **apDespill** gizmo. You can explore more of his tools at the following [link](https://adrianpueyo.com/gizmos/).

**DespillAP** allows you to remove the spill of a specific color from the input image, set a limit to control the amount of despill applied, inject the overall color to perform a proper despill, and apply a respill based on the desired background.

<p align="center">
  <img width="400" alt="Image" src="https://github.com/user-attachments/assets/b811a4d2-921f-4007-8699-f988f7cbb513">
</p>

# Features

- Select a default color (`Red`, `Green`, `Blue`, or `Pick`), or choose a custom color from the image to remove the spill from the input.
- `Absolute Mode` normalizes the spill relative to the intensity of the selected color. Mostly like a chroma keying algorithm.
- Despill algorithms include `Average`, `Max`, `Min`, and `Custom` weight.
- Hue `offset` adjusts the hue (in degrees) of the precomputed despill, if needed.
- Hue `limit` controls the intensity of the despill effect on the image. If an alpha is connected to the `Limit` input, the despill strength will be driven by the opacity of that alpha.
- Hue `mask` selects the desired channel from the `Limit` input to apply the limit.
- `Protect Tones` excludes a specific color in the image from being affected by the despill.
- Protect `Preview` displays a preview of the area that will be excluded from the despill.
- Protect `color` selects the reference color to be protected from the despill.
- Protect `tolerance`, similar to a threshold, expands or narrows the color range to include similar tones.
- Protect `effect` controls how strongly the selected color is being protected.
- Respill `math` includes `Rec 709`, `Ccir 601`, `Rec 2020`, `Average`, and `Max` for calculating the luminance of spill and respill colors.
- Respill `color` is the color used to replace the selected spill. If the `Respill` input is connected, it will be multiplied by the selected color.
- Respill `blackpoint` and `whitepoint` were added to control the areas where the calculated luminance matte is affecting the respill.

# Build

All builds were created locally using Docker, thanks to the open-source project **NukeDockerBuild** by **Gilles Vink**.  
[GitHub – NukeDockerBuild](https://github.com/gillesvink/NukeDockerBuild).

Once you have the image locally—either for Windows or Linux (`nukedockerbuild:12.1-windows` or `nukedockerbuild:12.1-linux` .. )—you can run all builds sequentially using [Taskfile](https://taskfile.dev/) (`Taskfile.yml`).

```bash
task build-all
```

Or run builds by platform and Nuke version using the format `build-PLATFORM:NUKE_VERSION`:

```bash
task build-linux:12.1
```

```bash
task build-windows:12.1
```

Or build for both platforms using a specific Nuke version with the format `build-single:NUKE_VERSION`:

```bash
task build-single:12.1
```

To clean your output directory after building, use this if a directory named `artifacts` has been created:

```bash
task clean
```

To clean only the `build` folders created during the build process—while keeping the `artifacts` directory, which stores the compiled libraries or binaries—use the following:

```bash
task clean-builds
```

para mostrar un listado de todos los artifacts o binarios compilados simplemente corre:

```bash
task show-artifacts
```

To create `.zip` packages for release, this will generate a separate zip file for each `.so` and `.dll` created, following the naming structure `PluginName-NukeVersion-Platform.zip`.  

Note: This requires the `zip` utility to be installed. On Linux, you can install it using: `sudo apt install zip`.

```bash
task package
```

# License

**DespillAP** is distribuited under the 