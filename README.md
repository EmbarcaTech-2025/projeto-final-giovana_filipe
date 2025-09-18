# BioSmartCooler: Recipiente Inteligente para Transporte de Órgãos

Bem-vindo ao repositório oficial do projeto **BioSmartCooler**! Este espaço documenta a jornada de desenvolvimento de um recipiente térmico inteligente, projetado para o transporte de órgãos através da tecnologia de sistemas embarcados e IoT.

Nosso objetivo é criar uma solução que monitore em tempo real as condições vitais para a preservação de órgãos, oferecendo segurança, rastreabilidade e contribuindo para aumentar as taxas de sucesso dos transplantes.

## 📂 Estrutura do Projeto

Este repositório está organizado em etapas, cada uma representando uma fase crucial do desenvolvimento, desde a concepção inicial até os ajustes finos de comunicação e prototipagem. Navegue pelas pastas para encontrar a documentação, os códigos e os diagramas específicos de cada fase.

---

### 📄 Etapa 1: Concepção e Definição de Requisitos (BioCooler)

Nesta fase inicial, o foco foi definir o problema, estabelecer os objetivos e especificar os requisitos do sistema. O projeto, nomeado "BioSmartCooler", nasceu como um protótipo para modernizar o transporte de órgãos.

**O que você vai encontrar:**
* **Documentação Conceitual:** O documento base (`BioSmartCooler_Etapa1.pdf`) que define o problema do transporte de órgãos e o objetivo da solução.
* **Requisitos Funcionais e Não Funcionais:** Uma lista detalhada de tudo que o sistema deveria fazer (ex: medir temperatura, detectar quedas) e as condições de operação (ex: autonomia de 4 horas).
* **Lista de Hardware Inicial:** A primeira seleção de componentes, centralizada na placa BitDogLab com Raspberry Pi Pico W, e os sensores essenciais (temperatura, umidade, impacto, etc.).

Esta etapa foi o alicerce teórico do projeto, definindo o escopo e a direção a ser seguida.

### ⚙️ Etapa 2: Arquitetura Avançada e Interface Dupla (BioSmartCooler)

A segunda etapa marca a evolução do protótipo para um sistema mais complexo e robusto. O foco foi a construção de uma arquitetura integrada e a implementação de interfaces de usuário completas.

**O que você vai encontrar:**
* **Arquitetura Detalhada:** Documentação sobre a arquitetura multi-protocolo (I2C, SPI, PWM, Wi-Fi) e o armazenamento redundante (MicroSD e nuvem).
* **Diagramas de Sistema:** Diagramas de hardware e fluxogramas que ilustram a lógica de funcionamento e a interação entre os componentes.
* **Implementação da Interface Dupla:** O desenvolvimento de dois modos de operação:
    * **Operador Local:** Com interação física via display OLED, teclado matricial e joystick.
    * **Operador Remoto:** Através de um dashboard web funcional, acessado via Wi-Fi.
* **Firmware e Software:** O código-fonte do firmware (`Main.c`), do servidor (`Node.js`) e do frontend (`dashboard.html`).

Esta fase transformou o conceito em um produto funcional com interfaces de controle bem definidas.

### 🔧 Etapa 3: Prototipagem, Depuração e Ajustes

Esta etapa foi dedicada à montagem do protótipo físico e à depuração (debugging) da comunicação entre o hardware e o software, um dos maiores desafios do projeto. O foco foi identificar e solucionar problemas de integração para garantir a estabilidade do sistema.

**O que você vai encontrar:**
* **Relatório de Desafios:** Documentação detalhada sobre os problemas encontrados, como a falha no envio de dados dos sensores para o dashboard, erros de requisição POST e a ausência de dados de aceleração na interface.
* **Planejamento de Melhorias:** As soluções propostas para os desafios, incluindo a otimização do envio de dados, a implementação de uma lógica de reconexão automática no firmware e melhorias na interface do dashboard para monitorar o status da conexão.
* **Fotos e Vídeos:** Registros visuais do protótipo montado e do dashboard em funcionamento.

Esta fase foi crucial para tornar o sistema confiável e identificar os próximos passos para o refinamento do software.

### 🚀 Etapa 4: Melhorias Futuras e Funcionalidades Avançadas

A última etapa planejada foca em evoluir o protótipo com base nos aprendizados da fase de testes, adicionando funcionalidades avançadas e melhorando o design físico para torná-lo mais próximo de um produto final.

**O que está planejado para esta fase:**
* **Otimização de Hardware:** Melhorar a estrutura física para ser mais compacta, adicionar proteções internas e um sistema de amortecimento contra impactos.
* **Geolocalização:** Adicionar um módulo GPS para rastreamento em tempo real.
* **Comunicação Avançada:** Implementar um protocolo de comunicação via satélite (Iridium) para garantir conectividade em qualquer localidade, especialmente em transportes aéreos.
* **Refinamento de Software e Lógica:** Corrigir bugs remanescentes no dashboard, ajustar os limiares de alerta e automatizar o travamento da tampa com base no sensor de luminosidade.

---

Sinta-se à vontade para explorar as pastas de cada etapa para uma visão aprofundada do nosso progresso.

**Autores:**
* Giovana Ferreira Santos
* Filipe Alves de Sousa
