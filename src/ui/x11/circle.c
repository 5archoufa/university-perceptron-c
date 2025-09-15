#include "ui/x11/circle.h"
#include <math.h>
#include "ui/x11/pixel.h"
#include <stdlib.h>
#include <stdio.h>

Shape* CreateCircle(V3 center, float radius){
    Circle* circle = malloc(sizeof(Circle));
    circle->center = center;
    circle->r = radius;
    Shape* shape = malloc(sizeof(Shape));
    shape->self = circle;
    shape->Draw = DrawCircle;
    return shape;
}

Pixel *DrawCircle(void* self)
{
    Circle* circle = (Circle*)self;
    V3 *c = &circle->center;
    Pixel *head = malloc(sizeof(Pixel));
    Pixel *tail = NULL;
    int pixelCount = 0;
    for (float a = 0.0; a <= 90.0; a += 0.2)
    {
        pixelCount+=4;
        if (tail != NULL)
        {
            tail->next = malloc(sizeof(Pixel));
            tail = tail->next;
        }
        else
        {
            tail = head;
        }

        float x = sin(a * M_PI / 180.0) * circle->r;
        float y = cos(a * M_PI / 180.0) * circle->r;
        V3 ur = {c->x + x, c->y + y, c->z};
        V3 ul = {c->x - x, ur.y, c->z};
        V3 br = {ur.x, c->y - y, c->z};
        V3 bl = {ul.x, br.y, c->z};

        tail->x = ur.x;
        tail->y = ur.y;
        tail->next = malloc(sizeof(Pixel));
        tail = tail->next;
        tail->x = ul.x;
        tail->y = ul.y;
        tail->next = malloc(sizeof(Pixel));
        tail = tail->next;
        tail->x = bl.x;
        tail->y = bl.y;
        tail->next = malloc(sizeof(Pixel));
        tail = tail->next;
        tail->x = br.x;
        tail->y = br.y;
        tail->next = NULL;
    }
    printf("Drew %d pixels\n", pixelCount);
    return head;
}