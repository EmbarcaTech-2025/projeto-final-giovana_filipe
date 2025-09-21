# Etapa 3 – Prototipagem e Ajustes
 
**Autores:** Filipe Alves de Sousa e Giovana Ferreira Santos

**Curso:** Residência Tecnológica em Sistemas Embarcados

**Instituição:** EmbarcaTech - HBr

Brasília, agosto de 2025

---
## 🎥 Fotos e Vídeos

Link:  https://youtu.be/xI2SOimQrlM

Fotos:

- Caixa de Transporte BioCooler
 <img width="400" height="600" alt="Caixa de Transporte BitCooler" src="https://github.com/user-attachments/assets/bf89ff4a-ce17-4172-bf99-c781dab7c1e3" />
 
- DASHBOARD
<img width="600" height="600" alt="Captura de tela 2025-08-29 222435" src="https://github.com/user-attachments/assets/727866f3-f74a-490d-901b-2291a9d7626a" />



## Desafios Encontrados Durante o Desenvolvimento

O desenvolvimento do projeto da caixa inteligente apresentou desafios significativos, principalmente relacionados à integração e estabilidade da comunicação entre o microcontrolador Pico W e o dashboard web.

- **Valores Nulos nos Dados dos Sensores:**:Inicialmente, os dados de temperatura, pressão e aceleração não estavam sendo atualizados no dashboard, aparecendo como zero ou com valores incorretos. A depuração revelou que os valores lidos dos sensores não estavam sendo corretamente atribuídos às variáveis que seriam enviadas.

- **Problemas de Comunicação entre Pico W e Servidor:**: O Pico W estava lendo e tentando enviar dados válidos, mas as atualizações no dashboard paravam após um curto período. A análise dos logs de depuração mostrou que o Pico W estava reportando um "Erro ao enviar dados".

- **Falha no Processamento da Requisição POST**: A requisição do tipo POST enviada pelo Pico W para o servidor não era processada corretamente. A variável que armazena os dados mais recentes no servidor não era atualizada, fazendo com que o dashboard ficasse "congelado" nos últimos valores recebidos.
  
- **Aceleração Ausente na Visualização**: Embora os valores de aceleração bruta (eixos X, Y e Z) fossem enviados, a magnitude total da aceleração não era calculada e exibida no dashboard. A interface esperava essa informação para apresentar um valor unificado.


## Melhorias Planejadas

Para garantir a confiabilidade e a usabilidade do sistema, a equipe planeja implementar as seguintes melhorias:

1. **Otimização do Envio de Dados**: A requisição POST será otimizada para ser mais robusta, incluindo o envio da magnitude total da aceleração como um campo separado. Isso garantirá que a informação seja exibida corretamente no dashboard.

2. **Lógica de Reconexão e Retry**: No firmware do Pico W, será adicionada uma rotina de verificação e reconexão automática em caso de falha de comunicação. Isso garantirá que, se a conexão Wi-Fi for perdida, o dispositivo tentará restabelecê-la para continuar enviando os dados sem interrupções.

3. **Aprimoramento da Interface de Monitoramento:**:  O dashboard será melhorado com a inclusão de um monitor de status de conexão. Ele alertará o usuário sobre a qualidade da rede e a última atualização recebida, proporcionando maior transparência sobre o estado do sistema.

