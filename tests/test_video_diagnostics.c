#include "ss4s.h"
#include "library.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)

int main(int argc, char **argv) {
    setenv("APPID", "ss4s-diagnostics-test", 1);
    setenv("SS4S_MOCK_NDL_QUEUE_FRAMES", "0", 1);
    const char *module = argc > 1 ? argv[1] : "ndl-webos5";
    SS4S_Config config = {.videoDriver = module};
    CHECK(SS4S_Init(argc, argv, &config) == 0);
    CHECK(SS4S_PostInit(argc, argv) == 0);
    SS4S_Player *player = SS4S_PlayerOpen();
    CHECK(player != NULL);
    bool ndl = strcmp(module, "ndl-webos5") == 0;
    if (ndl) {
        /* The second open must fail in Release too, without replacing the
         * first context or unloading its decoder. */
        CHECK(SS4S_PlayerOpen() == NULL);
    }
    SS4S_VideoDiagnostics diagnostics;
    CHECK(!SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
    CHECK(diagnostics.kind == SS4S_VIDEO_DIAGNOSTICS_NONE);
    SS4S_VideoInfo info = {.codec = SS4S_VIDEO_H265, .width = 3840, .height = 2160,
                          .frameRateNumerator = 60, .frameRateDenominator = 1};
    CHECK(SS4S_PlayerVideoOpen(player, &info) == SS4S_VIDEO_OPEN_OK);
    if (ndl) {
        CHECK(SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        CHECK(diagnostics.kind == SS4S_VIDEO_DIAGNOSTICS_QUEUE_ESTIMATE);
        CHECK(diagnostics.renderQueueFrames == 0);
        CHECK(diagnostics.estimatedQueueDelayUs == 0);
        CHECK(diagnostics.completionLatencyUs == -1);
        setenv("SS4S_MOCK_NDL_QUEUE_FRAMES", "2", 1);
        CHECK(SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        CHECK(diagnostics.renderQueueFrames == 2);
        CHECK(diagnostics.estimatedQueueDelayUs == 33333);
        unsigned char data = 0;
        CHECK(SS4S_PlayerVideoFeed(player, &data, 1, SS4S_VIDEO_FEED_DATA_FRAME_END) == SS4S_VIDEO_FEED_OK);
        usleep(50000);
        CHECK(SS4S_PlayerVideoFeed(player, &data, 1, SS4S_VIDEO_FEED_DATA_FRAME_END) == SS4S_VIDEO_FEED_OK);
        CHECK(SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        CHECK(diagnostics.estimatedQueueDelayUs == 33333);
        setenv("SS4S_MOCK_NDL_QUEUE_FRAMES", "-1", 1);
        CHECK(!SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        int latency;
        CHECK(!SS4S_PlayerGetVideoLatency(player, 0, &latency));
    } else {
        /* Legacy modules have no extension symbol. They must remain loadable
         * and report unavailable rather than reading past the old driver ABI. */
        CHECK(!SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        CHECK(diagnostics.kind == SS4S_VIDEO_DIAGNOSTICS_NONE);
        SS4S_VideoStatsReportFrame(player, 31000);
        int legacyLatency;
        CHECK(SS4S_PlayerGetVideoLatency(player, 0, &legacyLatency));
        CHECK(legacyLatency == 31000);
        CHECK(!SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
        CHECK(diagnostics.kind == SS4S_VIDEO_DIAGNOSTICS_NONE);
    }
    CHECK(SS4S_PlayerVideoClose(player));
    CHECK(!SS4S_PlayerGetVideoDiagnostics(player, 0, &diagnostics));
    SS4S_PlayerClose(player);
    if (ndl) {
        player = SS4S_PlayerOpen();
        CHECK(player != NULL);
        SS4S_PlayerClose(player);
    }
    SS4S_Quit();
    return 0;
}
