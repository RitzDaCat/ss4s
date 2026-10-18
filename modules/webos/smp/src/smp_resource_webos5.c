#include "smp_resource.h"
#include "smp_player.h"

#include <SDL.h>

struct StarfishResource {
    char windowId[32];
    int maxRefreshRate;
    /* Where the video goes, in display pixels; zero size means the whole display */
    SDL_Rect dst;
};

static void ResourceApplyWindow(StarfishResource *resource, const SS4S_VideoInfo *info) {
    SDL_Rect src = {0, 0, info->width, info->height};
    SDL_Rect dst = resource->dst;
    if (dst.w <= 0 || dst.h <= 0) {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        dst = (SDL_Rect) {0, 0, dm.w, dm.h};
    }
    SDL_webOSSetExportedWindow(resource->windowId, &src, &dst);
}

StarfishResource *StarfishResourceCreate(const char *appId) {
    (void) appId;
    StarfishResource *res = calloc(1, sizeof(StarfishResource));
    assert(res != NULL);
    if (!SDL_webOSGetRefreshRate(&res->maxRefreshRate)) {
        res->maxRefreshRate = 60;
    }
    return res;
}

void StarfishResourceDestroy(StarfishResource *res) {
    if (res->windowId[0] != '\0') {
        SDL_webOSDestroyExportedWindow(res->windowId);
    }
    free(res);
}

bool StarfishResourcePopulateLoadPayload(StarfishResource *resource, jvalue_ref arg,
                                         const SS4S_AudioInfo *audioInfo, const SS4S_VideoInfo *videoInfo) {
    (void) audioInfo;
    if (videoInfo != NULL) {
        if (resource->windowId[0] == '\0') {
            const char *createdWnd = SDL_webOSCreateExportedWindow(0);
            if (createdWnd == NULL) {
                StarfishLibContext->Log(SS4S_LogLevelError, "SMP", "Didn't get a valid windowId: %s",
                                        resource->windowId);
                return false;
            }
            strncpy(resource->windowId, createdWnd, sizeof(resource->windowId) - 1);
        }
        jvalue_ref option = jobject_get(arg, J_CSTR_TO_BUF("option"));
        jobject_set(option, J_CSTR_TO_BUF("windowId"), j_cstr_to_jval(resource->windowId));
    }
    return true;
}

bool StarfishResourceSetMediaId(StarfishResource *resource, const char *connId) {
    (void) resource;
    (void) connId;
    return true;
}

bool StarfishResourceSetMediaAudioData(StarfishResource *resource, const char *data) {
    (void) resource;
    (void) data;
    return true;
}

bool StarfishResourceSetMediaVideoData(StarfishResource *resource, const char *data, bool hdr) {
    (void) resource;
    (void) data;
    (void) hdr;
    return true;
}

bool StarfishResourceLoadCompleted(StarfishResource *resource, const char *mediaId) {
    (void) resource;
    (void) mediaId;
    return true;
}

bool StarfishResourcePostLoad(StarfishResource *resource, const SS4S_VideoInfo *info) {
    if (resource->windowId[0] == '\0') {
        return false;
    }
    ResourceApplyWindow(resource, info);
    return true;
}

bool StarfishResourceSetDisplayArea(StarfishResource *resource, const SS4S_VideoInfo *info, int viewportWidth,
                                    int viewportHeight, const SS4S_VideoRect *dst) {
    if (dst == NULL || viewportWidth <= 0 || viewportHeight <= 0) {
        resource->dst = (SDL_Rect) {0, 0, 0, 0};
    } else {
        SDL_DisplayMode dm;
        SDL_GetCurrentDisplayMode(0, &dm);
        resource->dst = (SDL_Rect) {
                dst->x * dm.w / viewportWidth, dst->y * dm.h / viewportHeight,
                dst->width * dm.w / viewportWidth, dst->height * dm.h / viewportHeight,
        };
    }
    if (resource->windowId[0] != '\0') {
        ResourceApplyWindow(resource, info);
    }
    return true;
}

bool StarfishResourceStartPlaying(StarfishResource *resource) {
    (void) resource;
    return true;
}

bool StarfishResourcePostUnload(StarfishResource *resource) {
    (void) resource;
    return true;
}