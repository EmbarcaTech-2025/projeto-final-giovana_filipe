# BioSmartCooler - Sistema de Transporte de Órgãos

## 📋 Visão Geral

O **BioSmartCooler** é um sistema eletrônico embarcado inovador projetado para o transporte seguro de órgãos humanos para transplante. O sistema integra múltiplos sensores, atuadores e interfaces para garantir o monitoramento contínuo e controle das condições ambientais críticas durante o transporte.

### 🎯 Objetivo Principal
Garantir que as condições críticas sejam mantidas durante todo o processo de transporte de órgãos, contribuindo significativamente para o sucesso dos transplantes e, consequentemente, para salvar vidas.

---

## 🏗️ Arquitetura do Sistema

### **Componentes Principais**

```
┌─────────────────┐    WiFi    ┌─────────────────┐
│   Smartphone    │◄──────────►│  BioSmartCooler │
│   / Computador  │            │   (Pico W)      │
└─────────────────┘            └─────────────────┘
         │                              │
         │                              │
         ▼                              ▼
┌─────────────────┐            ┌─────────────────┐
│   Dashboard     │            │   Sensores      │
│   Web Interface │            │   - BMP280      │
│                 │            │   - BH1750      │
│   Funcionalidades│           │   - MPU6050     │
│   - Monitoramento│           │   - DHT11       │
│   - Controle    │            │   - MicroSD     │
│   - Download    │            │   - Motor       │
└─────────────────┘            └─────────────────┘
```


### Componentes Principais

#### 1. **Placa BitDogLab (Raspberry Pi Pico)**
- **Processador**: Dual-core ARM Cortex-M0+ @ 133 MHz
- **Memória**: 264KB SRAM, 2MB Flash
- **Interface**: Display OLED, Botões, Joystick, LED RGB, Buzzer
- **Comunicação**: I2C, SPI, GPIO, PWM, ADC

#### 2. **Sensores Ambientais**
- **BMP280**: Temperatura (-40°C a +85°C) e Pressão (300-1100 hPa)
- **BH1750**: Luminosidade (1-65535 lux)
- **MPU6050**: Acelerômetro (±2g a ±16g) e Giroscópio (±250°/s a ±2000°/s)
- **DHT11**: Umidade (20-90% RH) e Temperatura (0-50°C)

#### 3. **Sistema de Segurança**
- **Teclado Matricial 4x4**: Interface de acesso
- **Motor de Passo**: Controle de travamento da tampa
- **Sistema de Alarme**: LED RGB + Buzzer

#### 4. **Sistema de Refrigeração**
- **Ventoinha**: Dissipação de calor dos componentes
- **Ponte H**: Controle de velocidade da ventoinha
- **Algoritmo PID**: Controle automático de temperatura

#### 5. **Armazenamento e Logging**
- **MicroSD**: Registro de dados em formato CSV/Excel
- **Frequência**: Amostragem a cada 5 segundos
- **Dados**: Timestamp, Temperatura, Pressão, Luminosidade, Aceleração, Umidade

---

## 📊 Diagramas do Sistema

### Diagrama de Blocos Funcional

<img width="8992" height="1092" alt="Diagrama de Blocos Funcional" src="https://github.com/user-attachments/assets/b47f25bd-b15f-4724-9c38-f186314cb247" />

**Explicação do Diagrama:**
Este diagrama mostra a arquitetura completa do sistema BioSmartCooler, dividida em blocos funcionais:

1. **Placa BitDogLab**: O núcleo do sistema com Raspberry Pi Pico, display OLED, botões, joystick, LED RGB e buzzer
2. **Sistema de Segurança**: Teclado matricial e motor de passo para controle de acesso
3. **Sensores Ambientais**: BMP280, BH1750, MPU6050 e DHT11 conectados via I2C
4. **Sistema de Refrigeração**: Ventoinha controlada por ponte H
5. **Armazenamento**: Cartão MicroSD para logging de dados
6. **Extensores**: Placas de conexão I2C e slots para expansão

As setas indicam o fluxo de dados e controle entre os componentes, com diferentes protocolos de comunicação (I2C, SPI, GPIO, PWM, ADC).

### Diagrama de Hardware

<img width="2160" height="1185" alt="Diagrama de Hardware" src="https://github.com/user-attachments/assets/c7e41816-d1a8-413a-ba61-52ac504ece4b" />

**Explicação do Diagrama:**
Este diagrama detalha as conexões físicas e protocolos de comunicação:

- **I2C**: Usado para comunicação com sensores (BMP280, BH1750, MPU6050) e display OLED
- **SPI**: Comunicação com cartão MicroSD para armazenamento de dados
- **GPIO**: Controle de teclado matricial, motor de passo e sensor DHT11
- **PWM**: Controle de LED RGB, buzzer e ventoinha
- **ADC**: Leitura do joystick analógico

Cada componente tem endereços específicos e frequências de operação otimizadas para sua função.

### Fluxograma Simplificado

<img width="1139" height="766" alt="Fluxograma Simplificado" src="https://github.com/user-attachments/assets/70c4a23c-87f8-489c-a999-6ca89ab6805d" />

**Explicação do Fluxograma:**
Este fluxograma representa o fluxo lógico do software do BioSmartCooler:

1. **Inicialização**: Sistema inicia e verifica hardware
2. **Verificação de Segurança**: Usuário deve inserir senha correta via teclado matricial
3. **Monitoramento Contínuo**: Sistema coleta dados dos sensores a cada 5 segundos
4. **Controle de Temperatura**: Verifica se temperatura está dentro dos limites (2°C a 8°C)
5. **Sistema de Alarme**: Ativa alertas visuais e sonoros em caso de condições críticas
6. **Logging de Dados**: Salva dados no MicroSD em formato CSV
7. **Interface do Usuário**: Permite navegação pelo menu para visualizar dados específicos

O fluxograma mostra como o sistema responde a diferentes condições e como mantém o monitoramento contínuo durante o transporte.

---

## 🔧 Protocolos de Comunicação

### I2C (Inter-Integrated Circuit)
- **Frequência**: 100kHz (sensores), 400kHz (display)
- **Endereços**:
  - BMP280: 0x76/0x77
  - BH1750: 0x23
  - MPU6050: 0x68
  - SSD1306: 0x3C

### SPI (Serial Peripheral Interface)
- **Frequência**: 25MHz
- **Aplicação**: Comunicação com MicroSD

### GPIO (General Purpose Input/Output)
- **Aplicações**: Teclado matricial, Motor de passo, DHT11
- **Tensão**: 3.3V

### PWM (Pulse Width Modulation)
- **Aplicações**: LED RGB, Buzzer, Ventoinha
- **Frequências**: 1kHz, 2kHz, 50Hz

### **outros protocolos de comunicação**

- **HTTP/HTTPS**: Interface web (porta 80)
- **WebSocket**: Dados em tempo real (porta 81)
- **WiFi**: Access Point "BioSmartCooler_AP"
- **JSON**: Formato de dados
---

## 🔄 Fluxo Operacional

### 1. **Inicialização**
- Verificação de hardware
- Calibração de sensores
- Tela de boas-vindas

### 2. **Sistema de Segurança**
- Verificação de senha via teclado matricial
- Máximo 3 tentativas
- Bloqueio por 5 minutos após falhas
- Liberação do motor de passo para destravar tampa

### 3. **Monitoramento Contínuo**
- Coleta de dados a cada 5 segundos
- Validação de integridade dos dados
- Registro no MicroSD
- Atualização do display

### 4. **Controle de Temperatura**
- Algoritmo PID para controle automático
- Limites operacionais: 2°C a 8°C
- Temperatura ideal: 4°C ± 1°C
- Ativação automática da ventoinha

### 5. **Sistema de Alarme**
- Alertas visuais (LED RGB)
- Alertas sonoros (Buzzer)
- Registro de eventos críticos
- Notificação de falhas

---

## 📱 Dashboard WiFi

### Funcionalidades do Dashboard

O sistema inclui um dashboard web acessível via WiFi que oferece:

#### **Monitoramento em Tempo Real**
- Exibição de dados dos sensores em tempo real
- Status da caixa (aberta/fechada) via sensor de luminosidade
- Contador de registros totais
- Indicadores visuais de saúde do sistema

#### **Controles Remotos**
- **Iniciar/Parar Gravação**: Controle remoto do logging de dados
- **Download CSV**: Download dos dados salvos no MicroSD
- **Destravar Caixa**: Acesso remoto via senha numérica
- **Diagnósticos**: Verificação de status do sistema

#### **Interface Web Responsiva**
- Design responsivo para diferentes dispositivos
- Tema médico com ícones relacionados à saúde
- Gráficos de temperatura em tempo real
- Notificações de alarme visuais

<img width="1920" height="1080" alt="Dashboard WiFi" src="https://github.com/user-attachments/assets/0048e6da-fa7c-4615-b667-446e28239463" />

### Configuração WiFi

#### **Modo Access Point (AP)**
- **SSID**: `BioSmartCooler_AP`
- **Senha**: `biosmart2024`
- **IP**: `192.168.4.1`
- **Porta**: `8080`

#### **Modo Station**
- Conecta a redes WiFi existentes
- Configurável via interface web
- Suporte a WPA2/WPA3

--- 

## 💾 Sistema de Logging

### Estrutura de Dados

O sistema registra dados em formato CSV com as seguintes colunas:
```
Timestamp, Temperatura, Pressão, Luminosidade, Aceleração_X, Aceleração_Y, Aceleração_Z, Umidade
```

### Arquivos Gerados

1. **`dados_sensores.csv`**: Dados principais dos sensores
2. **`eventos_sistema.csv`**: Log de eventos do sistema
3. **`alarmes.csv`**: Registro de alarmes e condições críticas

### Frequência de Amostragem
- **Padrão**: A cada 5 segundos
- **Configurável**: Via dashboard web
- **Backup**: Criação automática de backups

---

## 🔒 Sistema de Segurança

### Algoritmo de Verificação de Senha

```pseudocode
ALGORITMO VerificarSenha
    ENTRADA: senha_digitada, senha_correta
    VARIÁVEIS: tentativas = 0, max_tentativas = 3
    
    ENQUANTO tentativas < max_tentativas FAÇA
        SE senha_digitada == senha_correta ENTÃO
            RETORNAR VERDADEIRO
        SENÃO
            tentativas = tentativas + 1
            ExibirErro("Senha incorreta")
            SE tentativas >= max_tentativas ENTÃO
                BloquearSistema(300) // 5 minutos
            FIM SE
        FIM SE
    FIM ENQUANTO
    
    RETORNAR FALSO
FIM ALGORITMO
```

### Controle de Acesso
- **Senha Padrão**: `1234`
- **Tentativas Máximas**: 3
- **Tempo de Bloqueio**: 5 minutos
- **Log de Tentativas**: Registro de todas as tentativas de acesso

---

## 🌡️ Controle de Temperatura

### Algoritmo PID

```pseudocode
ALGORITMO ControleTemperatura
    ENTRADA: temperatura_atual, temperatura_desejada
    VARIÁVEIS: erro, erro_anterior, integral, derivativo
    CONSTANTES: Kp = 2.0, Ki = 0.1, Kd = 0.05
    
    erro = temperatura_desejada - temperatura_atual
    integral = integral + erro
    derivativo = erro - erro_anterior
    
    saida_pid = Kp * erro + Ki * integral + Kd * derivativo
    
    SE saida_pid > 0 ENTÃO
        AtivarVentoinha(saida_pid)
    SENÃO
        DesativarVentoinha()
    FIM SE
    
    erro_anterior = erro
FIM ALGORITMO
```

### Limites Operacionais
- **Temperatura Ideal**: 4°C ± 1°C
- **Limite Superior**: 8°C
- **Limite Inferior**: 2°C
- **Alarme Crítico**: < 0°C ou > 10°C

---

## 📋 Especificações Técnicas

### Condições Operacionais
- **Temperatura de Operação**: 0°C a 50°C
- **Umidade**: 10% a 90% RH
- **Tensão de Alimentação**: 5V DC
- **Consumo de Energia**: < 2W

### Performance
- **Tempo de Resposta**: < 100ms
- **Precisão de Temperatura**: ±0.1°C
- **Precisão de Umidade**: ±2% RH
- **Autonomia**: 8 horas (com baterias)

### Segurança
- **Proteção contra Acesso Não Autorizado**
- **Log de Eventos Completo**
- **Backup de Dados**
- **Indicadores de Status**

---

## 🏥 Aplicações

### Transporte de Órgãos
- **Coração**: 4-6 horas de viabilidade
- **Fígado**: 12-24 horas de viabilidade
- **Rim**: 24-48 horas de viabilidade
- **Pulmão**: 4-6 horas de viabilidade

### Monitoramento de Condições
- **Temperatura**: Crítica para preservação
- **Umidade**: Previne desidratação
- **Vibração**: Indica condições de transporte
- **Luminosidade**: Monitora exposição

---

## 🚀 Vantagens do Sistema

### 1. **Confiabilidade**
- Hardware robusto e testado
- Múltiplos sensores para redundância
- Sistema de backup de dados

### 2. **Segurança**
- Controle de acesso via senha
- Log completo de eventos
- Sistema de alarme integrado

### 3. **Facilidade de Uso**
- Interface intuitiva
- Display OLED informativo
- Controles simples

### 4. **Portabilidade**
- Design compacto
- Baixo consumo de energia
- Fácil transporte

### 5. **Monitoramento**
- Dados em tempo real
- Registro automático
- Análise posterior

---

## 📁 Estrutura do Projeto

```
projeto-final-giovana_filipe/
├── Project/
│   ├── CMakeLists.txt
│   ├── inc/
│   │   ├── bmp280.h
│   │   ├── bmp280.c
│   │   ├── mpu6050_i2c.h
│   │   ├── mpu6050_i2c.c
│   │   ├── luminosidade.h
│   │   ├── ssd1306.h
│   │   ├── ssd1306_i2c.h
│   │   ├── ssd1306_i2c.c
│   │   ├── microsd.h
│   │   └── wifi_dashboard.h
│   ├── src/
│   │   ├── main.c
│   │   ├── acelerometro.c
│   │   ├── luminosidade.c
│   │   ├── temperatura.c
│   │   ├── microsd.c
│   │   └── wifi_dashboard.c
│   └── web/
│       └── dashboard.html
├── Etapa_1/
├── Etapa_2/
└── README.md
```

---

## 🔧 Compilação e Instalação

### Pré-requisitos
- Raspberry Pi Pico SDK
- CMake 3.13+
- Compilador ARM GCC
- Ninja Build System

### Compilação
```bash
cd Project
mkdir build
cd build
cmake ..
ninja
```

### Flashing
```bash
cp BioSmartCooler.uf2 /media/RPI-RP2/
```

---

## 📞 Suporte

Para dúvidas ou problemas com o sistema, consulte:
- **Documentação**: Este README e arquivos de documentação
- **Issues**: Abra uma issue no repositório
- **Contato**: Equipe de desenvolvimento

---

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

---

## 👥 Autores

- **Giovana** - Desenvolvimento inicial
- **Filipe** - Integração e otimizações

---

*BioSmartCooler - Salvando vidas através da tecnologia* 