#include "ss4s.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)

static int calls(void) {
    const char *value = getenv("SS4S_MOCK_NDL_DROP_CALLS");
    return value != NULL ? atoi(value) : 0;
}

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "success";
    bool reject = strcmp(mode, "reject") == 0;
    bool retry = strcmp(mode, "retry") == 0;
    setenv("APPID", "ss4s-render-drop-test", 1);
    setenv("SS4S_MOCK_NDL_DROP_FAIL_CALLS", reject ? "100" : retry ? "2" : "0", 1);
    setenv("SS4S_MOCK_NDL_DROP_CALLS", "0", 1);
    setenv("SS4S_MOCK_NDL_QUEUE_FRAMES", "3", 1);
    unsetenv("SS4S_MOCK_NDL_APPLIED_DROP_THRESHOLD");
    SS4S_Config config = {.videoDriver = "ndl-webos5"};
    CHECK(SS4S_Init(argc, argv, &config) == 0);
    CHECK(SS4S_PostInit(argc, argv) == 0);
    SS4S_Player *player = SS4S_PlayerOpen();
    CHECK(player != NULL);
    CHECK(calls() == 0);
    SS4S_VideoInfo info = {.codec = SS4S_VIDEO_H265, .width = 1920, .height = 1080,
                          .frameRateNumerator = 120, .frameRateDenominator = 1};
    CHECK(SS4S_PlayerVideoOpen(player, &info) == SS4S_VIDEO_OPEN_OK);
    CHECK(calls() == 1);
    unsigned char data = 0;
    for (unsigned int i = 0; i < 12; ++i) {
        CHECK(SS4S_PlayerVideoFeed(player, &data, 1, SS4S_VIDEO_FEED_DATA_FRAME_END) == SS4S_VIDEO_FEED_OK);
    }
    CHECK(calls() == (reject ? 8 : retry ? 3 : 1));
    const char *applied = getenv("SS4S_MOCK_NDL_APPLIED_DROP_THRESHOLD");
    CHECK(reject ? applied == NULL : applied != NULL && strcmp(applied, "1") == 0);
    SS4S_VideoDiagnostics diagnostics;
    CHECK(SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
    /* The request must never clamp or fabricate the reported queue depth. */
    CHECK(diagnostics.renderQueueFrames == 3);
    CHECK(diagnostics.estimatedQueueDelayUs == 25000);
    setenv("SS4S_MOCK_NDL_DROP_CALLS", "0", 1);
    unsetenv("SS4S_MOCK_NDL_APPLIED_DROP_THRESHOLD");
    CHECK(SS4S_PlayerVideoSizeChanged(player, 1024, 768));
    CHECK(calls() == 1); /* Reapply after the native pipeline is recreated. */
    CHECK(SS4S_PlayerVideoClose(player));
    SS4S_PlayerClose(player);
    SS4S_Quit();
    return 0;
}
