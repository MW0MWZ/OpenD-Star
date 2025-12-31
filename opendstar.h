/*
 * OpenD-Star - Open Source D-Star (AMBE) Vocoder Library
 *
 * A software implementation of the D-Star AMBE vocoder for decoding
 * digital voice. No proprietary DVSI hardware required.
 *
 * This library provides D-Star AMBE frame decoding to PCM audio.
 * Encoding is not currently supported.
 *
 * THREAD SAFETY:
 * - Each opendstar_decoder_t instance is NOT thread-safe and must not
 *   be accessed concurrently from multiple threads.
 * - Multiple decoder instances can be used concurrently across threads
 *   (i.e., different decoders in different threads is safe).
 * - Utility functions (opendstar_version, opendstar_version_info)
 *   are thread-safe and reentrant.
 *
 * License: GNU General Public License v2.0 (GPL-2.0)
 */

#ifndef OPENDSTAR_H
#define OPENDSTAR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * Version Information
 * ============================================================================
 */

#define OPENDSTAR_VERSION_MAJOR   1
#define OPENDSTAR_VERSION_MINOR   0
#define OPENDSTAR_VERSION_PATCH   0

/*
 * ============================================================================
 * Constants
 * ============================================================================
 */

/* D-Star AMBE frame size: 72 bits = 9 bytes */
#define OPENDSTAR_AMBE_FRAME_BYTES  9
#define OPENDSTAR_AMBE_FRAME_BITS   72

/* PCM audio: 160 samples per 20ms frame at 8kHz */
#define OPENDSTAR_PCM_SAMPLES       160
#define OPENDSTAR_SAMPLE_RATE       8000

/* Voice parameters: 49 bits per frame */
#define OPENDSTAR_VOICE_PARAMS      49

/*
 * Error muting threshold: frames with more than this many corrected errors
 * should typically be muted to avoid harsh audio artifacts.
 * Based on empirical testing with D-Star AMBE streams.
 */
#define OPENDSTAR_MUTING_THRESHOLD  7

/*
 * UV (Unvoiced/Voiced) quality parameter for synthesis (0-3).
 * Higher values produce better quality output but require more processing.
 * 3 = High quality (recommended for D-Star)
 */
#define OPENDSTAR_UV_QUALITY        3

/*
 * ============================================================================
 * Types
 * ============================================================================
 */

/* Opaque decoder handle */
typedef struct opendstar_decoder opendstar_decoder_t;

/*
 * ============================================================================
 * Decoder API
 * ============================================================================
 */

/**
 * Create a new D-Star AMBE decoder instance.
 *
 * @return Decoder handle, or NULL on allocation failure.
 *         Must be freed with opendstar_decoder_destroy().
 */
opendstar_decoder_t *opendstar_decoder_create(void);

/**
 * Destroy a decoder instance and free all resources.
 *
 * @param dec  Decoder handle (may be NULL)
 */
void opendstar_decoder_destroy(opendstar_decoder_t *dec);

/**
 * Reset decoder state.
 *
 * Call this at the start of a new voice transmission to clear
 * any state from previous frames.
 *
 * @param dec  Decoder handle
 */
void opendstar_decoder_reset(opendstar_decoder_t *dec);

/**
 * Decode one D-Star AMBE frame to PCM audio.
 *
 * @param dec   Decoder handle
 * @param ambe  Input: 9-byte (72-bit) AMBE frame
 * @param pcm   Output: 160 samples of 16-bit signed PCM at 8kHz
 * @param errs  Optional output: number of bit errors corrected (may be NULL)
 *
 * @return true on success, false on failure (NULL parameters or excessive errors)
 *
 * Note: Returns false when error count exceeds OPENDSTAR_MUTING_THRESHOLD,
 *       indicating the frame is too corrupted for clean audio output.
 */
bool opendstar_decode(opendstar_decoder_t *dec,
                      const uint8_t ambe[OPENDSTAR_AMBE_FRAME_BYTES],
                      int16_t pcm[OPENDSTAR_PCM_SAMPLES],
                      int *errs);

/*
 * ============================================================================
 * Utility Functions
 * ============================================================================
 */

/**
 * Get the library version string.
 *
 * @return Version string (e.g., "1.0.0")
 */
const char *opendstar_version(void);

/**
 * Get detailed version information.
 *
 * @param major  Output: major version number (may be NULL)
 * @param minor  Output: minor version number (may be NULL)
 * @param patch  Output: patch version number (may be NULL)
 */
void opendstar_version_info(int *major, int *minor, int *patch);


#ifdef __cplusplus
}
#endif

#endif /* OPENDSTAR_H */
