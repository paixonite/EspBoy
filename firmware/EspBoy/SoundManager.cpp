#include "SoundManager.h"

SoundManager::SoundManager(int buzzer_pin) {
    _buzzer_pin = buzzer_pin;
    pinMode(_buzzer_pin, OUTPUT);
    _is_playing = false;
    _is_muted = false;
}

// Inicia a reprodução de uma melodia
void SoundManager::play(const Note melody[], int melody_length) {
    if (_is_muted) {
        return; // Se o sistema está mudo, simplesmente não inicia a melodia.
    }
    _current_melody = melody;
    _melody_length = melody_length;
    _current_note_index = 0;
    _is_playing = true;
    _note_start_time = millis();

    // Toca a primeira nota imediatamente
    int freq = _current_melody[0].frequency;
    int dur = _current_melody[0].duration;
    if (freq == REST) {
        noTone(_buzzer_pin);
    } else {
        tone(_buzzer_pin, freq, dur);
    }
}

// Para a reprodução
void SoundManager::stop() {
    _is_playing = false;
    noTone(_buzzer_pin);
}

// Verifica se é hora de tocar a próxima nota
void SoundManager::loop() {
    if (!_is_playing || _is_muted) {
        return;
    }

    // Pega a duração da nota atual
    int current_note_duration = _current_melody[_current_note_index].duration;
    
    // Adiciona uma pequena pausa entre as notas (20% da duração da nota)
    int pause_between_notes = current_note_duration * 0.20;

    // Verifica se o tempo da nota atual + a pausa já passaram
    if (millis() - _note_start_time > current_note_duration + pause_between_notes) {
        _current_note_index++; // Vai para a próxima nota

        // Se a melodia terminou
        if (_current_note_index >= _melody_length) {
            stop();
        } else {
            // Se não terminou, toca a próxima nota
            _note_start_time = millis(); // Reseta o timer
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
        stop(); // Se está silenciando, para qualquer som que esteja tocando
    }
}

bool SoundManager::isMuted() const {
    return _is_muted;
}