// Script para iniciar o servidor e executar os testes de integração

const { spawn } = require('child_process');
const path = require('path');

// Configurações
const SERVER_PATH = path.join(__dirname, 'server', 'server.ts');
const TEST_PATH = path.join(__dirname, 'test-integration.js');

console.log('🚀 Iniciando servidor para testes...');

// Iniciar o servidor usando ts-node
const server = spawn('npx', ['ts-node', SERVER_PATH], {
    stdio: 'pipe',
    shell: true
});

// Capturar saída do servidor
server.stdout.on('data', (data) => {
    console.log(`[Servidor] ${data.toString().trim()}`);
    
    // Quando o servidor estiver pronto, executar os testes
    if (data.toString().includes('Servidor rodando')) {
        console.log('\n🧪 Servidor iniciado, executando testes de integração...');
        
        // Aguardar um pouco para garantir que o servidor está pronto
        setTimeout(() => {
            const test = spawn('node', [TEST_PATH], {
                stdio: 'inherit',
                shell: true
            });
            
            test.on('close', (code) => {
                console.log(`\n🏁 Testes concluídos com código de saída: ${code}`);
                
                // Encerrar o servidor após os testes
                console.log('🛑 Encerrando servidor...');
                server.kill();
                process.exit(code);
            });
        }, 2000);
    }
});

server.stderr.on('data', (data) => {
    console.error(`[Servidor ERROR] ${data.toString().trim()}`);
});

server.on('close', (code) => {
    console.log(`Servidor encerrado com código de saída: ${code}`);
});

// Lidar com sinais de encerramento
process.on('SIGINT', () => {
    console.log('\n🛑 Interrompendo testes e encerrando servidor...');
    server.kill();
    process.exit(0);
});