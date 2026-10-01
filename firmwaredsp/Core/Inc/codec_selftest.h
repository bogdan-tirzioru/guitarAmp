#ifndef CODEC_SELFTEST_H
#define CODEC_SELFTEST_H
/* Optional in-firmware simulated-bus test. No board or peripheral access.
 * Returns 0 on success; otherwise the failed check's source line.
 * Called at startup only when GUITARAMP_CODEC_SELFTEST_ENABLE == 1. */
int codec_selftest_run(void);
#endif
