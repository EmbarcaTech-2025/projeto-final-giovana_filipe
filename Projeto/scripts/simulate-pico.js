// Script para simular o envio de dados do Pico W para o servidor

const fetch = require('node-fetch');

// Configurações
const SERVER_URL = 'http://localhost:3000';
const SEND_INTERVAL = 3000; // 3 segundos

// Função para gerar dados aleatórios dentro de um intervalo
function randomInRange(min, max) {
    return Math.random() * (max - min) + min;
}

// Função para gerar dados simulados dos sensores
function generateSensorData() {
    // Simular variação de temperatura entre 20°C e 30°C
    const temperatura = parseFloat(randomInRange(20, 30).toFixed(1));
    
    // Simular variação de pressão atmosférica entre 1000 e 1025 hPa
    const pressao = parseFloat(randomInRange(1000, 1025).toFixed(2));
    
    // Simular variação de luminosidade entre 100 e 1000 lux
    const luminosidade = Math.floor(randomInRange(100, 1000));
    
    // Simular dados do acelerômetro
    const aceleracao = {
        x: parseFloat(randomInRange(-1, 1).toFixed(2)),
        y: parseFloat(randomInRange(-1, 1).toFixed(2)),
        z: parseFloat(randomInRange(0, 1).toFixed(2))
    };
    
    // Simular estado da caixa (aberta ou fechada) com 20% de chance de estar aberta
    const caixaAberta = Math.random() < 0.2;
    
    return {
        temperatura,
        pressao,
        luminosidade,
        aceleracao,
        caixaAberta,
        timestamp: new Date()
    };
}

// Função para enviar dados para o servidor
async function sendData() {
    const data = generateSensorData();
    
    console.log('\n📤 Enviando dados:', JSON.stringify(data, null, 2));
    
    try {
        const response = await fetch(`${SERVER_URL}/sensores`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            throw new Error(`Erro na resposta: ${response.status}`);
        }
        
        const responseText = await response.text();
        console.log('✅ Dados enviados com sucesso:', responseText);
    } catch (error) {
        console.error('❌ Erro ao enviar dados:', error.message);
    }
}

// Função para verificar o status do servidor
async function checkServerStatus() {
    try {
        const response = await fetch(SERVER_URL);
        if (response.ok) {
            console.log('✅ Servidor está online');
            return true;
        } else {
            console.log(`❌ Servidor respondeu com status: ${response.status}`);
            return false;
        }
    } catch (error) {
        console.error('❌ Não foi possível conectar ao servidor:', error.message);
        return false;
    }
}

// Função principal
async function main() {
    console.log('🤖 Iniciando simulação do Pico W...');
    console.log(`🔗 Conectando ao servidor: ${SERVER_URL}`);
    
    // Verificar se o servidor está online
    const serverOnline = await checkServerStatus();
    
    if (!serverOnline) {
        console.log('⚠️ Servidor não está disponível. Verifique se o servidor está rodando.');
        console.log('ℹ️ Você pode iniciar o servidor com: npx ts-node server/server.ts');
        process.exit(1);
    }
    
    console.log(`🔄 Enviando dados a cada ${SEND_INTERVAL/1000} segundos...`);
    console.log('ℹ️ Pressione Ctrl+C para encerrar a simulação');
    
    // Enviar dados imediatamente na primeira vez
    await sendData();
    
    // Configurar envio periódico
    setInterval(sendData, SEND_INTERVAL);
}

// Iniciar a simulação
main();

// Lidar com sinais de encerramento
process.on('SIGINT', () => {
    console.log('\n👋 Encerrando simulação do Pico W...');
    process.exit(0);
});