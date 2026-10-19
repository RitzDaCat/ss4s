#include <stdint.h>
#include <stdio.h>
#include <time.h>

static uint64_t nowUs = UINT64_C(2147483000);
static int StatsClockGetTime(clockid_t clock, struct timespec *ts) {
    (void) clock;
    ts->tv_sec = nowUs / 1000000;
    ts->tv_nsec = (nowUs % 1000000) * 1000;
    return 0;
}

/* Deterministic clock, including the 32-bit microsecond boundary; compile the
 * real implementation rather than a second implementation of the algorithm. */
#define clock_gettime StatsClockGetTime
#include "../src/stats.c"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); return 1; \
} } while (0)

int main(void) {
    SS4S_StatsCounter counter;
    SS4S_StatsCounterInit(&counter, 3);
    /* Unused slots must never contribute, regardless of allocator contents. */
    for (size_t i = 0; i < counter.capacity; i++) {
        counter.items[i] = (SS4S_StatsItem) {nowUs, 999999};
    }
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == -1);
    SS4S_StatsCounterReportFrame(&counter, 100);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 100);
    nowUs += 1000;
    SS4S_StatsCounterReportFrame(&counter, 300);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 200);
    nowUs += 1000;
    SS4S_StatsCounterReportFrame(&counter, 500);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 300);
    nowUs += 1000;
    SS4S_StatsCounterReportFrame(&counter, 700);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 500);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 500) == 700);
    nowUs += 1000001;
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == -1);
    uint32_t token = SS4S_StatsCounterBeginFrame(&counter);
    nowUs += 50;
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == -1);
    SS4S_StatsCounterEndFrame(&counter, token);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 50);
    SS4S_StatsCounterDeinit(&counter);
    SS4S_StatsCounterInit(&counter, 3);
    SS4S_StatsCounterReportFrame(&counter, 2000000000u);
    SS4S_StatsCounterReportFrame(&counter, 2000000000u);
    SS4S_StatsCounterReportFrame(&counter, 2000000000u);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == 2000000000);
    SS4S_StatsCounterReportFrame(&counter, UINT32_MAX - 1);
    SS4S_StatsCounterReportFrame(&counter, UINT32_MAX - 1);
    SS4S_StatsCounterReportFrame(&counter, UINT32_MAX - 1);
    CHECK(SS4S_StatsCounterGetAverageLatencyUs(&counter, 1000000) == INT32_MAX);
    SS4S_StatsCounterDeinit(&counter);
    return 0;
}
