#ifndef RENDER_DATA_H
#define RENDER_DATA_H

#include <X11/Xlib.h>

typedef struct RenderData RenderData;

typedef struct
{
    unsigned long color;
    int count;
} ColoredPixels;

struct RenderData
{
    int coloredPixelsCount;
    ColoredPixels **coloredPixels;
    int pointCount;
    XPoint **points;
};

void RenderData_Free(RenderData *renderData)
{
    for (int i = 0; i < renderData->coloredPixelsCount; i++)
    {
        ColoredPixels_Free(renderData->coloredPixels[i]);
    }
    free(renderData->coloredPixels);
    free(renderData->points);
    free(renderData);
}

void RenderData_Setup(RenderData *renderData, int coloredPixelsCount, ColoredPixels** coloredPixels, int pointCount, XPoint** points)
{
    renderData->coloredPixelsCount = coloredPixelsCount;
    renderData->coloredPixels = coloredPixels;
    renderData->pointCount = pointCount;
    renderData->points = points;
}

ColoredPixels *ColoredPixels_Create(unsigned long color, int count)
{
    ColoredPixels *coloredPixels = malloc(sizeof(ColoredPixels));
    coloredPixels->color = color;
    coloredPixels->count = count;
    return coloredPixels;
}

static void ColoredPixels_Free(ColoredPixels *coloredPixels)
{
    free(coloredPixels);
}

#endif