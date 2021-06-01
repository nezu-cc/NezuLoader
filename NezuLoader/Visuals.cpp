#include "pch.h"
#include "Visuals.h"

void DrawInferno(Entity* inferno, ImDrawList* draw) {

    if (!Cfg::c.visuals.flame_esp)
        return;

    static const auto flameCircumference = [] {
        std::array<vec, 10> points;
        for (size_t i = 0; i < points.size(); ++i) {
            const float rad = deg2rad(i * (360.0f / points.size()));
            points[i] = vec(60.0f * cos(rad), 60.0f * sin(rad), 0.0f);
        }
        return points;
    }();

    const auto& origin = inferno->GetAbsOrigin();
    ImVec2 origin2;
    if (!W2s(origin, origin2))
        return;

    Entity* owner = I::ClientEntityList->GetClientEntityFromHandle(inferno->ownerEntity());

    ImColor color = !owner || owner->IsEnemy() ? Cfg::c.visuals.flame_esp_enemy : 
        owner->index != I::Engine->GetLocalPlayer() ? Cfg::c.visuals.flame_esp_team : Cfg::c.visuals.flame_esp_self;

    if (color.Value.w == 0.f) //fully transparent
        return;

    std::vector<vec> points;
    points.reserve(inferno->fireCount() * flameCircumference.size());

    for (int i = 0; i < inferno->fireCount(); ++i) {
        if (!inferno->fireIsBurning()[i])
            continue;

        const vec center(inferno->fireXDelta()[i] + origin.x, inferno->fireYDelta()[i] + origin.y, inferno->fireZDelta()[i] + origin.z);

        for (const auto& point : flameCircumference)
            points.push_back(center + point);

    }

    points = convexHull2d(points);

    if (points.size() < 3) //we need at least 3 points to draw a polygon
        return;

    std::vector<ImVec2> screenPoints;
    screenPoints.reserve(points.size());

    for (const auto& point : points) {
        ImVec2 point2;
        if (W2s(point, point2))
            screenPoints.push_back(point2);
    }

    draw->AddConvexPolyFilled(screenPoints.data(), screenPoints.size(), color);
}

void Visuals::DrawEsp(ImDrawList* draw) {

    for (int i = 1; i <= I::ClientEntityList->GetHighestEntityIndex(); ++i) {
        Entity* entity = I::ClientEntityList->GetClientEntity(i);
        if (!entity)
            continue;

        if (entity->IsDormant())
            continue;

        switch (entity->GetClientClass()->m_ClassID) {
        case ClassID::Inferno:
            DrawInferno(entity, draw);
            break;
        }
    }

}
