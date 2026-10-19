#define NDL_DIRECTMEDIA_API_VERSION 2

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "NDL_directmedia.h"

#include "ndl_directmedia_mock.h"


int NDL_DirectMediaInit(const char *app_id) {
    return mock_ndl_init(app_id);
}

int NDL_DirectMediaUnload(void) {
    mock_ndl_lock(__func__);
    if (!audio_opened && !video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    audio_opened = false;
    video_opened = false;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectMediaLoad(NDL_DIRECTMEDIA_DATA_INFO_T *data, NDLMediaLoadCallback callback) {
    mock_ndl_lock(__func__);
    if (audio_opened || video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    usleep(1000000);
    audio_opened = data->audio.type != 0;
    video_opened = data->video.type != 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectAudioPlay(void *buffer, unsigned int size, long long pts) {
    (void) buffer;
    (void) size;
    (void) pts;
    mock_ndl_lock(__func__);
    if (!audio_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoPlay(void *buffer, unsigned int size, long long pts) {
    (void) buffer;
    (void) size;
    (void) pts;
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectAudioSupportMultiChannel(int *isSupported) {
    mock_ndl_lock(__func__);
    usleep(100000);
    *isSupported = 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoGetRenderBufferLength(int *length) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    /* Test-only control; the production driver never reads this variable. */
    const char *configured = getenv("SS4S_MOCK_NDL_QUEUE_FRAMES");
    *length = configured != NULL ? atoi(configured) : 0;
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoSetFrameDropThreshold(int threshold) {
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    const char *value = getenv("SS4S_MOCK_NDL_DROP_CALLS");
    unsigned int calls = value != NULL ? (unsigned int) atoi(value) + 1 : 1;
    char text[32];
    snprintf(text, sizeof(text), "%u", calls);
    setenv("SS4S_MOCK_NDL_DROP_CALLS", text, 1);
    value = getenv("SS4S_MOCK_NDL_DROP_FAIL_CALLS");
    unsigned int failures = value != NULL ? (unsigned int) atoi(value) : 0;
    if (threshold != 1 || calls <= failures) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    snprintf(text, sizeof(text), "%d", threshold);
    setenv("SS4S_MOCK_NDL_APPLIED_DROP_THRESHOLD", text, 1);
    mock_ndl_unlock(__func__);
    return 0;
}

int NDL_DirectVideoSetHDRInfo(NDL_DIRECTVIDEO_HDR_INFO_T hdrInfo) {
    (void) hdrInfo;
    mock_ndl_lock(__func__);
    if (!video_opened) {
        mock_ndl_unlock(__func__);
        return -1;
    }
    mock_ndl_unlock(__func__);
    return 0;
}
