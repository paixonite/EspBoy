#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "Arduino.h"
#include "ConfigManager.h"
#include "audio_assets.h"

class SoundManager {
public:
    SoundManager(int buzzer_pin, ConfigManager* config = nullptr);
    void begin();
    void play(const Note melody[], int melody_length);
    void stop();
    void loop();

    void setVolume(int volume);
    int getVolume() const;
    void setMute(bool muted);
    bool isMuted() const;

private:
    int _buzzer_pin;
    ConfigManager* _config;
    const Note* _current_melody;
    int _melody_length;
    int _current_note_index;
    unsigned long _note_start_time;
    bool _is_playing;
    bool _note_ringing;

    uint32_t calculateDuty() const;
    void startNote(int freq);
};

#endif // SOUNDMANAGER_H