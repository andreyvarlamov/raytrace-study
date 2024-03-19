#ifndef RAYTRACE_H
#define RAYTRACE_H

#include "va_types.h"
#include "va_linmath.h"

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    i32 HorizontalResolution;
    i32 VerticalResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};
#pragma pack(pop)

struct image_u32
{
    u32 *Pixels;
    int Width;
    int Height;
};

struct plane
{
    vec3 N;
    f32 D;
    int MatIndex;
};

struct sphere
{
    vec3 P;
    f32 R;
    int MatIndex;
};

struct material
{
    vec3 EmitColor;
    vec3 ReflectColor;
};

struct world
{
    int MaterialCount;
    material *Materials;

    int PlaneCount;
    plane *Planes;

    int SphereCount;
    sphere *Spheres;
};

#endif
