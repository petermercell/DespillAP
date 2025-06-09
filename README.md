<p align="center">
  <img width="300" alt="Image" src="https://github.com/user-attachments/assets/f2e88d7f-33e2-4a6e-b3a1-d9c4f66cb476">
</p>

![Static Badge](https://img.shields.io/badge/Nuke-12.1%20%7C%2012.2%20%7C%2013.0%20%7C%2013.1%20%7C%2013.2%20%7C%2014.0%20%7C%2014.1%20%7C%2015.0%20-brightgreen?style=flat&logo=nuke&logoColor=white&label=nuke) ![Static Badge](https://img.shields.io/badge/supported-brightgreen?style=flat&logo=github&logoColor=white&label=windows) ![Static Badge](https://img.shields.io/badge/supported-brightgreen?style=flat&logo=github&logoColor=white&label=linux)

**DespillAP** is a native CPU-based plugin for Nuke, currently available for Windows and Linux. It is built using the original algorithms developed by Adrian Pueyo for the **apDespill** gizmo. You can explore more of his tools at the following [link](https://adrianpueyo.com/es/gizmos/).

**DespillAP** allows you to remove the spill of a specific color from the input image, set a limit to control the amount of despill applied, inject the overall color to perform a proper despill, and apply a respill based on the desired background.

<p align="center">
  <img width="400" alt="Image" src="https://github.com/user-attachments/assets/b811a4d2-921f-4007-8699-f988f7cbb513">
</p>

## Features

- Select a default color (`Red`, `Green`, `Blue`, or `Pick`), or choose a custom color from the image to remove the spill from the input.
- `Absolute Mode`: normalizes the spill relative to the intensity of the selected color.
- Despill algorithms include `Average`, `Max`, `Min`, and `Custom`.

