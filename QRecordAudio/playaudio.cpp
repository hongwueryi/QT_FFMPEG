#include <QThread>
#include <qfile.h>
#include "playaudio.h"
#include "pch.h"
// 采样率
#define SAMPLE_RATE 44100
// 采样格式
#define SAMPLE_FORMAT AUDIO_S16LSB
// 采样大小
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量
#define SAMPLES 1024 
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// 文件缓冲区的大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

#define BUFFER_LEN (SAMPLES * BYTES_PER_SAMPLE + 1)
QFile file(FILENAME);

CAudioPlayer::CAudioPlayer(QObject* parent)
{

}

CAudioPlayer::~CAudioPlayer()
{
    file.close();
    // 关闭音频设备
    SDL_CloseAudio();
    // 清理所有初始化的子系统
    SDL_Quit();
}

void CAudioPlayer::run()
{
// 开始播放
    SDL_PauseAudio(0);

    // 存放文件数据
    Uint8 data[BUFFER_LEN];

    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if (buffer.len > 0) continue;

        buffer.len = file.read((char*)data, BUFFER_SIZE);

        // 文件数据已经读取完毕
        if (buffer.len <= 0) {
            // 剩余的样本数量
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        }

        // 读取到了文件数据
        buffer.data = data;
    }
}

// userdata：SDL_AudioSpec.userdata
// stream：音频缓冲区（需要将音频数据填充到这个缓冲区）
// len：音频缓冲区的大小（SDL_AudioSpec.samples * 每个样本的大小）
void pull_audio_data(void* userdata, Uint8* stream, int len)
{
    // 清空stream
    SDL_memset(stream, 0, len);

    // 取出缓冲信息
    AudioBuffer* buffer = (AudioBuffer*)userdata;
    if (buffer->len == 0) return;

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;

    // 填充数据
    SDL_MixAudio(stream,
        buffer->data,
        buffer->pullLen,
        SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void CAudioPlayer::Init()
{
    SDL_version v;
    SDL_VERSION(&v);
    printf("%d.%d.%d\n", v.major, v.minor, v.patch);
    if (0 != SDL_Init(SDL_INIT_AUDIO))
    {
        printf("SDL_Init failed, ERR=%s\n", SDL_GetError());
    }

    SDL_AudioSpec spec;
    spec.format = SAMPLE_FORMAT;
    spec.channels = CHANNELS;
    spec.freq = SAMPLE_RATE;
    spec.samples = SAMPLES;
    spec.callback = pull_audio_data;
    spec.userdata = &buffer;

    if (0 != SDL_OpenAudio(&spec, nullptr))
    {
        printf("SDL_OpenAudio failed, ERR=%s\n", SDL_GetError());
    }

    if (!file.open(QFile::ReadOnly)) {
        printf("file open failed, ERR=%s\n", SDL_GetError());
        // 关闭音频设备
        SDL_CloseAudio();
        // 清除所有初始化的子系统
        SDL_Quit();
        return;
    }

    this->start();
}