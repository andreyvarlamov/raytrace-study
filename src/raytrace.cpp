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
    f32 MinHitDistance = 0.001f;
    f32 Tolerance = 0.0001f;

    vec3 FinalColor = {};
    vec3 Attenuation = Vec3(1, 1, 1);
    for (int BounceI = 0; BounceI < 8; BounceI++)
    {
        f32 HitDistance = FLT_MAX;

        b32 HitSomething = false;
        int HitMaterial = 0;
        vec3 NextOrigin = {};
        vec3 NextNormal = {};

        plane *Plane = World->Planes;
        for (int PlaneI = 0; PlaneI < World->PlaneCount; PlaneI++, Plane++)
        {
            f32 Denom = VecDot(Plane->N, RayDirection);
            if (Denom < -Tolerance || Denom > Tolerance)
            {
                f32 t = (-Plane->D - VecDot(Plane->N, RayOrigin)) / Denom;
                if (t > MinHitDistance && t < HitDistance)
                {
                    HitDistance = t;
                    HitMaterial = Plane->MatIndex;
                    HitSomething = true;

                    NextOrigin = RayOrigin + t*RayDirection;
                    NextNormal = Plane->N;
                }
            }
        }

        sphere *Sphere = World->Spheres;
        for (int SphereI = 0; SphereI < World->SphereCount; SphereI++, Sphere++)
        {
            vec3 SphereRelativeRayOrigin = RayOrigin - Sphere->P;
            f32 a = VecDot(RayDirection, RayDirection);
            f32 b = 2.0f*VecDot(RayDirection, SphereRelativeRayOrigin);
            f32 c = VecDot(SphereRelativeRayOrigin, SphereRelativeRayOrigin) - Sphere->R * Sphere->R;

            f32 Denom = 2.0f*a; // NOTE: Cannot be 0
            f32 RootTerm = SqrtF(b*b - 4.0f*a*c);
            if (RootTerm > Tolerance)
            {
                f32 tp = (-b + RootTerm) / Denom;
                f32 tn = (-b - RootTerm) / Denom;

                f32 t = tp;
                if (tn > MinHitDistance && tn < tp)
                {
                    t = tn;
                }

                if (t > MinHitDistance && t < HitDistance)
                {
                    HitDistance = t;
                    HitMaterial = Sphere->MatIndex;
                    HitSomething = true;

                    NextOrigin = RayOrigin + t*RayDirection;
                    NextNormal = VecNormalize(NextOrigin - Sphere->P);
                }
            }
        }

        if (HitSomething)
        {
            material Mat = World->Materials[HitMaterial];

            FinalColor += Attenuation * Mat.EmitColor;
            Attenuation = Attenuation * Mat.ReflectColor;

            RayOrigin = NextOrigin;
            RayDirection = NextNormal;
        }
        else
        {
            // NOTE: Didn't hit anything, emit color of null material, and break out of the bounce loop
            material Mat = World->Materials[HitMaterial];
            FinalColor += Attenuation * Mat.EmitColor;
            break;
        }
    }
    
    return FinalColor;
}

int main(int argc, char **argv)
{
    printf("Raycasting... \n");
    
    world World = {};

    material Materials[] = {
        {Vec3(0.3f, 0.4f, 0.5f), Vec3()},
        {Vec3(), Vec3(0.5f, 0.5f, 0.5f)},
        {Vec3(), Vec3(0.7f, 0.5f, 0.3f)},
    };

    plane Planes[] = {
        {Vec3(0.0f, 1.0f, 0.0f), 0.0f, 1},
    };

    sphere Spheres[] = {
        {Vec3(0.0f, 0.0f, 0.0f), 1.0f, 2},
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

    image_u32 Image = AllocateImage(1280, 720);

    f32 FilmDist = 1.0f;
    f32 FilmW = 1.0f;
    f32 FilmH = 1.0f;
    if (Image.Width > Image.Height)
    {
        FilmH = FilmW * ((f32)Image.Height / (f32)Image.Width);
    }
    else
    {
        FilmW = FilmH * ((f32)Image.Width / (f32)Image.Height);
    }
    f32 HalfFilmW = 0.5f*FilmW;
    f32 HalfFilmH = 0.5f*FilmH;
    
    vec3 FilmCenter = CameraP - FilmDist*CameraZ;
    
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

        if (Y % 64 == 0)
        {
            printf("\rRaycasting %d%%...        ", 100*Y / Image.Height);
            fflush(stdout);
        }
    }

    SaveBMP(&Image, "temp/raytest.bmp");

    printf("\nDone.\n");
    
    return 0;
}
