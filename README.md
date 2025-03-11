# Introduction 
This repository holds scripts intended for use in generating a simulation of a Bayer pattern image (RGGB format) as well as for interpolating the resulting image. The "demosaicing" (interpolation) script makes use of convolution kernels so that arbitrary interpolation algorithms can be implemented more easily. 

The expected inputs are *.png files and the resulting outputs are *.bmp files. 


## Quickstart
``` ./run_scripts.sh inputs/kodim19.png```

> [!NOTE]  
> This script makes use of `xdg-open`, so if you don't have this dependency then please view the output files (`output/bayer.bmp` and `output/demosaic.bmp`) manually.

> [!NOTE]  
> This script requires `ImageMagick` 


## Compiling
There is a `Makefile`. Run `make` to compile everything, and `make clean` to clean up.
> [!NOTE]  
> `./run_script [input_file.png]` will perform compilation.

## Example
#### Input Image
![Lighthouse!](/inputs/kodim19.png "Lighthouse")

#### Bayer Pattern Image
![Bayer Pattern Image!](/output/bayer.bmp "Bayer Pattern Image (RGGB)")

#### Interpolated Image
![Interpolated Image!](/output/demosaic.bmp "Interpolated Image")
