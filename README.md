# Frozlunky

Seeds, Online Multiplayer, Custom Levels, and more for Spelunky HD!

## Installation

Download `Frozlunky.exe` from https://github.com/sashavol/Frozlunky/releases/latest. It can be placed anywhere and launched once Spelunky is running.

Full Documentation is available at [https://sashavol.github.io/Frozlunky/](https://sashavol.github.io/Frozlunky/).

## Development

Before you're able to build Frozlunky you'll need the following prerequisites:

* [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) with `Desktop development with C++` workload installed.
* [vcpkg](https://github.com/microsoft/vcpkg#getting-started). Make sure to run `vcpkg integrate install` so it works with Visual Studio

Once these are setup you can clone this repo and open the `Frozlunky.sln` solution.

There are `Debug` and `Release` configurations. The `Debug` configuration is best for development as it provides console output which is helpful for debugging. The `Release` configuration is more performant and provides a static executable that can be used for distribution.
