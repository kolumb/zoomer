#ifndef ZOOMER_NAVIGATION_H_
#define ZOOMER_NAVIGATION_H_

#include <stdbool.h>
#include "./zoomer_config.h"
#include "./zoomer_vector.h"

#define VELOCITY_THRESHOLD 15.0f

typedef struct {
    Vector curr;
    Vector prev;
    bool drag;
} Mouse;

typedef struct {
    Vector position;
    Vector velocity;
    float scale;
    float delta_scale;
    Vector scale_pivot;
} Camera;

Vector world (const Camera camera, const Vector pos);
void camera_update(Camera *camera, const Config config, const float dt, const Mouse mouse, const Vector window_size);

#endif // ZOOMER_NAVIGATION_H_
