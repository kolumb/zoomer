#include "./zoomer_navigation.h"

Vector world (const Camera camera, const Vector pos) {
    Vector result = { pos.x / camera.scale, pos.y / camera.scale };
    return result;
}

void camera_update(Camera *camera, const Config config, const float dt, const Mouse mouse, const Vector window_size) {
    if (fabs(camera->delta_scale) > 0.5) {
        Vector p0 = VectorScale(VectorSubtract(camera->scale_pivot, VectorScale(window_size, 0.5f)), 1 / camera->scale);
        camera->scale = fmaxf(camera->scale + camera->delta_scale * dt, config.min_scale);
        Vector p1 = VectorScale(VectorSubtract(camera->scale_pivot, VectorScale(window_size, 0.5f)), 1 / camera->scale);
        VectorAddMut(&camera->position, VectorSubtract(p0, p1));

        camera->delta_scale -= camera->delta_scale * dt * config.scale_friction;
    }

    if (!mouse.drag && (VectorLength(camera->velocity) > VELOCITY_THRESHOLD)) {
        VectorAddMut(&camera->position, VectorScale(camera->velocity, dt));
        VectorSubtractMut(&camera->velocity, VectorScale(camera->velocity, dt * config.drag_friction));
    }
}
