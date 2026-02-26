# 🎮 EspBoy ![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow) ![Plataforma](https://img.shields.io/badge/plataforma-ESP32--S3-blue) ![Framework](https://img.shields.io/badge/framework-Arduino-cyan)

![EspBoy logo](EspBoy_logo.png)

Um console de jogos portátil, estilo "Game Boy", construído do zero utilizando um ESP32-S3 da LILYGO, componentes eletrônicos básicos e muita programação! Este é um projeto pessoal com fins educacionais, desenvolvido utilizando materiais e o espaço maker do **Colégio Técnico da UFMG (Coltec)**.

## 🕹️ Sobre o Projeto

O objetivo do EspBoy é construir uma plataforma de hardware e software robusta e modular, desenvolvendo na prática habilidades em:

- **Desenvolvimento de Hardware:** Montagem de circuitos, soldagem e integração de componentes.
- **Programação de Baixo Nível:** Interação direta com GPIOs, timers e periféricos do microcontrolador.
- **Arquitetura de Software:** Criação de um sistema organizado, com um "firmware principal" que gerencia "bibliotecas" de jogos independentes.
- **Gerenciamento de Energia:** Implementação de um sistema de bateria recarregável para portabilidade real.

## ⚙️ Hardware Utilizado

- **Placa de Desenvolvimento:** LILYGO T-Display ESP32-S3 (com display TFT de 1.9" 170x320 integrado).
- **Entradas:** 8 botões *push button* (Direcionais, Start, Select, A, B).
- **Áudio:** 1 Buzzer passivo para reprodução de melodias e efeitos sonoros.
- **Alimentação:** Bateria de LiPo 3.7V recarregável, gerenciada pelo módulo de carregamento USB-C TP4056 e um slide switch.

## ✨ Features do Firmware

O software do EspBoy foi projetado para ser escalável e eficiente.

- **👾 Jogos:** Atualmente, conta com os clássicos Snake e Flappy Bird. Planos futuros incluirão jogos como Tetris e Frogger.
- **🔋 Suporte a Bateria Recarregável:** O firmware inclui rotinas para habilitar e monitorar o nível da bateria, permitindo que o console seja jogado em qualquer lugar.
- **🧩 Arquitetura de Software Modular (POO):** O código é organizado com um arquivo `.ino` principal que atua como um "mini-sistema operacional", e cada jogo é sua própria classe (biblioteca). Isso facilita a adição de novos jogos sem alterar o código principal.
- **🎵 Gerenciador de Áudio Não-Bloqueante:** Um sistema de som que toca melodias e efeitos sonoros em segundo plano, sem usar `delay()` e sem travar a lógica do jogo.
- **🕹️ Controles Responsivos:** A leitura dos botões é feita de forma não-bloqueante, com técnicas de *debounce* e detecção de borda (ação ao soltar o botão) para uma experiência de jogo precisa.

## 📂 Estrutura do Código

O projeto segue uma organização que separa as responsabilidades do firmware. 

```
EspBoy/
|
|-- [ Core & Hardware ]
|-- EspBoy.ino           # Firmware principal: máquina de estados, menu e inicialização.
|-- pins.h               # Centraliza a definição de todos os pinos físicos do ESP32.
|
|-- [ Motor de Jogos ]
|-- Game.h               # Classe base genérica (interface padrão).
|
|-- [ Sistema de Áudio ]
|-- SoundManager.h       # Classe para gerenciamento de áudio não-bloqueante via buzzer.
|-- SoundManager.cpp     # Implementação do controle de som.
|-- audio_assets.h       # Biblioteca de notas musicais e arrays de melodias.
|
|-- [ Jogos Implementados ]
|-- SnakeGame.h          # Cabeçalho do Snake.
|-- SnakeGame.cpp        # Implementação.
|-- FlappyBirdGame.h     # Cabeçalho do Flappy Bird.
|-- FlappyBirdGame.cpp   # Implementação.
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
