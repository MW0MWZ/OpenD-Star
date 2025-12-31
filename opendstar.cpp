/*
 * OpenD-Star - Open Source D-Star (AMBE) Vocoder Library
 *
 * Implementation file
 */

#include "opendstar.h"
#include <cstring>
#include <cstdlib>
#include <cassert>

/* mbelib-neo for decoding */
extern "C" {
#include "mbelib.h"
}

/*
 * ============================================================================
 * Version
 * ============================================================================
 */

static const char *version_string = "1.0.0";

/*
 * ============================================================================
 * Decoder Implementation
 * ============================================================================
 */

struct opendstar_decoder {
    mbe_parms cur_mp;
    mbe_parms prev_mp;
    mbe_parms prev_mp_enhanced;
};

opendstar_decoder_t *opendstar_decoder_create(void)
{
    opendstar_decoder_t *dec = static_cast<opendstar_decoder_t *>(calloc(1, sizeof(opendstar_decoder_t)));
    if (dec) {
        /* Initialize MBE parameters. Note: mbe_initMbeParms() always succeeds
         * as it only performs memory initialization (no allocation or I/O). */
        mbe_initMbeParms(&dec->cur_mp, &dec->prev_mp, &dec->prev_mp_enhanced);
    }
    return dec;
}

void opendstar_decoder_destroy(opendstar_decoder_t *dec)
{
    free(dec);
}

void opendstar_decoder_reset(opendstar_decoder_t *dec)
{
    if (dec) {
        mbe_initMbeParms(&dec->cur_mp, &dec->prev_mp, &dec->prev_mp_enhanced);
    }
}

/*
 * Convert 72-bit AMBE frame to ambe_fr[4][24] format for D-Star.
 *
 * D-Star AMBE 3600x2400 frame structure:
 * - Total: 72 bits (9 bytes)
 * - Organized as 4 groups of 24 bits each
 * - Group 0: bits 0-23
 * - Group 1: bits 24-47
 * - Group 2: bits 48-71
 * - Group 3: reserved/unused (set to 0)
 *
 * PRECONDITIONS (verified by caller):
 * - bytes points to valid buffer of at least OPENDSTAR_AMBE_FRAME_BYTES (9) bytes
 * - ambe_fr points to valid buffer of 4x24 char array
 * - Function processes exactly 72 bits (3 groups * 24 bits)
 * - byte_idx will range from 0 to 8 (within 9-byte buffer)
 */
static void bytes_to_ambe_fr(const uint8_t *bytes, char ambe_fr[4][24])
{
    assert(bytes != nullptr);

    /* Clear all groups */
    memset(ambe_fr, 0, 4 * 24);

    /* Extract bits from bytes into ambe_fr format - optimized to reduce divisions */
    int byte_idx = 0;
    int bit_pos = 7;

    for (int group = 0; group < 3; group++) {
        for (int i = 0; i < 24; i++) {
            ambe_fr[group][i] = (bytes[byte_idx] >> bit_pos) & 1;

            /* Move to next bit */
            if (bit_pos == 0) {
                bit_pos = 7;
                byte_idx++;
            } else {
                bit_pos--;
            }
        }
    }

    /* Verify we processed exactly 72 bits = 9 bytes */
    assert(byte_idx == 9 && bit_pos == 7);
}

bool opendstar_decode(opendstar_decoder_t *dec,
                      const uint8_t ambe[OPENDSTAR_AMBE_FRAME_BYTES],
                      int16_t pcm[OPENDSTAR_PCM_SAMPLES],
                      int *errs)
{
    if (!dec || !ambe || !pcm)
        return false;

    /* Convert bytes to ambe_fr format */
    char ambe_fr[4][24];
    bytes_to_ambe_fr(ambe, ambe_fr);

    /* Decode AMBE frame to PCM using mbelib's all-in-one function */
    char ambe_d[49];  /* Output buffer for demodulated parameters */
    int errs1 = 0, errs2 = 0;
    char err_str[64];  /* Required by mbelib - cannot pass NULL */
    err_str[0] = '\0';

    mbe_processAmbe3600x2400Frame(pcm, &errs1, &errs2, err_str, ambe_fr, ambe_d,
                                  &dec->cur_mp, &dec->prev_mp, &dec->prev_mp_enhanced,
                                  OPENDSTAR_UV_QUALITY);

    int total_errs = errs1 + errs2;

    if (errs) {
        *errs = total_errs;
    }

    /* Return false if error count exceeds muting threshold.
     * Frames with excessive errors produce harsh audio artifacts. */
    if (total_errs > OPENDSTAR_MUTING_THRESHOLD) {
        return false;
    }

    return true;
}

/*
 * ============================================================================
 * Utility Functions
 * ============================================================================
 */

const char *opendstar_version(void)
{
    return version_string;
}

void opendstar_version_info(int *major, int *minor, int *patch)
{
    if (major) *major = OPENDSTAR_VERSION_MAJOR;
    if (minor) *minor = OPENDSTAR_VERSION_MINOR;
    if (patch) *patch = OPENDSTAR_VERSION_PATCH;
}
