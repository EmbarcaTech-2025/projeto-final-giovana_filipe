// Script para configurar o ambiente de desenvolvimento

const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

// Cores para o console
const colors = {
    reset: '\x1b[0m',
    green: '\x1b[32m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    red: '\x1b[31m'
};

// Função para executar comandos com feedback visual
function runCommand(command, message) {
    console.log(`${colors.blue}> ${message}...${colors.reset}`);
    try {
        execSync(command, { stdio: 'inherit' });
        console.log(`${colors.green}✓ Concluído!${colors.reset}\n`);
        return true;
    } catch (error) {
        console.error(`${colors.red}✗ Falha: ${error.message}${colors.reset}\n`);
        return false;
    }
}

// Função principal
function setup() {
    console.log(`${colors.green}=== Configuração do BioSmartCooler ===${colors.reset}\n`);
    
    // Verificar se o Node.js está instalado
    try {
        const nodeVersion = execSync('node --version').toString().trim();
        console.log(`${colors.green}✓ Node.js ${nodeVersion} detectado${colors.reset}\n`);
    } catch (error) {
        console.error(`${colors.red}✗ Node.js não encontrado. Por favor, instale o Node.js antes de continuar.${colors.reset}\n`);
        process.exit(1);
    }
    
    // Instalar dependências do servidor
    console.log(`${colors.yellow}=== Instalando dependências do servidor ===${colors.reset}`);
    const serverPath = path.join(__dirname, 'server');
    
    if (fs.existsSync(serverPath)) {
        process.chdir(serverPath);
        if (!runCommand('npm install', 'Instalando dependências do servidor')) {
            console.error(`${colors.red}✗ Falha ao instalar dependências do servidor${colors.reset}`);
            process.exit(1);
        }
        process.chdir(__dirname);
    } else {
        console.error(`${colors.red}✗ Diretório do servidor não encontrado${colors.reset}`);
        process.exit(1);
    }
    
    // Instalar dependências globais para os scripts de teste
    console.log(`${colors.yellow}=== Instalando dependências para os scripts de teste ===${colors.reset}`);
    if (!runCommand('npm install -g node-fetch@2', 'Instalando node-fetch globalmente')) {
        console.log(`${colors.yellow}! Tentando instalar localmente...${colors.reset}`);
        runCommand('npm install node-fetch@2', 'Instalando node-fetch localmente');
    }
    
    console.log(`${colors.green}=== Configuração concluída com sucesso! ===${colors.reset}\n`);
    console.log(`Para iniciar o servidor: ${colors.blue}cd server && npm start${colors.reset}`);
    console.log(`Para executar os testes: ${colors.blue}cd server && npm test${colors.reset}`);
    console.log(`Para simular o Pico W: ${colors.blue}cd server && npm run simulate${colors.reset}\n`);
}

// Executar a configuração
setup();