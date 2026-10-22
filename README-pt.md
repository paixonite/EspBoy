# 🎮 LoverBoy ![Status](https://img.shields.io/badge/status-in%20development-yellow) ![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

> *Um projeto de paixão.*

[English version](./README.md)

![LoverBoy logo](LoverBoy_logo.png)

Um console de videogame portátil inspirado no clássico "Game Boy", construído do zero utilizando um LILYGO ESP32-S3, componentes eletrônicos básicos e muita programação! Este é um projeto educacional e pessoal, originalmente desenvolvido com os recursos e espaço maker do Colégio Técnico da UFMG (Coltec) e em constante evolução durante a minha graduação em Ciência da Computação (DCC/UFMG).

## 🕹️ Sobre o Projeto

O objetivo do LoverBoy é criar uma plataforma modular e robusta de hardware e software, desenvolvendo habilidades práticas em:

- **Desenvolvimento de Hardware:** Montagem de circuitos, soldagem, roteamento de PCB e integração de componentes.
- **Programação de Baixo Nível:** Interação direta com GPIOs, temporizadores (PWM) e periféricos do microcontrolador.
- **Arquitetura de Software:** Sistema operacional organizado por responsabilidades, com classes abstratas e uso do padrão de projeto **Template Method** para ditar o ciclo de vida dos jogos.
- **Gerenciamento de Energia:** Sistema de alimentação com bateria recarregável LiPo para portabilidade real.

## ⚙️ Hardware Utilizado

- **Placa de Desenvolvimento:** LILYGO T-Display ESP32-S3 (com tela TFT de 1.9" 170x320 paralela de 8 bits integrada).
- **Entradas:** 8 botões táteis (D-Pad: Cima, Baixo, Esquerda, Direita; Ações: A, B; Sistema: Start, Select).
- **Áudio:** 1 Buzzer passivo para melodias e efeitos sonoros não-bloqueantes.
- **Alimentação:** Bateria recarregável LiPo 3.7V, gerenciada por módulo de carga TP4056 USB-C e chave deslizante.

## ✨ Recursos do Firmware

O software do LoverBoy foi projetado para ser modular, escalável e responsivo:

- **Jogos Inclusos:** Clássicos **Crossy**, **Flappy Bird**, **Frogger**, **Genius**, **Snake** e **Tetris** rodando a ~60 FPS com alta fluidez.
- **Renderização por Diferença (Delta/Zero-Flicker):** Pipeline gráfico otimizado para barramento SPI/paralelo que calcula apenas o que mudou entre os quadros, apagando e redesenhando estritamente os pixels em movimento para eliminar cintilações.
- **Sistema de Recordes Persistente:** Memória Flash integrada (Preferences/NVS) que salva o Top 3 de cada jogo, incluindo tela clássica estilo arcade para inserir 3 iniciais.
- **Tutoriais Integrados:** Todos os jogos contam com uma tela dedicada de tutorial acessível a partir do menu inicial em carrossel vertical de 3 opções (Jogar, Tutorial, Recordes), explicando controles e objetivos em inglês e português.
- **Configurações de Som Permanentes:** O estado de mudo persiste entre reinicializações, evitando o buzzer alto na inicialização.
- **Interface Multilíngue:** Inglês (padrão) e Português do Brasil, selecionáveis nas Configurações. Todo texto da tela fica em uma única tabela (`i18n_strings.h`), e as letras acentuadas são desenhadas sobre a fonte pixelada retrô pelo `TextRenderer`.
- **Menu de Configurações Dinâmico:** Interface no console para ajuste de volume, brilho da tela e reset de recordes.
- **Áudio Não-Bloqueante:** Fila de som baseada em máquina de estados por contagem de milissegundos, sem congelar a física ou lógica dos jogos.
- **Controles Responsivos (`InputManager`):** Gerenciador centralizado de entradas com detecção de borda de latência zero, qualificação de soltura contra cliques duplos (debounce), repetição automática (DAS/ARR) e bloqueio de vazamento entre telas.

## 📂 Estrutura do Código

O projeto segue uma organização estrita de responsabilidades:

```
firmware/LoverBoy/
|
|-- LoverBoy.ino                      # Ponto de entrada: inicializa o sistema e roda o loop
|-- src/
    |-- core/                       # Gerenciadores de hardware e núcleo do sistema
    |   |-- LoverBoyCore.h/.cpp       # Orquestrador global e alternador de modos (Menu vs Jogo)
    |   |-- InputManager.h/.cpp     # Leitura centralizada de botões, debounce e auto-repeat
    |   |-- BatteryManager.h/.cpp   # Lógica matemática e ADC para leitura da bateria
    |   |-- ConfigManager.h/.cpp    # Persistência centralizada de configurações na NVS
    |   |-- SoundManager.h/.cpp     # Gerenciador de fila de áudio e melodias não-bloqueante
    |   |-- pins.h                  # Mapeamento de pinos do ESP32-S3 e display
    |   |-- audio_assets.h          # Biblioteca de frequências e vetores de melodias
    |
    |-- games/                      # Motor de jogos e jogos implementados
    |   |-- Game.h/.cpp             # Classe base abstrata com ciclo de vida (Template Method)
    |   |-- CrossyGame.h/.cpp       # Implementação do jogo Crossy
    |   |-- FlappyBirdGame.h/.cpp   # Implementação do jogo Flappy Bird
    |   |-- FroggerGame.h/.cpp      # Implementação do jogo Frogger
    |   |-- GeniusGame.h/.cpp       # Implementação do jogo Genius
    |   |-- SnakeGame.h/.cpp        # Implementação do jogo Snake
    |   |-- TetrisGame.h/.cpp       # Implementação do jogo Tetris
    |
    |-- ui/                         # Interface de usuário e renderização visual
    |   |-- MenuManager.h/.cpp      # Splash screen, carrossel de jogos, configurações e HUD
    |   |-- TextRenderer.h/.cpp     # Desenho de texto UTF-8 com acentos na fonte pixelada
    |   |-- Utf8Text.h/.cpp         # Funções puras de decodificação UTF-8 e decomposição de acentos
    |
    |-- localization/               # Internacionalização e traduções
        |-- Localization.h/.cpp     # Seleção de idioma e busca de textos (tr())
        |-- i18n_strings.h          # Tabela única com todos os textos em todos os idiomas
```

## 🚀 Como Compilar e Gravar

### Opção 1: Linux Nativo (`arduino-cli`)
Ideal para computadores com Linux nativo.

1. **Compilar**:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 firmware/LoverBoy
   ```
2. **Gravar**:
   ```bash
   arduino-cli upload -p /dev/ttyACM0 --fqbn esp32:esp32:lilygo_t_display_s3 firmware/LoverBoy
   ```
   *(Ajuste `-p` com a porta serial identificada da sua placa).*

### Opção 2: Windows Nativo (Arduino IDE)
Ideal para desenvolvedores no Windows que preferem ambiente visual com interface gráfica.

1. **Suporte à Placa**: Instale o suporte a placas **ESP32** pelo Gerenciador de Placas da Arduino IDE.
2. **Bibliotecas**: Instale a biblioteca **TFT_eSPI** de Bodmer.
3. **Configuração da Tela**: No arquivo `TFT_eSPI/User_Setup_Select.h`, descomente `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`.
4. **Compilar e Gravar**:
   - Abra `firmware/LoverBoy/LoverBoy.ino`.
   - Selecione a placa **LilyGo T-Display-S3** e a porta `COMx` correspondente.
   - Clique em **Carregar** (*Upload*).

### Opção 3: Terminal WSL (`build_and_upload.sh`)
Ideal para quem desenvolve diretamente de dentro do terminal Linux do WSL no Windows.

1. **Compilação e Gravação em 1 Comando** (no terminal do WSL):
   ```bash
   ./build_and_upload.sh -p COM7
   ```
   *Parâmetros e flags customizáveis:*
   - `-p, --port <PORT>`: Porta serial no Windows (padrão: `COM7`).
   - `-b, --baud <BAUD>`: Velocidade de gravação (padrão: `921600`).
   - `-d, --dir <DIR>`: Caminho para a pasta do firmware (padrão: `firmware/LoverBoy`).
   - `-c, --cli <PATH>`: Caminho do `arduino-cli` no WSL (detectado automaticamente se omitido).
   - `-e, --esptool <PATH>`: Caminho do `esptool.exe` no Windows (detectado automaticamente se omitido).

## 👤 Autor

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
