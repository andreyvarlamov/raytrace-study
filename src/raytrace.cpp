#include "raytrace.h"

#include "va_types.h"
#include "va_util.h"
#include "va_colors.h"

#include <stdio.h>
#include <stdlib.h>

func image_u32
AllocateImage(int Width, int Height)
{
    image_u32 Result;
    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = (u32 *) malloc(Result.Width * Result.Height * sizeof(u32));
    return Result;
}

func b32
SaveBMP(image_u32 *Image, const char *FileName)
{
    size_t ImageSize = Image->Width * Image->Height * sizeof(*Image->Pixels);
    
    bitmap_header BitmapHeader = {};
    BitmapHeader.FileType = 0x4D42;
    BitmapHeader.FileSize = (u32) (sizeof(bitmap_header) + ImageSize);
    BitmapHeader.Reserved1 = 0;
    BitmapHeader.Reserved2 = 0;
    BitmapHeader.BitmapOffset = (u32) sizeof(bitmap_header);
    BitmapHeader.Size = 40;
    BitmapHeader.Width = Image->Width;
    BitmapHeader.Height = Image->Height;
    BitmapHeader.Planes = 1;
    BitmapHeader.BitsPerPixel = 32;
    BitmapHeader.Compression = 0;
    BitmapHeader.SizeOfBitmap = 0;
    BitmapHeader.HorizontalResolution = 0;
    BitmapHeader.VerticalResolution = 0;
    BitmapHeader.ColorsUsed = 0;
    BitmapHeader.ColorsImportant = 0;

    b32 Success = false;
    FILE *File = fopen(FileName, "wb");
    if (File)
    {
        if (fwrite(&BitmapHeader, sizeof(BitmapHeader), 1, File) == 1)
        {
            if (fwrite(Image->Pixels, ImageSize, 1, File) == 1)
            {
                Success = true;
            }
        }

        fclose(File);
    }

    if (Success)
    {
        return true;
    }
    else
    {
        printf("Failed to write image data.\n");
        return false;
    }
}

func vec3
RayCast(world *World, vec3 RayOrigin, vec3 RayDirection)
{
    vec3 FinalColor = Vec3(1.0f, 0.0f, 0.0f);
    return FinalColor;
}

int main(int argc, char **argv)
{
    world World = {};

    material Materials[] = {
        Vec3(1.0f, 1.0f, 1.0f),
        Vec3(0.5f, 0.5f, 0.5f),
        Vec3(1.0f, 0.0f, 0.0f),
    };

    plane Planes[] = {
        {Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1},
    };

    sphere Spheres[] = {
        {Vec3(0.0f, 10.0f, 0.0f), 2.0f, 2},
    };

    World.MaterialCount = ArrayCount(Materials);
    World.Materials = Materials;
    World.PlaneCount = ArrayCount(Planes);
    World.Planes = Planes;
    World.SphereCount = ArrayCount(Spheres);
    World.Spheres = Spheres;

    vec3 CameraP = Vec3(0, 1, 10);
    vec3 CameraZ = VecNormalize(CameraP); // CameraP - Vec3(0, 0, 0)
    vec3 CameraX = VecNormalize(VecCross(Vec3(0, 1, 0), CameraZ)); // World Up X Camera Z
    vec3 CameraY = VecNormalize(VecCross(CameraZ, CameraX));

    f32 FilmDist = 1.0f;
    f32 FilmW = 1.0f;
    f32 FilmH = 1.0f;
    f32 HalfFilmW = 0.5f*FilmW;
    f32 HalfFilmH = 0.5f*FilmH;
    
    vec3 FilmCenter = CameraP - FilmDist*CameraZ;
    
    image_u32 Image = AllocateImage(1280, 720);

    u32 *Pixel = Image.Pixels;
    for (int Y = 0; Y < Image.Height; Y++)
    {
        f32 FilmY = (f32)Y / (f32)Image.Height * 2.0f - 1.0f;
        for (int X = 0; X < Image.Width; X++)
        {
            f32 FilmX = (f32)X / (f32)Image.Width * 2.0f - 1.0f;

            vec3 FilmP = FilmCenter + FilmX*HalfFilmW*CameraX + FilmY*HalfFilmH*CameraY;

            vec3 RayOrigin = CameraP;
            vec3 RayDirection = VecNormalize(FilmP - CameraP);

            vec3 Color = RayCast(&World, RayOrigin, RayDirection);

            u32 ColorBGRA = PackBGRA(Vec4(Color, 1.0f));
            
            *Pixel++ = ColorBGRA;
        }
    }

    SaveBMP(&Image, "temp/raytest.bmp");
    
    return 0;
}
