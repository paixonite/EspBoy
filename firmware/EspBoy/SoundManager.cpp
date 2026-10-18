#include "SoundManager.h"

SoundManager::SoundManager(int buzzer_pin) {
    _buzzer_pin = buzzer_pin;
    pinMode(_buzzer_pin, OUTPUT);
    _is_playing = false;
    _is_muted = false;
}

void SoundManager::begin() {
    Preferences prefs;
    prefs.begin("sound_cfg", true);
    _is_muted = prefs.getBool("muted", false);
    prefs.end();
}

// Starts playing a melody
void SoundManager::play(const Note melody[], int melody_length) {
    if (_is_muted) {
        return; // If system is muted, simply do not start melody.
    }
    _current_melody = melody;
    _melody_length = melody_length;
    _current_note_index = 0;
    _is_playing = true;
    _note_start_time = millis();

    // Play the first note immediately
    int freq = _current_melody[0].frequency;
    int dur = _current_melody[0].duration;
    if (freq == REST) {
        noTone(_buzzer_pin);
    } else {
        tone(_buzzer_pin, freq, dur);
    }
}

// Stops playback
void SoundManager::stop() {
    _is_playing = false;
    noTone(_buzzer_pin);
}

// Check if it is time to play the next note
void SoundManager::loop() {
    if (!_is_playing || _is_muted) {
        return;
    }

    // Get current note duration
    int current_note_duration = _current_melody[_current_note_index].duration;
    
    // Add a small pause between notes (20% of note duration)
    int pause_between_notes = current_note_duration * 0.20;

    // Check if current note time + pause have elapsed
    if (millis() - _note_start_time > current_note_duration + pause_between_notes) {
        _current_note_index++; // Advance to next note

        // If melody finished
        if (_current_note_index >= _melody_length) {
            stop();
        } else {
            // If not finished, play next note
            _note_start_time = millis(); // Reset timer
            int freq = _current_melody[_current_note_index].frequency;
            int dur = _current_melody[_current_note_index].duration;
            if (freq == REST) {
                noTone(_buzzer_pin);
            } else {
                tone(_buzzer_pin, freq, dur);
            }
        }
    }
}

void SoundManager::setMute(bool muted) {
    _is_muted = muted;
    if (_is_muted) {
        stop(); // If muting, stop any currently playing sound
    }
    Preferences prefs;
    prefs.begin("sound_cfg", false);
    prefs.putBool("muted", _is_muted);
    prefs.end();
}

bool SoundManager::isMuted() const {
    return _is_muted;
}