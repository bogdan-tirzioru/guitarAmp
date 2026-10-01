#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* Build-time strap: set to 1 here, or override with a compiler -D flag.
 * Default: LED application only; no codec access. */
#ifndef GUITARAMP_CODEC_ENABLE
#define GUITARAMP_CODEC_ENABLE 0
#endif

#if (GUITARAMP_CODEC_ENABLE != 0) && (GUITARAMP_CODEC_ENABLE != 1)
#error "GUITARAMP_CODEC_ENABLE must be 0 or 1"
#endif
#endif
