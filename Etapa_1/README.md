# BioCooler: Recipiente Inteligente para Transporte de Órgãos

## 📝 Descrição

O BioCooler é um protótipo de recipiente térmico inteligente projetado para modernizar e otimizar o transporte de órgãos para transplantes. Utilizando sistemas eletrônicos embarcados e conceitos de Internet das Coisas (IoT), o projeto visa solucionar as limitações dos métodos convencionais, que geralmente se baseiam em caixas térmicas com gelo e carecem de monitoramento em tempo real.

Este sistema monitora continuamente parâmetros críticos como temperatura, umidade, luminosidade e impacto, fornecendo alertas sonoros e visuais imediatos em caso de qualquer anomalia. Com isso, o BioCooler aumenta a segurança e a viabilidade dos órgãos, contribuindo para o sucesso dos transplantes.

## 🎯 Problema

O transporte de órgãos para transplantes exige um controle rigoroso de condições ambientais como temperatura e umidade, além de segurança contra impactos. O método tradicional com caixas térmicas e gelo não oferece controle inteligente ou registro em tempo real das condições durante o transporte, o que pode comprometer a viabilidade do órgão.

## 🚀 Objetivo da Solução

O objetivo é desenvolver um protótipo de recipiente térmico inteligente que simule o transporte de órgãos com as seguintes características:
* Monitoramento em tempo real de temperatura, umidade, luminosidade e impacto.
* Emissão de alertas sonoros e visuais em caso de anomalias.
* Armazenamento ou transmissão dos dados monitorados.

## ✨ Funcionalidades (Requisitos)

### Requisitos Funcionais
* **RF01:** Medir e exibir a temperatura interna do recipiente.
* **RF02:** Medir e exibir a umidade interna do recipiente.
* **RF03:** Detectar e registrar movimentos bruscos ou quedas.
* **RF04:** Detectar a abertura da tampa (mudança de luminosidade).
* **RF05:** Exibir os dados em um display visível do lado externo.
* **RF06:** Emitir alerta sonoro (buzzer) em situações críticas.
* **RF07:** Alimentação autônoma via bateria ou power bank.

### Requisitos Não Funcionais
* **RNF01:** O sistema deve operar por ao menos 4 horas sem recarga.
* **RNF02:** A interface deve ser de fácil leitura e operação.
* **RNF03:** Os sensores devem ter tempo de resposta rápido e precisão adequada.
* **RNF04:** Os componentes devem estar bem fixados e protegidos contra possíveis movimentos bruscos.
* **RNF05:** O recipiente deve manter isolamento térmico adequado (simulado).

## 🛠️ Hardware Utilizado

O núcleo do sistema é a placa **BitDogLab com Raspberry Pi Pico W**, que processa os dados dos sensores e gerencia os atuadores.

| Item | Quantidade | Descrição |
| --- | :---: | --- |
| Caixa térmica de isopor (8 a 12L) | 1 | Recipiente base com isolamento térmico. |
| Placa BitDogLab com Raspberry Pi Pico W | 1 | Microcontrolador com periféricos integrados. |
| Sensor de Temperatura e Pressão BMP280 | 1 | Sensor externo conectado via placa adaptadora I2C. |
| Sensor de Umidade e Temperatura AHT10 | 1 | Sensor externo conectado via placa adaptadora I2C. |
| Sensor de Luminosidade BH1750 | 1 | Sensor externo conectado via placa adaptadora I2C. |
| Acelerômetro e Giroscópio MPU6050 | 1 | Sensor externo conectado via placa adaptadora I2C. |
| Servo motor 9g SG90 | 1 | Atuador externo conectado via placa adaptadora I2C. |
| Teclado matricial 4x4 | 1 | Interface externa conectada via conector IDC direto. |
| Placa para SDCARD SPI | 1 | Módulo externo conectado via conector IDC direto. |
| Placa extensora I2C com 8 conectores XH | 1 | Permite conexão simultânea de até 7 sensores I2C. |
| Fonte de energia (power bank ou bateria Li-ion) | 1 | Alimentação portátil para o sistema. |

## 👥 Autores

* Giovana Ferreira Santos
* Filipe Alves de Sousa
