# 🎮 EspBoy ![Status](https://img.shields.io/badge/status-in%20development-yellow) ![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

[English version](./README.md)

![EspBoy logo](EspBoy_logo.png)

Um console de videogame portátil inspirado no clássico "Game Boy", construído do zero utilizando um LILYGO ESP32-S3, componentes eletrônicos básicos e muita programação! Este é um projeto educacional e pessoal, originalmente desenvolvido com os recursos e espaço maker do Colégio Técnico da UFMG (Coltec) e em constante evolução durante a minha graduação em Ciência da Computação (DCC/UFMG).

## 🕹️ Sobre o Projeto

O objetivo do EspBoy é criar uma plataforma modular e robusta de hardware e software, desenvolvendo habilidades práticas em:

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

O software do EspBoy foi projetado para ser modular, escalável e responsivo:

- **Jogos Inclusos:** Clássicos **Crossy Road**, **Flappy Bird**, **Frogger**, **Genius**, **Snake** e **Tetris** rodando a ~60 FPS com alta fluidez.
- **Renderização por Diferença (Delta/Zero-Flicker):** Pipeline gráfico otimizado para barramento SPI/paralelo que calcula apenas o que mudou entre os quadros, apagando e redesenhando estritamente os pixels em movimento para eliminar cintilações.
- **Sistema de Recordes Persistente:** Memória Flash integrada (Preferences/NVS) que salva o Top 3 de cada jogo, incluindo tela clássica estilo arcade para inserir 3 iniciais.
- **Configurações de Som Permanentes:** O estado de mudo persiste entre reinicializações, evitando o buzzer alto na inicialização.
- **Interface Multilíngue:** Inglês (padrão) e Português do Brasil, selecionáveis nas Configurações. Todo texto da tela fica em uma única tabela (`i18n_strings.h`), e as letras acentuadas são desenhadas sobre a fonte pixelada retrô pelo `TextRenderer`.
- **Menu de Configurações Dinâmico:** Interface no console para ajuste de volume, brilho da tela e reset de recordes.
- **Áudio Não-Bloqueante:** Fila de som baseada em máquina de estados por contagem de milissegundos, sem congelar a física ou lógica dos jogos.
- **Controles Responsivos (`InputManager`):** Gerenciador centralizado de entradas com detecção de borda de latência zero, qualificação de soltura contra cliques duplos (debounce), repetição automática (DAS/ARR) e bloqueio de vazamento entre telas.

## 📂 Estrutura do Código

O projeto segue uma organização estrita de responsabilidades:

```
EspBoy/
|
|-- [ Ponto de Entrada ]
|-- EspBoy.ino            # Inicialização do sistema e loop principal
|-- pins.h                # Mapeamento dos pinos de hardware
|
|-- [ Sistema Operacional (Gerenciadores) ]
|-- EspBoyCore.h/.cpp     # Orquestrador global e alternador de modos (Menu vs Jogo)
|-- InputManager.h/.cpp   # Leitura centralizada de botões, debounce e auto-repeat
|-- MenuManager.h/.cpp    # Gerencia Splash Screen, Carrossel de Jogos e Configurações
|-- BatteryManager.h/.cpp # Lógica matemática e ADC para leitura da bateria
|-- ConfigManager.h/.cpp  # Persistência centralizada de configurações na NVS
|-- Localization.h/.cpp   # Seleção de idioma e busca de textos (tr()).
|-- i18n_strings.h        # Tabela única com todos os textos em todos os idiomas.
|-- TextRenderer.h/.cpp   # Desenho de texto UTF-8 com acentos na fonte pixelada.
|-- Utf8Text.h/.cpp       # Funções puras de UTF-8 / decomposição de acentos.
|
|-- [ Motor de Jogos ]
|-- Game.h/.cpp           # Classe base abstrata com ciclo de vida (Template Method)
|
|-- [ Sistema de Áudio ]
|-- SoundManager.h/.cpp   # Gerenciador de fila de áudio não-bloqueante
|-- audio_assets.h        # Biblioteca de frequências e vetores de melodias
|
|-- [ Jogos Implementados ]
|-- CrossyRoadGame.h/.cpp # Lógica isolada do jogo Crossy Road
|-- FlappyBirdGame.h/.cpp # Lógica isolada do jogo Flappy Bird
|-- FroggerGame.h/.cpp    # Lógica isolada do jogo Frogger
|-- GeniusGame.h/.cpp     # Lógica isolada do jogo Genius
|-- SnakeGame.h/.cpp      # Lógica isolada do jogo Snake
|-- TetrisGame.h/.cpp     # Lógica isolada do jogo Tetris
```

## 🚀 Como Compilar e Gravar

### Opção A: Linha de Comando / Antigravity (Recomendado)

1. **Compilar com `arduino-cli`** (WSL / Linux):
   ```bash
   arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 firmware/EspBoy
   ```
2. **Gravação Direta** (PowerShell no Windows):
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\build_and_upload.ps1
   ```
   *(Opcional: utilize `-Port COMx` para especificar a porta COM desejada).*

### Opção B: Arduino IDE

1. Instale o suporte às placas **ESP32** no Gerenciador de Placas da Arduino IDE.
2. Instale a biblioteca **TFT_eSPI** de Bodmer.
3. No arquivo `TFT_eSPI/User_Setup_Select.h`, descomente `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`.
4. Abra o arquivo `firmware/EspBoy/EspBoy.ino`.
5. Selecione a placa **LilyGo T-Display-S3** e clique em Carregar.

## 👤 Autor

- **Vítor Paixão** - [https://github.com/paixonite](https://github.com/paixonite)
