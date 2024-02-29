# wisdom_sensor_net

WISDOM team RPi Pico based sensor network codebase

Maintainers:
- Evan Morse (emm08060@vermontstate.edu OR emorse@tech.dev)
- Amelia Vlahogiannis (agv07030@vermontstate.edu)

## Cloning the Repo
Repo should live at: ~/pico/wisdom_sensor_net  
Because we use submodules in our rep, you need to clone recursively:

`git clone --recursive -j8 git@github.com:e-mo/wisdom_sensor_net.git`

NOTE: You'll need proper SSH access to the repo before this will work properly.  
Contact me (Evan Morse) or ping me on the team Discord to configure access.

## Subprojects
Since our work is really a collection of many projects, our repo is also split out into many seperately buidable
subprojects. There is a template subproject which can be copied to hit the ground running with all the files needed
to build code for the RP2040.

### Creating a New Subproject
(from repo root)
1. `cp -r template subproject_name`
2. Edit "target_name" in this line in wisdom_config.cmake (something more meaningful):
```CMake
set(target "target_name")
```
3. Begin editing src/main.c
4. Pet nearest cat

### Building Subproject
Assuming you have properly configured your wisdom_config.cmake file, the build process is very simple:
1. Navigate to root folder of subproject you wish to build.
2. `make build`
3. (only if using picotool) `make load`

If not using picotool, you'll have to copy the generated .uf2 onto the pico bootloader manually.
