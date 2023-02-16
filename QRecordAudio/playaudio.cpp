#include <QThread>
#include <qfile.h>
#include "playaudio.h"
#include "pch.h"
// ������
#define SAMPLE_RATE 44100
// ������ʽ
#define SAMPLE_FORMAT AUDIO_S16LSB
// ������С
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// ������
#define CHANNELS 2
// ��Ƶ����������������
#define SAMPLES 1024 
// ÿ������ռ�ö��ٸ��ֽ�
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// �ļ��������Ĵ�С
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

#define BUFFER_LEN (SAMPLES * BYTES_PER_SAMPLE + 1)
QFile file(FILENAME);

CAudioPlayer::CAudioPlayer(QObject* parent)
{

}

CAudioPlayer::~CAudioPlayer()
{
    file.close();
    // �ر���Ƶ�豸
    SDL_CloseAudio();
    // �������г�ʼ������ϵͳ
    SDL_Quit();
}

void CAudioPlayer::run()
{
// ��ʼ����
    SDL_PauseAudio(0);

    // ����ļ�����
    Uint8 data[BUFFER_LEN];

    while (!isInterruptionRequested()) {
        // ֻҪ���ļ��ж�ȡ����Ƶ���ݣ���û�������ϣ�������
        if (buffer.len > 0) continue;

        buffer.len = file.read((char*)data, BUFFER_SIZE);

        // �ļ������Ѿ���ȡ���
        if (buffer.len <= 0) {
            // ʣ�����������
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        }

        // ��ȡ�����ļ�����
        buffer.data = data;
    }
}

// userdata��SDL_AudioSpec.userdata
// stream����Ƶ����������Ҫ����Ƶ������䵽�����������
// len����Ƶ�������Ĵ�С��SDL_AudioSpec.samples * ÿ�������Ĵ�С��
void pull_audio_data(void* userdata, Uint8* stream, int len)
{
    // ���stream
    SDL_memset(stream, 0, len);

    // ȡ��������Ϣ
    AudioBuffer* buffer = (AudioBuffer*)userdata;
    if (buffer->len == 0) return;

    // ȡlen��bufferLen����Сֵ��Ϊ�˱�֤���ݰ�ȫ����ָֹ��Խ�磩
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;

    // �������
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
        // �ر���Ƶ�豸
        SDL_CloseAudio();
        // ������г�ʼ������ϵͳ
        SDL_Quit();
        return;
    }

    this->start();
}