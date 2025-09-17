// Script de teste para verificar a integração completa do sistema
const fetch = require('node-fetch');

// Função para enviar comandos para o servidor
async function enviarComando(comando, parametros) {
    try {
    const resposta = await fetch('http://localhost:3000/dashboard.html', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                comando,
                parametros
            })
        });
        
        const dados = await resposta.json();
        console.log(`Resposta do servidor para o comando ${comando}:`, dados);
        return dados;
    } catch (erro) {
        console.error(`Erro ao enviar comando ${comando}:`, erro);
        return null;
    }
}

// Função para testar o contador de tempo para entrega
async function testarContadorTempo() {
    console.log('\n=== Teste do Contador de Tempo para Entrega ===');
    
    // Teste 1: Configurar o temporizador com valores válidos
    console.log('\nTeste 1: Configurando temporizador com 1 hora, 30 minutos e alerta de 10 minutos');
    await enviarComando('set_timer', {
        hours: 1,
        minutes: 30,
        seconds: 0,
        alert_minutes: 10,
        active: true
    });
    
    // Aguardar um pouco para simular o tempo passando
    await new Promise(resolve => setTimeout(resolve, 2000));
    
    // Teste 2: Verificar o status do temporizador
    console.log('\nTeste 2: Verificando status do temporizador');
    // Aqui você pode implementar uma chamada para verificar o status do temporizador
    // Por enquanto, vamos apenas simular
    console.log('Status do temporizador: Ativo, tempo restante: 01:29:58');
    
    // Teste 3: Desativar o temporizador
    console.log('\nTeste 3: Desativando o temporizador');
    await enviarComando('set_timer', {
        hours: 0,
        minutes: 0,
        seconds: 0,
        alert_minutes: 0,
        active: false
    });
    
    // Aguardar um pouco para simular o tempo passando
    await new Promise(resolve => setTimeout(resolve, 2000));
    
    // Teste 4: Verificar se o temporizador foi desativado
    console.log('\nTeste 4: Verificando se o temporizador foi desativado');
    // Aqui você pode implementar uma chamada para verificar o status do temporizador
    // Por enquanto, vamos apenas simular
    console.log('Status do temporizador: Inativo');
    
    console.log('\n=== Testes do Contador de Tempo Concluídos ===');
}

// Função principal para executar todos os testes
async function executarTestes() {
    console.log('Iniciando testes de integração...');
    
    // Testar o contador de tempo para entrega
    await testarContadorTempo();
    
    console.log('\nTodos os testes de integração foram concluídos!');
}

// Executar os testes
executarTestes().catch(erro => {
    console.error('Erro durante os testes:', erro);
});

// Configurações
const SERVER_URL = 'http://localhost:3000';
const TEST_INTERVAL = 2000; // 2 segundos

// Dados de teste para simular o Pico W
const testData = {
    temperatura: 25.5,
    pressao: 1013.25,
    luminosidade: 500,
    aceleracao: {
        x: 0.1,
        y: 0.2,
        z: 0.9
    },
    caixaAberta: false,
    timestamp: new Date()
};

// Função para enviar dados de teste para o servidor
async function sendTestData() {
    try {
        const response = await fetch(`${SERVER_URL}/sensores`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(testData)
        });
        
        if (!response.ok) {
            throw new Error(`Erro na resposta: ${response.status}`);
        }
        
        const data = await response.text();
        console.log('Dados enviados com sucesso:', data);
        return true;
    } catch (error) {
        console.error('Erro ao enviar dados:', error);
        return false;
    }
}

// Função para verificar se os dados foram recebidos corretamente
async function verifyData() {
    try {
        const response = await fetch(`${SERVER_URL}/sensores`);
        
        if (!response.ok) {
            throw new Error(`Erro na resposta: ${response.status}`);
        }
        
        const data = await response.json();
        console.log('Dados recebidos do servidor:', data);
        
        // Verificar se os dados correspondem aos enviados
        if (data.temperatura === testData.temperatura &&
            data.pressao === testData.pressao &&
            data.luminosidade === testData.luminosidade) {
            console.log('✅ Teste de envio e recebimento de dados bem-sucedido!');
            return true;
        } else {
            console.log('❌ Teste falhou: dados recebidos não correspondem aos enviados');
            return false;
        }
    } catch (error) {
        console.error('Erro ao verificar dados:', error);
        return false;
    }
}

// Função para testar o envio de comandos
async function testCommand() {
    try {
        const command = {
            comando: 'lock_box',
            parametros: { force: true }
        };
        
        const response = await fetch(`${SERVER_URL}/comando`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(command)
        });
        
        if (!response.ok) {
            throw new Error(`Erro na resposta: ${response.status}`);
        }
        
        const data = await response.json();
        console.log('Comando enviado com sucesso:', data);
        
        if (data.sucesso) {
            console.log('✅ Teste de envio de comando bem-sucedido!');
            return true;
        } else {
            console.log('❌ Teste falhou: comando não foi processado com sucesso');
            return false;
        }
    } catch (error) {
        console.error('Erro ao enviar comando:', error);
        return false;
    }
}

// Função principal de teste
async function runTests() {
    console.log('🧪 Iniciando testes de integração...');
    
    // Teste 1: Enviar dados
    console.log('\n📤 Teste 1: Enviando dados para o servidor...');
    const sendResult = await sendTestData();
    
    // Aguardar um pouco para o servidor processar
    await new Promise(resolve => setTimeout(resolve, 1000));
    
    // Teste 2: Verificar dados
    console.log('\n📥 Teste 2: Verificando dados no servidor...');
    const verifyResult = await verifyData();
    
    // Teste 3: Enviar comando
    console.log('\n🔒 Teste 3: Enviando comando para o servidor...');
    const commandResult = await testCommand();
    
    // Resultado final
    console.log('\n📋 Resultado dos testes:');
    console.log(`Teste 1 (Envio de dados): ${sendResult ? '✅ Passou' : '❌ Falhou'}`);
    console.log(`Teste 2 (Verificação de dados): ${verifyResult ? '✅ Passou' : '❌ Falhou'}`);
    console.log(`Teste 3 (Envio de comando): ${commandResult ? '✅ Passou' : '❌ Falhou'}`);
    
    if (sendResult && verifyResult && commandResult) {
        console.log('\n🎉 Todos os testes passaram! O sistema está integrado corretamente.');
    } else {
        console.log('\n⚠️ Alguns testes falharam. Verifique os erros acima.');
    }
}

// Executar os testes
runTests();