typedef struct Pixel Pixel;

struct Pixel {
    float x, y;
    Pixel* next;
};


void FreePixels(Pixel* head);