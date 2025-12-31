/*
 * OpenD-Star CLI Tool
 *
 * Command-line utility for D-Star AMBE decoding.
 *
 * Usage:
 *   dstar_codec decode <input.ambe> <output.raw>
 *   dstar_codec info
 */

#include "opendstar.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void print_usage(const char *prog)
{
    fprintf(stderr, "OpenD-Star Codec Tool v%s\n\n", opendstar_version());
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s decode <input.ambe> <output.raw>  - Decode AMBE to PCM\n", prog);
    fprintf(stderr, "  %s info                              - Show library info\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "File formats:\n");
    fprintf(stderr, "  .ambe  - Raw D-Star AMBE frames (9 bytes per frame)\n");
    fprintf(stderr, "  .raw   - Raw PCM audio (16-bit signed, 8kHz, mono)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Convert PCM to WAV:\n");
    fprintf(stderr, "  sox -t raw -r 8000 -e signed -b 16 -c 1 output.raw output.wav\n");
}

static void print_info(void)
{
    printf("OpenD-Star Library Information\n");
    printf("==============================\n\n");
    printf("Version: %s\n\n", opendstar_version());
    printf("Codec: D-Star AMBE (AMBE 3600x2400)\n");
    printf("  - Voice data rate: 2400 bps\n");
    printf("  - FEC overhead: 1200 bps\n");
    printf("  - Total bit rate: 3600 bps\n\n");
    printf("Audio Format:\n");
    printf("  - Sample rate: %d Hz\n", OPENDSTAR_SAMPLE_RATE);
    printf("  - Bit depth: 16-bit signed\n");
    printf("  - Channels: Mono\n");
    printf("  - Frame size: %d samples (20ms)\n\n", OPENDSTAR_PCM_SAMPLES);
    printf("AMBE Frame Format:\n");
    printf("  - Size: %d bits (%d bytes)\n", OPENDSTAR_AMBE_FRAME_BITS, OPENDSTAR_AMBE_FRAME_BYTES);
    printf("  - Frame rate: 50 fps\n");
    printf("  - Voice parameters: %d bits\n\n", OPENDSTAR_VOICE_PARAMS);
    printf("Components:\n");
    printf("  - Decoder: mbelib-neo (GPL)\n");
}

static int do_decode(const char *input_file, const char *output_file)
{
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
        return 1;
    }

    FILE *fout = fopen(output_file, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
        fclose(fin);
        return 1;
    }

    opendstar_decoder_t *dec = opendstar_decoder_create();
    if (!dec) {
        fprintf(stderr, "Error: Failed to create decoder\n");
        fclose(fin);
        fclose(fout);
        return 1;
    }

    uint8_t ambe[OPENDSTAR_AMBE_FRAME_BYTES];
    int16_t pcm[OPENDSTAR_PCM_SAMPLES];
    int frame_count = 0;
    int muted_frames = 0;
    int total_errs = 0;

    while (true) {
        size_t bytes_read = fread(ambe, 1, OPENDSTAR_AMBE_FRAME_BYTES, fin);

        if (bytes_read == 0) {
            /* End of file */
            break;
        }

        if (bytes_read < OPENDSTAR_AMBE_FRAME_BYTES) {
            /* Partial frame at end of file */
            fprintf(stderr, "Warning: Partial frame at end of file (%zu bytes, expected %d). Ignoring.\n",
                    bytes_read, OPENDSTAR_AMBE_FRAME_BYTES);
            break;
        }

        int errs = 0;
        if (opendstar_decode(dec, ambe, pcm, &errs)) {
            /* Successful decode - write PCM output */
            size_t written = fwrite(pcm, sizeof(int16_t), OPENDSTAR_PCM_SAMPLES, fout);
            if (written != OPENDSTAR_PCM_SAMPLES) {
                fprintf(stderr, "Error: Failed to write PCM data (wrote %zu of %d samples)\n",
                        written, OPENDSTAR_PCM_SAMPLES);
                opendstar_decoder_destroy(dec);
                fclose(fin);
                fclose(fout);
                return 1;
            }
            frame_count++;
            total_errs += errs;
        } else {
            /* Frame muted due to excessive errors */
            muted_frames++;
            total_errs += errs;
        }
    }

    opendstar_decoder_destroy(dec);
    fclose(fin);
    fclose(fout);

    printf("Decoded %d frames (%.2f seconds)\n", frame_count, frame_count * 0.02);
    if (muted_frames > 0) {
        printf("Muted %d frames due to excessive errors (>%d)\n", muted_frames, OPENDSTAR_MUTING_THRESHOLD);
    }
    printf("Total bit errors corrected: %d\n", total_errs);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "info") == 0) {
        print_info();
        return 0;
    }

    if (strcmp(argv[1], "decode") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: decode requires input and output files\n\n");
            print_usage(argv[0]);
            return 1;
        }
        return do_decode(argv[2], argv[3]);
    }

    fprintf(stderr, "Error: Unknown command '%s'\n\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}
