# Instruções para Execução do BioSmartCooler

## Requisitos

1. **Node.js**: Necessário para executar o servidor
   - Baixe e instale o Node.js da [página oficial](https://nodejs.org/)
   - Versão recomendada: 18.x ou superior

2. **Configuração do ESP32/Pico W**
   - Certifique-se de que o firmware está atualizado
   - Configure o Wi-Fi conforme as instruções abaixo

## Configuração do Servidor

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

4. O servidor estará disponível em:
   ```
   http://10.63.72.222:3000/
   ```

## Configuração do Wi-Fi

O sistema está configurado para usar os seguintes endereços IP:

- **Raspberry Pi**: 10.63.72.222 (Servidor)
- **Router**: 100.64.175.141

## Funcionalidades Implementadas

1. **Contador de tempo para entrega com alerta sonoro**
   - Configure o tempo através do dashboard
   - Receba alertas sonoros quando o tempo estiver próximo do fim

2. **Dashboard com informações em tempo real**
   - Visualize temperatura, pressão e luminosidade
   - Veja o status da conexão
   - Data e hora atualizadas em tempo real

3. **Controle da caixa**
   - Opções para travar/destravar a caixa
   - Monitoramento de movimentos bruscos
   - Alertas para temperatura alta

## Acesso ao Dashboard

Para acessar o dashboard, abra o navegador e acesse:

```
http://10.63.72.222:3000/dashboard.html
```

## Solução de Problemas

1. **Servidor não inicia**
   - Verifique se o Node.js está instalado corretamente
   - Verifique se todas as dependências foram instaladas

2. **Dashboard mostra "Desconectado"**
   - Verifique se o servidor está em execução
   - Verifique se o IP está configurado corretamente

3. **Comandos não são processados**
   - Verifique a conexão entre o servidor e o ESP32/Pico W
   - Verifique os logs do servidor para identificar possíveis erros