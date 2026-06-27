# Phase 1 – Zephyr Fundamentals

> "Before writing production firmware, it is important to understand the ecosystem that makes Zephyr different from traditional embedded development."

---

# Why should I care?

If you're developing professional embedded software, understanding Zephyr fundamentals will help you:

- Understand how Zephyr projects are organized.
- Navigate the build system with confidence.
- Learn the terminology used throughout the documentation.
- Build a solid foundation before learning Devicetree and Kconfig.
- Understand why Zephyr scales from hobby projects to commercial products.

---

# Motivation

Coming from STM32Cube, FreeRTOS, or bare-metal development, Zephyr can initially feel overwhelming because it introduces many new concepts at once:

- west
- CMake
- Kconfig
- Devicetree
- Sysbuild
- Modules
- Workspaces

This chapter explains the big picture before diving into the details.

---

# What is Zephyr?

> *To be completed.*

---

# What is west?

`west` is Zephyr's command-line tool for managing a Zephyr workspace.

In practice, it does three important jobs:

- It downloads Zephyr and Zephyr modules using a manifest file.
- It keeps all repositories in the workspace on the versions expected by the project.
- It provides common commands such as `west update`, `west build`, `west flash`, and `west debug`.

It is important to separate `west` from the Zephyr SDK.

`west` manages source repositories and runs Zephyr-aware commands. The Zephyr SDK provides compilers, linkers, and host tools. A computer can have one SDK installation shared by multiple projects, while each serious project can still have its own west workspace and pinned Zephyr version.

This is different from a simple package manager. A Zephyr application often depends on multiple Git repositories: Zephyr itself, HALs, MCUboot, CMSIS, networking libraries, debug tools, and sometimes vendor modules. `west` gives the project one place to describe that complete dependency graph.

The file that describes that graph is called the **west manifest**. In this project, the manifest is:

```text
west.yml
```

That file is intentionally committed to Git because it defines the Zephyr version and module set used by this application.

---

# What is a Zephyr Workspace?

A Zephyr workspace is the local directory where `west` places:

- west metadata
- the Zephyr repository
- Zephyr modules
- the application repository

For this project, the intended workspace layout is:

```text
zephyr-production-reference-workspace/
├── .west/
├── zephyr/
├── modules/
└── zephyr-production-reference/
    ├── west.yml
    ├── README.md
    ├── docs/
    └── src/
```

The important detail is that the workspace root is **not** the Git repository.

This Git repository is only the application:

```text
zephyr-production-reference/
├── west.yml
├── README.md
├── docs/
└── src/
```

Everything outside the application directory is created or managed by `west` and should not be committed to this repository.

---

# How this project uses west

This project uses the Zephyr **T2 topology**, where the application repository is also the manifest repository.

That means:

- `zephyr-production-reference` contains `west.yml`.
- `west.yml` tells west which Zephyr revision to use.
- Zephyr and its modules are downloaded next to the application, not inside it.
- The project controls its own Zephyr dependency instead of relying on a random global Zephyr checkout.

The minimal manifest currently looks like this:

```yaml
manifest:
  remotes:
    - name: zephyrproject-rtos
      url-base: https://github.com/zephyrproject-rtos

  projects:
    - name: zephyr
      remote: zephyrproject-rtos
      revision: v4.4.0
      import: true

  self:
    path: zephyr-production-reference
```

The key line is:

```yaml
revision: v4.4.0
```

This pins the Zephyr version. For a production reference project, pinning is better than using `main` because it makes builds reproducible.

The other key line is:

```yaml
import: true
```

Zephyr's own manifest lists the modules Zephyr needs. By importing it, this project does not need to manually list every Zephyr module itself.

---

# First-time setup flow

From a clean machine, the intended setup flow is:

```sh
mkdir -p ~/zephyr-workspaces/zephyr-production-reference-workspace
cd ~/zephyr-workspaces/zephyr-production-reference-workspace
git clone https://github.com/Threephoneoff/zephyr-production-reference.git
west init -l zephyr-production-reference
west update
```

After this, the workspace should look like:

```text
~/zephyr-workspaces/
└── zephyr-production-reference-workspace/
    ├── .west/
    ├── zephyr/
    ├── modules/
    └── zephyr-production-reference/
```

If the application repo is already cloned, run `west init -l` from the workspace root and point it at the application directory.

Example:

```sh
cd ~/zephyr-workspaces/zephyr-production-reference-workspace
west init -l zephyr-production-reference
west update
```

Do not run `git init` in the workspace root. The workspace root is just a local container for west.

---

# `west init -l` vs `west init -m`

Both commands initialize a west workspace. The difference is where the manifest repository comes from.

Use `west init -l` when the application repository is already cloned locally:

```sh
west init -l zephyr-production-reference
```

In this project, `-l` means:

```text
Use the west.yml inside the existing zephyr-production-reference directory.
Do not clone the application repository again.
```

This is the command to use while developing this project locally.

Use `west init -m` when west should clone the manifest repository for you:

```sh
west init -m https://github.com/Threephoneoff/zephyr-production-reference
```

In this case, `-m` means:

```text
Clone this Git repository first.
Then use its west.yml to initialize the workspace.
```

The practical rule is:

```text
-l = I already have the application repo on disk.
-m = west should clone the application repo for me.
```

For this project right now, use `-l` because the repository already exists locally.

---

# Where Zephyr west commands come from

Not every `west` command comes from west itself.

Some commands are built into west:

```sh
west init
west update
west list
west topdir
```

Other commands are provided by Zephyr as **west extension commands**:

```sh
west build
west flash
west debug
west boards
west zephyr-export
```

The practical difference is:

```text
west core commands = available because west is installed
Zephyr extension commands = available because the workspace contains Zephyr
```

After `west update`, the workspace contains the `zephyr/` repository. Inside that repository, Zephyr declares its west extension commands in:

```text
zephyr/scripts/west-commands.yml
```

That file maps command names to Python implementations. For example, the `zephyr-export` command is declared like this:

```yaml
- file: scripts/west_commands/export.py
  commands:
    - name: zephyr-export
      class: ZephyrExport
      help: export Zephyr installation as a CMake config package
```

This means that when we run:

```sh
west zephyr-export
```

west loads the `ZephyrExport` class from:

```text
zephyr/scripts/west_commands/export.py
```

This is why `west zephyr-export` may feel unusual at first. It is not a generic west command. It is a Zephyr-provided command that west discovers from the downloaded Zephyr repository.

---

# Why run `west zephyr-export`?

After downloading Zephyr with `west update`, run:

```sh
west zephyr-export
```

This registers the current Zephyr checkout as a CMake package in the user CMake package registry.

On Linux and macOS, that registry is located under:

```text
~/.cmake/packages/
```

The practical purpose is to make this Zephyr installation discoverable by CMake. This helps Zephyr applications find the correct Zephyr base when configuring a build.

The important mental model is:

```text
west update
   downloads Zephyr and modules

west zephyr-export
   tells CMake where this Zephyr installation is
```

You normally run `west zephyr-export` once after setting up or changing the Zephyr checkout. It is not something you need to run before every build.

---

# Python packages and the Zephyr SDK

After `west update` and `west zephyr-export`, the next setup step is installing Python packages used by Zephyr scripts and modules.

The current recommended command is:

```sh
west packages pip --install
```

This is better than manually installing only:

```sh
pip install -r zephyr/scripts/requirements.txt
```

because `west packages pip --install` asks the current workspace which Python requirement files are needed by Zephyr and its modules.

In practice:

```text
pip install -r zephyr/scripts/requirements.txt
   installs Python requirements for Zephyr itself

west packages pip --install
   installs Python requirements for Zephyr and modules in this workspace
```

This command expects a Python virtual environment. A practical setup is:

```sh
cd /Users/denislavtrifonov/work/projects/zephyr-data-logger
python3 -m venv .venv
source .venv/bin/activate
west packages pip --install
```

The virtual environment belongs to the local workspace, not the application repository. Do not commit `.venv/`.

After Python packages, install the Zephyr SDK:

```sh
west sdk install --gnu-toolchains arm-zephyr-eabi
```

The Zephyr SDK provides the cross-compiler and host tools used to build firmware. For this STM32H7 project, the important GNU toolchain is:

```text
arm-zephyr-eabi
```

Without `--gnu-toolchains arm-zephyr-eabi`, the SDK installer may install toolchains for many architectures. That is useful for broad Zephyr development, but unnecessary for this STM32-only project.

The SDK version is selected from:

```text
zephyr/SDK_VERSION
```

This keeps the SDK aligned with the Zephyr version downloaded by `west update`.

If the SDK is installed in the default location, Zephyr usually finds it automatically during `west build`. If it is installed in a custom location, make the path explicit:

```sh
export ZEPHYR_SDK_INSTALL_DIR=/path/to/zephyr-sdk-<version>
```

The useful mental model is:

```text
west packages pip --install
   prepares Python tools used by Zephyr

west sdk install
   installs compilers and build tools used to produce firmware
```

---

# Minimal application structure

After the workspace and toolchain are ready, the application repository needs the minimum files Zephyr expects:

```text
zephyr-production-reference/
├── CMakeLists.txt
├── prj.conf
└── src/
    └── main.c
```

`CMakeLists.txt` connects the application to Zephyr's build system:

```cmake
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_production_reference)

target_sources(app PRIVATE src/main.c)
```

`prj.conf` is the application Kconfig configuration file. It can be empty for the first build. Later phases will enable logging, shell, settings, storage, and drivers here.

`src/main.c` is the first application entry point:

```c
#include <zephyr/kernel.h>

int main(void)
{
	printk("Zephyr Production Reference is running on %s\n", CONFIG_BOARD);
	return 0;
}
```

This first application intentionally does almost nothing. The goal is to prove that the workspace, dependencies, SDK, CMake, and board support are wired together correctly before adding product logic.

---

# How `west build` works

`west build` is a Zephyr-aware wrapper around CMake and the underlying build tool, usually Ninja.

For this project, the first build command is:

```sh
west build -b stm32h7b3i_dk zephyr-production-reference
```

This means:

```text
Build the application in zephyr-production-reference
for the stm32h7b3i_dk board
using the current west workspace.
```

The command does several things for us:

1. Finds the west workspace root by locating `.west/`.
2. Finds the Zephyr repository in the workspace.
3. Finds the application source directory.
4. Creates or reuses a build directory.
5. Runs CMake when configuration is needed.
6. Lets Zephyr process Kconfig and Devicetree.
7. Invokes the build tool to compile and link the firmware.

The board is selected with:

```sh
-b stm32h7b3i_dk
```

That board name connects the build to Zephyr's board definition for the STM32H7B3I Discovery kit. The board definition provides information such as SoC, CPU, memory layout, default Devicetree, default Kconfig options, and flash/debug runner metadata.

The application source directory is this argument:

```sh
zephyr-production-reference
```

If running from the workspace root, use:

```sh
west build -b stm32h7b3i_dk zephyr-production-reference
```

If running from inside the application repository, use:

```sh
west build -b stm32h7b3i_dk .
```

By default, `west build` creates a build directory named:

```text
build/
```

For cleaner production work, it is often useful to give each board its own build directory:

```sh
west build -b stm32h7b3i_dk zephyr-production-reference -d build/stm32h7b3i_dk
```

The build output contains files such as:

```text
build/
├── CMakeCache.txt
└── zephyr/
    ├── zephyr.elf
    ├── zephyr.hex
    ├── zephyr.bin
    └── zephyr.map
```

Useful artifacts:

- `zephyr.elf`: debug ELF used by debuggers.
- `zephyr.hex` / `zephyr.bin`: firmware image formats used for flashing.
- `zephyr.map`: linker map useful for memory analysis.
- `CMakeCache.txt`: records important build configuration decisions.

When debugging toolchain discovery, inspect:

```sh
grep -E "ZEPHYR_SDK_INSTALL_DIR|CMAKE_C_COMPILER|ZEPHYR_TOOLCHAIN_VARIANT" build/CMakeCache.txt
```

Use a pristine build when changing important build configuration:

```sh
west build -p always -b stm32h7b3i_dk zephyr-production-reference
```

The `-p always` option removes stale build state before configuring again. This is useful when switching boards, changing toolchains, changing Devicetree overlays, or when CMake/Kconfig state looks suspicious.

The practical mental model is:

```text
west build
   is the Zephyr-aware frontend

CMake
   configures the build

Kconfig
   selects software configuration

Devicetree
   describes hardware

Ninja
   compiles and links the firmware

Zephyr SDK
   provides the compiler and host tools
```

---

# Practical west tips

`west.yml` is part of the product.

If the manifest changes, the dependency graph changes. Treat manifest edits with the same care as source code changes.

Use pinned revisions for real work.

Using `main` is convenient for experimenting, but it can break unexpectedly. Pinning to a Zephyr release or commit makes the project easier to reproduce on another computer or in CI.

Run `west update` after changing `west.yml`.

Editing the manifest does not automatically update downloaded repositories. `west update` applies the manifest to the workspace.

Know where you are.

Many confusing west errors happen because commands are run from the wrong directory. `west topdir` prints the workspace root:

```sh
west topdir
```

Use `west list` to see what west manages.

```sh
west list
```

This helps you understand which repositories came from the manifest and where they live on disk.

Use `west help` when a command looks mysterious.

```sh
west help
west help zephyr-export
west help build
```

If a command is Zephyr-specific, its implementation usually lives under:

```text
zephyr/scripts/west_commands/
```

Keep application code in the application repo.

Avoid editing Zephyr directly unless you are intentionally preparing an upstream Zephyr contribution or a downstream patch. For normal application development, keep code in:

```text
zephyr-production-reference/
```

Build output is disposable.

Build directories should not be committed. They can always be regenerated with `west build`.

---

# Shared workspace vs project workspace

It is possible to keep one global Zephyr workspace and put many applications inside it. That is fine for early experiments.

For this project, the better habit is to let the project own its Zephyr version through `west.yml`.

The practical tradeoff:

| Approach | Good for | Problem |
|----------|----------|---------|
| One shared Zephyr checkout | Quick experiments | Projects silently depend on the same Zephyr version |
| One manifest per project | Reproducible projects | Uses more disk space |

For production firmware, reproducibility matters more than saving disk space.

---

# Key Concepts

- Application
- Board
- Module
- Workspace
- Manifest
- west
- Manifest repository
- Project repository
- CMake
- Build directory
- Toolchain
- Devicetree
- Kconfig

---

# Build Flow

For this project, the high-level flow is:

```text
west.yml
   ↓
west update
   ↓
Zephyr + modules are downloaded
   ↓
west zephyr-export
   ↓
west packages pip --install
   ↓
west sdk install
   ↓
west build
   ↓
workspace topdir is found
   ↓
application source directory is selected
   ↓
board definition is selected
   ↓
CMake configures the application
   ↓
Kconfig selects software configuration
   ↓
Devicetree describes hardware
   ↓
Firmware image is produced
```

This phase focuses on understanding the workspace and `west`. CMake, Kconfig, and Devicetree will be explored more deeply in later phases.

---

# Practical Example

After the workspace is initialized, a typical build command will look like:

```sh
west build -b <board-name> zephyr-production-reference
```

For this project, the target board is the STM32H7B3I Discovery kit:

```sh
west build -b stm32h7b3i_dk zephyr-production-reference
```

Run this from the workspace root:

```sh
cd /Users/denislavtrifonov/work/projects/zephyr-data-logger
west build -b stm32h7b3i_dk zephyr-production-reference
```

---

# Things I Learned

- [ ] A west workspace contains the application, Zephyr, modules, and `.west/` metadata.
- [ ] The application repository owns the manifest in the T2 topology.
- [ ] Zephyr-specific west commands come from `zephyr/scripts/west-commands.yml`.
- [ ] `west packages pip --install` installs Python requirements for Zephyr and modules.
- [ ] `west sdk install` installs the compiler/toolchain side of the setup.
- [ ] A minimal Zephyr application needs `CMakeLists.txt`, `prj.conf`, and `src/main.c`.
- [ ] `west build` is a wrapper around CMake and Ninja.
- [ ] The `-b` option selects the board.
- [ ] The source directory argument selects the application.
- [ ] The build directory contains generated files and firmware artifacts.

---

# Common Mistakes

- [ ] Treating the workspace root as the application Git repository.
- [ ] Editing files inside `zephyr/` when the change belongs in the application.
- [ ] Using Zephyr `main` for a project that should be reproducible.
- [ ] Forgetting to run `west update` after changing `west.yml`.
- [ ] Assuming every `west` command is built into west itself.
- [ ] Forgetting that Zephyr-specific west commands come from the downloaded `zephyr/` repository.
- [ ] Installing Python packages globally instead of using a workspace virtual environment.
- [ ] Confusing Python package setup with SDK/toolchain setup.
- [ ] Installing every SDK toolchain when only `arm-zephyr-eabi` is needed for STM32.
- [ ] Forgetting that a custom SDK install path may need `ZEPHYR_SDK_INSTALL_DIR`.
- [ ] Trying to build before adding `CMakeLists.txt`, `prj.conf`, and `src/main.c`.
- [ ] Reusing a stale build directory after changing boards or important configuration.
- [ ] Committing `build/` output.
- [ ] Running west commands from a directory that is not inside a west workspace.
- [ ] Committing build directories or downloaded Zephyr modules.

---

# Production Perspective

In a production firmware project, the Zephyr version is part of the product definition.

If two engineers build the same commit, they should get the same source dependencies. If CI builds the project six months later, it should use the same Zephyr revision unless the project intentionally upgrades it.

That is why this project uses an application-owned `west.yml`. The repository does not just contain application code. It also records the Zephyr baseline that the application expects.

---

# References

- Zephyr west documentation: https://docs.zephyrproject.org/latest/develop/west/index.html
- Zephyr west workspaces and topologies: https://docs.zephyrproject.org/latest/develop/west/workspaces.html
- Zephyr west manifest documentation: https://docs.zephyrproject.org/latest/develop/west/manifest.html
- Zephyr getting started guide: https://docs.zephyrproject.org/latest/develop/getting_started/index.html
- Conference talks
- Blog articles

---

# Before Moving On

At the end of this phase, I should be able to:

- [ ] Explain the overall Zephyr architecture.
- [ ] Describe the purpose of west.
- [ ] Explain why this project uses T2 topology.
- [ ] Explain what belongs in Git and what belongs only in the local west workspace.
- [ ] Initialize a workspace from this project's `west.yml`.
- [ ] Use `west topdir` and `west list` to inspect the workspace.
- [ ] Explain where Zephyr extension commands such as `west build` and `west zephyr-export` come from.
- [ ] Explain why `west zephyr-export` is useful after setup.
- [ ] Explain the difference between Python packages and the Zephyr SDK.
- [ ] Install Python requirements using `west packages pip --install`.
- [ ] Install the SDK toolchain needed for STM32 using `west sdk install --gnu-toolchains arm-zephyr-eabi`.
- [ ] Explain how `ZEPHYR_SDK_INSTALL_DIR` helps when the SDK is installed in a custom location.
- [ ] Explain what `west build -b stm32h7b3i_dk zephyr-production-reference` does.
- [ ] Explain when to use a pristine build with `-p always`.
- [ ] Identify the important files generated under `build/zephyr/`.
- [ ] Understand the relationship between west, CMake, Kconfig, and Devicetree.
- [ ] Create and build a basic Zephyr application.
- [ ] Navigate a Zephyr workspace confidently.

---

# Next Phase

➡️ Phase 2 – Devicetree
