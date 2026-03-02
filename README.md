# 🎮 EspBoy ![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow) ![Plataforma](https://img.shields.io/badge/plataforma-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

![EspBoy logo](EspBoy_logo.png)

Um console de jogos portátil, estilo "Game Boy", construído do zero utilizando um ESP32-S3 da LILYGO, componentes eletrônicos básicos e muita programação! Este é um projeto pessoal com fins educacionais, desenvolvido originalmente com os materiais e o espaço maker do Colégio Técnico da UFMG (Coltec) e em constante evolução durante a graduação em Ciência da Computação (DCC/UFMG).

## 🕹️ Sobre o Projeto

O objetivo do EspBoy é construir uma plataforma de hardware e software robusta e modular, desenvolvendo na prática habilidades em:

- **Desenvolvimento de Hardware:** Montagem de circuitos, soldagem e integração de componentes.
- **Programação de Baixo Nível:** Interação direta com GPIOs, timers (PWM) e periféricos do microcontrolador.
- **Arquitetura de Software:** Criação de um sistema operacional organizado por responsabilidades, com classes abstratas ditando o ciclo de vida dos jogos através do padrão Template Method.
- **Gerenciamento de Energia:** Implementação de um sistema de bateria recarregável para portabilidade real.

## ⚙️ Hardware Utilizado

- **Placa de Desenvolvimento:** LILYGO T-Display ESP32-S3 (com display TFT de 1.9" 170x320 integrado).
- **Entradas:** 8 botões *push button* (Direcionais, Start, Select, A, B).
- **Áudio:** 1 Buzzer passivo para reprodução de melodias e efeitos sonoros.
- **Alimentação:** Bateria de LiPo 3.7V recarregável, gerenciada pelo módulo de carregamento USB-C TP4056 e um slide switch.

## ✨ Features do Firmware

O software do EspBoy foi projetado para ser modular, escalável e responsivo.

- 👾 Jogos Inclusos: Atualmente, conta com os clássicos Snake e Flappy Bird, rodando de forma fluida a ~60 FPS. Planos futuros incluirão jogos como Tetris e Frogger.
- 🏆 Sistema de Highscores Persistente: O console possui uma memória Flash integrada que salva o Top 3 de cada jogo, incluindo uma tela de entrada de 3 iniciais no clássico estilo arcade.
- ⚙️ Menu de Configurações Dinâmico: Uma UI de sistema dedicada permite ajustar o volume, alterar o brilho da tela e resetar os recordes.
- 🧩 Arquitetura Orientada a Objetos (POO): O sistema é orquestrado por um Core (EspBoyCore) que delega tarefas para gerenciadores específicos (MenuManager, BatteryManager). Novos jogos herdam de uma superclasse Game que fornece a máquina de estados padrão e os métodos template.
- 🎵 Áudio Não-Bloqueante: Um sistema de som que toca melodias e efeitos sonoros em segundo plano através de uma máquina de estados, sem travar a física ou a lógica dos jogos.
- 🕹️ Controles Responsívos: A leitura dos botões utiliza técnicas de detecção de borda, debounce e prevenção de vazamento de inputs.

## 📂 Estrutura do Código

O projeto segue uma organização que separa as responsabilidades do firmware. 

```
EspBoy/
|
|-- [ Ponto de Entrada ]
|-- EspBoy.ino            # Apenas inicializa o sistema operacional do console.
|-- pins.h                # Centraliza o mapeamento de hardware do ESP32.
|
|-- [ Sistema Operacional (Managers) ]
|-- EspBoyCore.h/.cpp     # Orquestrador de estado global.
|-- MenuManager.h/.cpp    # Gerencia Splash Screen, Carrossel de Jogos e Settings.
|-- BatteryManager.h/.cpp # Lógica matemática e ADC de leitura da bateria.
|
|-- [ Motor de Jogos (Engine) ]
|-- Game.h/.cpp           # Classe abstrata base com Template Methods.
|
|-- [ Sistema de Áudio ]
|-- SoundManager.h/.cpp   # Gerenciador de fila de áudio não-bloqueante.
|-- audio_assets.h        # Biblioteca de frequências e arrays de melodias.
|
|-- [ Jogos Implementados ]
|-- SnakeGame.h/.cpp      # Lógica isolada do Snake.
|-- FlappyBirdGame.h/.cpp # Lógica isolada do Flappy Bird.
```

## 🚀 Como Compilar e Usar

1.  **Ambiente:** O projeto foi desenvolvido usando a **Arduino IDE**.
2.  **Hardware:** Monte o circuito conforme as definições da pcb.
3.  **Bibliotecas:**
    - Instale o suporte para placas **ESP32** na Arduino IDE.
    - Instale a biblioteca **TFT_eSPI** de Bodmer. (Pode ser necessário configurar o `User_Setup.h` da biblioteca para a placa LILYGO T-Display S3, embora muitas versões recentes a detectem automaticamente).
4.  **Compilação:**
    - Clone este repositório
    - Abra o arquivo `EspBoy.ino` na Arduino IDE
    - Selecione a placa correta (LILYGO T-Display S3)
    - Faça o upload.

## 👤 Autor

- **Vítor Paixão** - https://github.com/Paixonite
