# BioSmartCooler - Sistema de Monitoramento com WiFi

Este projeto implementa um sistema de monitoramento para o BioSmartCooler, utilizando um Raspberry Pi Pico W para coletar dados de sensores e enviá-los para um servidor web, que disponibiliza um dashboard para visualização em tempo real.

## Estrutura do Projeto

```
├── inc/                  # Arquivos de cabeçalho e bibliotecas
├── src/                  # Código fonte principal
├── utils/                # Módulos utilitários
│   ├── dashboard/        # Módulo de dashboard
│   ├── send-data-to-server/ # Módulo de envio de dados
│   └── wifi-connection/  # Módulo de conexão WiFi
├── server/               # Servidor web (Node.js/Express)
└── web/                  # Interface web (HTML/CSS/JS)
```

## Componentes Principais

1. **Módulo WiFi**: Gerencia a conexão WiFi do Pico W
2. **Módulo de Envio de Dados**: Envia dados dos sensores para o servidor
3. **Módulo de Dashboard**: Gerencia a interface entre o Pico W e o dashboard web
4. **Servidor Web**: Recebe dados do Pico W e disponibiliza endpoints para o dashboard
5. **Interface Web**: Dashboard para visualização dos dados em tempo real

## Requisitos

### Hardware
- Raspberry Pi Pico W
- Sensores (BMP280, BH1750, MPU6050)
- Display OLED
- Servo motor
- Botões e joystick

### Software
- Pico SDK
- Node.js e npm
- TypeScript

## Configuração

### Configuração do Servidor

1. Navegue até a pasta do servidor:
   ```
   cd server
   ```

2. Instale as dependências:
   ```
   npm install
   ```

3. Inicie o servidor:
   ```
   npm start
   ```
   ou
   ```
   npx ts-node server.ts
   ```

### Compilação do Firmware

1. Configure o ambiente de desenvolvimento do Pico SDK

2. Compile o projeto:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Carregue o firmware no Pico W

## Testes de Integração

Para testar a integração entre o Pico W, o servidor e o dashboard:

1. Certifique-se de que o servidor está configurado

2. Execute o script de testes:
   ```
   node run-tests.js
   ```

## Uso

1. Conecte o Pico W à rede WiFi configurada

2. Inicie o servidor web

3. Acesse o dashboard em um navegador:
   ```
   http://localhost:3000/dashboard.html
   ```

4. O dashboard exibirá os dados dos sensores em tempo real

## Funcionalidades

- Monitoramento de temperatura, pressão, luminosidade e aceleração
- Controle de abertura/fechamento da caixa
- Alertas para condições anormais
- Interface web responsiva

## Licença

Este projeto é licenciado sob a licença MIT.
