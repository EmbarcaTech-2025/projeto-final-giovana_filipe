# BioSmartCooler: Recipiente Inteligente para Transporte de Órgãos

## 🎯 O Desafio

O sucesso de um transplante de órgão depende criticamente de uma logística segura e eficiente. Métodos tradicionais, como caixas térmicas com gelo, não oferecem monitoramento em tempo real, criando riscos relacionados a variações de temperatura, umidade e impactos que podem comprometer a viabilidade do órgão. Estima-se que a logística inadequada responda por 5% a 10% das doações não concretizadas, e cerca de 30% dos órgãos são perdidos por falta de rapidez no transporte.

## 💡 A Solução: BioSmartCooler

O BioSmartCooler é um recipiente térmico inteligente que utiliza um sistema embarcado e conceitos de IoT para modernizar o transporte de órgãos.

* **Monitoramento Contínuo:** O sistema monitora em tempo real parâmetros vitais como temperatura, umidade, pressão, luminosidade e aceleração (impactos).
* **Alertas Imediatos:** Em caso de qualquer anomalia, o sistema emite alertas sonoros (buzzer) e visuais (LED RGB) para notificar o operador.
* **Proatividade e Segurança:** A solução garante uma camada extra de segurança, aumentando a taxa de sucesso em transplantes ao preservar a integridade do órgão.

## 🛠️ Diagrama de Hardware

O sistema é centralizado na placa **BitDogLab Board** e se comunica com diversos módulos, sensores e atuadores para garantir seu funcionamento.

* **Processador Central:** BitDogLab Board com microcontrolador RP2040.
* **Sensores:**
    * **Temperatura e Pressão:** BMP280
    * **Temperatura e Umidade:** AHT10
    * **Luminosidade:** BH1750
    * **Aceleração/Giroscópio:** MPU 6050
* **Atuadores:**
    * **Servo Motor SG90:** Controla a trava da tampa.
    * **Display OLED:** Exibe informações para o operador local.
    * **Buzzer e LED RGB:** Para alertas sonoros e visuais.
* **Armazenamento:** Os dados são salvos em um cartão MicroSD (Datalogg).
* **Interfaces de Usuário:**
    * **Local:** Teclado matricial, joystick e botões.
    * **Remota:** Dashboard web acessado via Wi-Fi.

## 💻 Software

O software do projeto é dividido em três componentes principais:

1.  **`Main.c` (Firmware do Sistema Embarcado):**
    * Lê continuamente os dados dos sensores.
    * Controla os atuadores (ventoinha, trava, LEDs, buzzer).
    * Gerencia a segurança, exigindo senha para destravar a caixa e registrando todos os eventos.
2.  **`Node.js` (Backend/Servidor):**
    * Responsável pela comunicação entre o sistema embarcado e o dashboard.
3.  **`dashboard.html` (Frontend):**
    * Interface web para monitoramento e controle remoto.

## 👤 Interação do Usuário

O sistema foi projetado para dois perfis de operadores:

### Operador Local
* Acessa o sistema fisicamente.
* Usa o teclado para digitar a senha e destravar a tampa.
* Acompanha os dados em tempo real pelo display OLED, navegando com botões e joystick.
* Recebe alertas visuais diretamente pelo LED RGB.

### Operador Remoto
* Acessa o dashboard via Wi-Fi em um navegador.
* Acompanha os dados em tempo real, com atualizações a cada 100ms (10 Hz).
* Controla a trava da tampa remotamente com login e senha.
* Importa e exporta planilhas com o histórico de dados salvos.
* Recebe notificações de alerta pelo sistema.

## 🚀 Próximos Passos e Melhorias

O projeto está em evolução e as próximas melhorias planejadas incluem:

* **Estrutura Física:** Tornar a caixa mais compacta, adicionar uma rede de proteção interna e um sistema de amortecimento contra impactos.
* **Hardware:** Reduzir o número de fios/jumpers e adicionar um módulo de geolocalização (GPS) para rastreamento.
* **Comunicação:** Implementar um protocolo de comunicação via satélite (Iridium) para garantir a conectividade durante viagens aéreas.
* **Software:**
    * Corrigir a funcionalidade dos botões no dashboard.
    * Ajustar os limiares de alertas (temperatura, tempo).
    * Melhorar a organização dos logs para exportação em `.csv`.
    * Migrar a comunicação de Pooling HTTP para WebSocket para maior eficiência.
* **Lógica:** Automatizar o travamento da tampa, acionando o servo motor assim que a luminosidade interna for zero (tampa fechada).

## 👥 Autores

* Filipe Alves de Sousa
* Giovana Ferreira Santos
