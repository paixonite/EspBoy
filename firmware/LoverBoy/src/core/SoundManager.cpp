#include "SoundManager.h"
#include <math.h>

static const uint8_t BUZZER_LEDC_RES = 10; // 10-bit resolution (0..1023, 512 = 50% duty cycle)
static const uint32_t BUZZER_INIT_FREQ = 2000;

SoundManager::SoundManager(int buzzer_pin, ConfigManager* config)
    : _buzzer_pin(buzzer_pin),
      _config(config),
      _current_melody(nullptr),
      _melody_length(0),
      _current_note_index(0),
      _note_start_time(0),
      _is_playing(false),
      _note_ringing(false) {
}

void SoundManager::begin() {
    // Attach buzzer pin to an independent 10-bit LEDC timer/channel
    ledcAttach(_buzzer_pin, BUZZER_INIT_FREQ, BUZZER_LEDC_RES);
    ledcWrite(_buzzer_pin, 0);
}

uint32_t SoundManager::calculateDuty() const {
    int vol = getVolume();
    if (vol <= 0) return 0;
    if (vol >= 100) return 512; // 50% duty cycle = maximum acoustic output

    // Exponential curve maps 10%..90% volume to perceptually linear pulse widths (2..381)
    float normalized = vol / 100.0f;
    uint32_t duty = (uint32_t)(512.0f * powf(normalized, 2.8f));
    if (duty < 2) duty = 2;
    if (duty > 512) duty = 512;
    return duty;
}

void SoundManager::startNote(int freq) {
    if (freq == REST || isMuted()) {
        ledcWrite(_buzzer_pin, 0);
        _note_ringing = false;
    } else {
        ledcWriteTone(_buzzer_pin, freq);
        ledcWrite(_buzzer_pin, calculateDuty());
        _note_ringing = true;
    }
}

void SoundManager::play(const Note melody[], int melody_length) {
    if (isMuted() || melody == nullptr || melody_length <= 0) {
        return;
    }
    _current_melody = melody;
    _melody_length = melody_length;
    _current_note_index = 0;
    _is_playing = true;
    _note_start_time = millis();

    startNote(_current_melody[0].frequency);
}

void SoundManager::stop() {
    _is_playing = false;
    _note_ringing = false;
    ledcWrite(_buzzer_pin, 0);
}

void SoundManager::loop() {
    if (!_is_playing) {
        return;
    }
    if (isMuted()) {
        stop();
        return;
    }

    unsigned long elapsed = millis() - _note_start_time;
    int current_note_duration = _current_melody[_current_note_index].duration;
    int pause_between_notes = current_note_duration * 0.20;

    // Silence buzzer after note duration for clean articulation gap between notes
    if (_note_ringing && elapsed >= (unsigned long)current_note_duration) {
        ledcWrite(_buzzer_pin, 0);
        _note_ringing = false;
    }

    // Advance to next note after duration + articulation pause
    if (elapsed > (unsigned long)(current_note_duration + pause_between_notes)) {
        _current_note_index++;

        if (_current_note_index >= _melody_length) {
            stop();
        } else {
            _note_start_time = millis();
            startNote(_current_melody[_current_note_index].frequency);
        }
    }
}

void SoundManager::setVolume(int volume) {
    if (_config != nullptr) {
        _config->setVolume(volume);
    }
    if (volume <= 0) {
        stop();
    } else if (_is_playing && _note_ringing) {
        ledcWrite(_buzzer_pin, calculateDuty());
    }
}

int SoundManager::getVolume() const {
    return (_config != nullptr) ? _config->getVolume() : 80;
}

void SoundManager::setMute(bool muted) {
    if (_config != nullptr) {
        if (muted && !_config->isMuted()) {
            _config->toggleMute();
        } else if (!muted && _config->isMuted()) {
            _config->toggleMute();
        }
    }
    if (muted) {
        stop();
    }
}

bool SoundManager::isMuted() const {
    return getVolume() <= 0;
}