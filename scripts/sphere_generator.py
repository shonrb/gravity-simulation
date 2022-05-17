#!/usr/bin/env python3
''' sphere_generator.py
    Generate vertices and surface normals for a sphere
    with a specified level of detail (cube sphere).
    Outputs values separated by commas which can be embedded
    in a c++ source file.
'''
from math import sqrt
from os import path, pardir

LEVEL_OF_DETAIL = 8
TARGET          = "/resources/spheremesh.txt"
DIR             = path.dirname(__file__)
OUTPUT_FILE     = path.abspath(path.join(DIR, pardir)) + TARGET

def normalise(vec):
    x, y, z = vec
    mag = sqrt(x*x + y*y + z*z)
    return x/mag, y/mag, z/mag

def sub(a, b):
    return tuple(
        ac - bc
        for ac, bc in zip(a, b)
    )

# Create a n*n grid of points where n is the level of detail
grid_vertices = [
    [
        ((x * 2) / LEVEL_OF_DETAIL - 1, 
         (y * 2) / LEVEL_OF_DETAIL - 1)
        for x in range(LEVEL_OF_DETAIL+1)
    ]
    for y in range(LEVEL_OF_DETAIL+1)
]

# Triangulate the grid into a mesh
triangulated = sum([
    [
        grid_vertices[x  ][y  ],
        grid_vertices[x  ][y+1],
        grid_vertices[x+1][y+1],
        grid_vertices[x+1][y+1],
        grid_vertices[x+1][y  ],
        grid_vertices[x  ][y  ],
    ]
    for x in range(LEVEL_OF_DETAIL)
    for y in range(LEVEL_OF_DETAIL)
], [])

# Join 6 copies of the mesh to form a cube
cube_mesh = [
    *[(-1.0, y,   z  ) for y, z in triangulated[::-1]],
    *[( 1.0, y,   z  ) for y, z in triangulated],
    *[( x,  -1.0, z  ) for x, z in triangulated],
    *[( x,   1.0, z  ) for x, z in triangulated[::-1]],
    *[( x,   y,  -1.0) for x, y in triangulated[::-1]],
    *[( x,   y,   1.0) for x, y in triangulated]
]

# Normalise each vertex in the cube mesh, then flatten into a 
# 1D array.
sphere_mesh_vertices = sum(
    [list(normalise(i)) for i in cube_mesh],
    []
)

as_str = [str(i) + "f" for i in sphere_mesh_vertices]
new_contents = ",\n".join(as_str)
open(OUTPUT_FILE, "w").write(new_contents)