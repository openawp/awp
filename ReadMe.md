# AWP source code
[license](LICENSE) 

This source code implements the AWP-CH algorithm for parallel computing exact geodesic distances on manifold triangle meshes without boundaries.'

## Usage:
-m [mesh file]: input model file

-s [source index]: index of source

-o [output mesh file with geodesic distances information]: (only support .obj files)

## Example: 
```
AWP.exe -m bunny_nf144k.m -s 13340 -o bunny_info.obj
```
## More
As research code, it does not handle skinny or degenerate triangles. The code has been tested on the following GPUs

- GTX 970 with 1664 CUDA cores and 2.44 Tflops;
- GTX Titan X (Maxwell) with 3072 CUDA cores and 7.0 T358 flops; and
- GTX Titan Xp with 3840 CUDA cores and 12 Tflops.

If using the code, please cite the paper
```
Ying et al., Parallelizing Discrete Geodesic Algorithms with Perfect Efficiency, Computer-Aided Design, 2019.
```
