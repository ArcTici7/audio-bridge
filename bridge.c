#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <aaudio/AAudio.h>
#include <tinyalsa/asoundlib.h>

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAMES 192

int main() {
    struct pcm_config config = {
        .channels = CHANNELS,
        .rate = SAMPLE_RATE,
        .format = PCM_FORMAT_S16_LE,
        .period_size = FRAMES,
        .period_count = 4,
    };

    struct pcm *pcm_in = pcm_open(1, 0, PCM_IN, &config);
    if (!pcm_is_ready(pcm_in)) {
        printf("PCM input open failed\n");
        return -1;
    }

    AAudioStreamBuilder *builder;
    AAudio_createStreamBuilder(&builder);

    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setSampleRate(builder, SAMPLE_RATE);
    AAudioStreamBuilder_setChannelCount(builder, CHANNELS);
    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);

    AAudioStream *stream;
    if (AAudioStreamBuilder_openStream(builder, &stream) != AAUDIO_OK) {
        printf("AAudio open failed\n");
        return -1;
    }

    AAudioStream_requestStart(stream);

    int16_t buffer[FRAMES * CHANNELS];

    while (1) {
        int ret = pcm_read(pcm_in, buffer, sizeof(buffer));
        if (ret != 0) {
            printf("pcm_read error\n");
            continue;
        }

        int written = AAudioStream_write(stream, buffer, FRAMES, 1000000);
        if (written < 0) {
            printf("AAudio write error: %d\n", written);
        }
    }

    pcm_close(pcm_in);
    AAudioStream_close(stream);
    return 0;
}
