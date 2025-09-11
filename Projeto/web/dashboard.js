// Interatividade para o dashboard do BioCooler
// Comunicação apenas via polling HTTP (fetch), sem WebSocket

// Variáveis globais
let timerInterval = null; // Intervalo para o cronômetro
let cronometroAtivo = false;
let tempoDecorridoMs = 0;
let ultimoUpdateTimestamp = null;
let estimatedEndTime = null; // Tempo estimado para entrega
let alertMinutesThreshold = 10; // Minutos para alertar antes do fim do tempo
let currentPassword = ''; // Senha atual para destravar/travar a caixa

// Constantes para alertas
const TEMP_THRESHOLD = 30; // Temperatura máxima em °C
const ACCEL_THRESHOLD = 2.0; // Aceleração máxima em m/s²

// Inicialização quando o documento estiver pronto
document.addEventListener('DOMContentLoaded', function() {
    // Iniciar polling HTTP para buscar dados do servidor
    fetchSensorData();
    setInterval(fetchSensorData, 3000);

    // Atualizar data e hora atual
    updateCurrentDateTime();
    setInterval(updateCurrentDateTime, 1000);

    // Iniciar cronômetro local se necessário
    startCronometroInterval();

    // Verificar status inicial dos elementos
    updateUIStatus();
});

// Atualizar data e hora atual
function updateCurrentDateTime() {
    const now = new Date();
    const options = { 
        weekday: 'long', 
        year: 'numeric', 
        month: 'long', 
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
    };
    const dateTimeString = now.toLocaleDateString('pt-BR', options);
    
        const dateTimeElement = document.getElementById('current-datetime');
        if (dateTimeElement) {
            dateTimeElement.textContent = dateTimeString;
        }
    
    // Atualizar o tempo restante se o temporizador estiver ativo
    if (estimatedEndTime !== null) {
        updateRemainingTime();
    }
}



// Função para buscar dados dos sensores do servidor
function fetchSensorData() {
    const serverUrl = `${window.location.origin}/sensores`;

    
    fetch(serverUrl)
        .then(response => {
            if (!response.ok) {
                throw new Error('Erro na resposta do servidor: ' + response.status);
            }
            return response.json();
        })
        .then(data => {
            console.log('Dados recebidos do servidor:', data);
            updateConnectionStatus(true);
            updateDashboardData(data);
        })
        .catch(error => {
            console.error('Erro ao buscar dados do servidor:', error);
            updateConnectionStatus(false);
        });
}

// Atualizar status de conexão na UI
function updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connectionStatus');
    const textElement = document.getElementById('connectionText');
    
    if (connected) {
        statusElement.className = 'connection-status connected';
        textElement.textContent = 'Conectado';
    } else {
        statusElement.className = 'connection-status disconnected';
        textElement.textContent = 'Desconectado';
    }
}

// Atualizar dados do dashboard com base nos dados recebidos
function updateDashboardData(data) {
    if (data.umidade !== undefined) {
        const humElem = document.getElementById('humidity');
        if (humElem) humElem.textContent = Number(data.umidade).toFixed(1);
    }
    // Atualizar temperatura
    if (data.temperatura !== undefined) {
        document.getElementById('temperature').textContent = data.temperatura.toFixed(1);
        // Verificar alerta de temperatura
        if (data.alerta_temp_ativo) {
            document.getElementById('temp-alert').textContent = `ALERTA: Temperatura acima de ${TEMP_THRESHOLD}°C!`;
            document.getElementById('temp-alert').className = 'alert-status danger';
        } else {
            document.getElementById('temp-alert').textContent = 'Normal';
            document.getElementById('temp-alert').className = 'alert-status normal';
        }
    }
    // Atualizar pressão
    if (data.pressao !== undefined) {
        document.getElementById('pressure').textContent = data.pressao.toFixed(2);
    }
    // Atualizar luminosidade
    if (data.luminosidade !== undefined) {
        document.getElementById('luminosity').textContent = data.luminosidade.toFixed(1);
        // Mensagem de status da caixa baseada na luminosidade
        let lumStatusElem = document.getElementById('luminosity-status');
        if (!lumStatusElem) {
            // Cria o elemento se não existir
            const lumElem = document.getElementById('luminosity');
            if (lumElem && lumElem.parentElement) {
                lumStatusElem = document.createElement('div');
                lumStatusElem.id = 'luminosity-status';
                lumStatusElem.style.marginTop = '4px';
                lumStatusElem.style.textAlign = 'center';
                lumElem.parentElement.appendChild(lumStatusElem);
            }
        }
        if (lumStatusElem) {
            if (Number(data.luminosidade) === 0) {
                lumStatusElem.textContent = 'Caixa Fechada';
                lumStatusElem.className = 'alert-status danger';
            } else {
                lumStatusElem.textContent = 'Caixa Aberta';
                lumStatusElem.className = 'alert-status normal';
            }
        }
    }
    // Atualizar aceleração
    if (data.aceleracao_total !== undefined) {
        document.getElementById('acceleration').textContent = data.aceleracao_total.toFixed(2);
        // Verificar alerta de movimento brusco
        if (data.alerta_acel_ativo) {
            document.getElementById('accel-alert').textContent = 'ALERTA: Movimento brusco detectado!';
            document.getElementById('accel-alert').className = 'alert-status danger';
        } else {
            document.getElementById('accel-alert').textContent = 'Normal';
            document.getElementById('accel-alert').className = 'alert-status normal';
        }
    }
    // Exibir tempo cronometrado (cronômetro)
    if (typeof data.tempo_decorrido_ms === 'number') {
        tempoDecorridoMs = data.tempo_decorrido_ms;
        ultimoUpdateTimestamp = Date.now();
        if (tempoDecorridoMs > 0) {
            cronometroAtivo = true;
            startCronometroInterval();
        } else {
            cronometroAtivo = false;
            stopCronometroInterval();
        }
        renderCronometro();
    }
    // Atualizar status da caixa
    if (data.caixa_aberta !== undefined) {
        // Se caixa_aberta: destrancada, senão: trancada
        const boxStatus = data.caixa_aberta ? 'Destrancada' : 'Trancada';
        document.getElementById('box-status').textContent = boxStatus;
        document.getElementById('box-status').className = data.caixa_aberta ? 'status open' : 'status closed';
        // Atualizar visibilidade do botão de travar/destravar
        if (!data.caixa_aberta) {
            document.getElementById('unlockBoxBtn').style.display = 'inline-block';
            document.getElementById('lockBoxBtn').style.display = 'none';
        } else {
            document.getElementById('unlockBoxBtn').style.display = 'none';
            document.getElementById('lockBoxBtn').style.display = 'inline-block';
        }
    }
    // Atualizar status do LED RGB
    if (data.led_status !== undefined) {
        updateLedStatus(data.led_status);
    }
    // Atualizar total de registros
    if (data.registros_totais !== undefined) {
        document.getElementById('totalRecords').textContent = data.registros_totais;
    }
    // Atualizar status de gravação
    if (data.logging_ativo !== undefined) {
        const recordingStatus = data.logging_ativo ? 'Gravando' : 'Parado';
        document.getElementById('recordingStatus').textContent = recordingStatus;
        updateRecordingButtons(data.logging_ativo);
    }
    // Atualizar timestamp da última atualização
    const now = new Date();
    const options = { 
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit',
        day: '2-digit',
        month: '2-digit',
        year: 'numeric'
    };
    document.getElementById('last-update').textContent = `Última atualização: ${now.toLocaleDateString('pt-BR', options)}`;
}

// Atualiza o valor do cronômetro no card Tempo Cronometrado
function renderCronometro() {

    const chronElem = document.getElementById('chronometer-time');
    if (chronElem) {
        chronElem.textContent = formatarTempo(tempoDecorridoMs);
    }
}

// Formata milissegundos para HH:MM:SS
function formatarTempo(ms) {
    let totalSeconds = Math.floor(ms / 1000);
    let hours = Math.floor(totalSeconds / 3600);
    let minutes = Math.floor((totalSeconds % 3600) / 60);
    let seconds = totalSeconds % 60;
    return `${hours.toString().padStart(2,'0')}:${minutes.toString().padStart(2,'0')}:${seconds.toString().padStart(2,'0')}`;
}

// Inicia o intervalo do cronômetro local
function startCronometroInterval() {
    if (timerInterval) return;
    timerInterval = setInterval(() => {
        if (cronometroAtivo) {
            // Incrementa o tempo decorrido localmente desde o último update do servidor
            if (ultimoUpdateTimestamp) {
                const agora = Date.now();
                const delta = agora - ultimoUpdateTimestamp;
                tempoDecorridoMs += delta;
                ultimoUpdateTimestamp = agora;
                renderCronometro();
            }
        }
    }, 1000);
}

function stopCronometroInterval() {
    if (timerInterval) {
        clearInterval(timerInterval);
        timerInterval = null;
    }
}

// Atualizar botões de gravação com base no status
function updateRecordingButtons(isRecording) {
    if (isRecording) {
        document.querySelector('.btn-success').style.display = 'none';
        document.querySelector('.btn-danger').style.display = 'inline-block';
    } else {
        document.querySelector('.btn-success').style.display = 'inline-block';
        document.querySelector('.btn-danger').style.display = 'none';
    }
}

// Atualizar data e hora atual
function updateCurrentDateTime() {
    const now = new Date();
    document.getElementById('current-datetime').textContent = now.toLocaleString();
    
    // Atualizar contador de tempo se estiver ativo
    if (estimatedEndTime !== null) {
        updateRemainingTime();
    }
}

// Atualizar tempo restante no contador
function updateRemainingTime() {
    const now = new Date();
    const timeDiff = estimatedEndTime - now;
    
    if (timeDiff <= 0) {
        // Tempo esgotado
        document.getElementById('estimated-time').textContent = '00:00:00';
        document.getElementById('time-alert').textContent = 'TEMPO ESGOTADO!';
        document.getElementById('time-alert').className = 'alert-status danger';
        
        // Enviar comando para ativar o buzzer
        sendCommand('buzzer_on', 'time_alert');
        
        // Parar o contador
        clearInterval(timerInterval);
        estimatedEndTime = null;
    } else {
        // Calcular horas, minutos e segundos restantes
        const hours = Math.floor(timeDiff / (1000 * 60 * 60));
        const minutes = Math.floor((timeDiff % (1000 * 60 * 60)) / (1000 * 60));
        const seconds = Math.floor((timeDiff % (1000 * 60)) / 1000);
        
        // Formatar o tempo restante
        const timeString = `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
        document.getElementById('estimated-time').textContent = timeString;
        
        // Verificar se está próximo do tempo limite para alerta
        const minutesLeft = Math.ceil(timeDiff / (1000 * 60));
        if (minutesLeft <= alertMinutesThreshold) {
            document.getElementById('time-alert').textContent = `ALERTA: Menos de ${minutesLeft} minutos restantes!`;
            document.getElementById('time-alert').className = 'alert-status warning';
            
            // Enviar comando para ativar o buzzer se acabou de entrar no limite
            if (minutesLeft === alertMinutesThreshold) {
                sendCommand('buzzer_on', 'time_warning');
            }
        } else {
            document.getElementById('time-alert').textContent = 'Em andamento';
            document.getElementById('time-alert').className = 'alert-status normal';
        }
    }
}

// Atualizar status do LED RGB
function updateLedStatus(status) {
    // Resetar todos os LEDs
    document.getElementById('led-red').className = 'led red';
    document.getElementById('led-green').className = 'led green';
    document.getElementById('led-blue').className = 'led blue';
    
    // Atualizar status do LED com base no estado recebido
    switch(status) {
        case 'normal':
            // Verde ativo - tudo normal
            document.getElementById('led-green').className = 'led green active';
            document.getElementById('led-status').textContent = 'Normal';
            break;
        case 'warning':
            // Azul ativo - aviso
            document.getElementById('led-blue').className = 'led blue active';
            document.getElementById('led-status').textContent = 'Aviso';
            break;
        case 'danger':
            // Vermelho ativo - perigo
            document.getElementById('led-red').className = 'led red active';
            document.getElementById('led-status').textContent = 'Alerta';
            break;
        case 'all_off':
            // Todos desligados
            document.getElementById('led-status').textContent = 'Desligado';
            break;
        default:
            // Combinações personalizadas
            if (status.includes('r')) {
                document.getElementById('led-red').className = 'led red active';
            }
            if (status.includes('g')) {
                document.getElementById('led-green').className = 'led green active';
            }
            if (status.includes('b')) {
                document.getElementById('led-blue').className = 'led blue active';
            }
            document.getElementById('led-status').textContent = 'Personalizado';
    }
}

// Enviar comando para o servidor
function sendCommand(command, params = null) {
    const serverUrl = 'http://localhost:3000/comando';
    
    const message = {
        comando: command,
        parametros: params
    };
    
    fetch(serverUrl, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(message)
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Erro na resposta do servidor: ' + response.status);
        }
        return response.json();
    })
    .then(data => {
        console.log(`Comando enviado com sucesso: ${command}`, data);
        return true;
    })
    .catch(error => {
        console.error('Erro ao enviar comando:', error);
        return false;
    });
}

// Funções para o teclado numérico
function addDigit(digit) {
    if (currentPassword.length < 4) {
        currentPassword += digit;
        document.getElementById('unlockPassword').value = '*'.repeat(currentPassword.length);
    }
}

function clearPassword() {
    currentPassword = '';
    document.getElementById('unlockPassword').value = '';
}

// Funções para os modais
function showUnlockModal() {
    document.getElementById('unlockModal').style.display = 'block';
    clearPassword();
}

function closeUnlockModal() {
    document.getElementById('unlockModal').style.display = 'none';
    clearPassword();
}

function showLockModal() {
    document.getElementById('lockModal').style.display = 'block';
}

function closeLockModal() {
    document.getElementById('lockModal').style.display = 'none';
}

function showTimerModal() {
    document.getElementById('timerModal').style.display = 'block';
}

function closeTimerModal() {
    document.getElementById('timerModal').style.display = 'none';
}

// Funções para travar/destravar a caixa
function unlockBox() {
    if (currentPassword.length === 4) {
        sendCommand('unlock_box', { password: currentPassword });
        closeUnlockModal();
    } else {
        alert('A senha deve ter 4 dígitos!');
    }
}

function lockBox() {
    sendCommand('lock_box');
    closeLockModal();
}

// Configurar o temporizador
function setTimer() {
    const hours = parseInt(document.getElementById('hours').value) || 0;
    const minutes = parseInt(document.getElementById('minutes').value) || 0;
    const seconds = parseInt(document.getElementById('seconds').value) || 0;
    alertMinutesThreshold = parseInt(document.getElementById('alertMinutes').value) || 10;
    
    // Verificar se pelo menos um valor foi definido
    if (hours === 0 && minutes === 0 && seconds === 0) {
        // Se todos os valores forem zero, desativar o temporizador
        sendCommand('set_timer', {
            hours: 0,
            minutes: 0,
            seconds: 0,
            alert_minutes: 0,
            active: false
        });
        
        // Limpar o intervalo e resetar o tempo estimado
        if (timerInterval !== null) {
            clearInterval(timerInterval);
            timerInterval = null;
        }
        estimatedEndTime = null;
        
        // Atualizar a interface
        document.getElementById('estimated-time').textContent = '--:--:--';
        document.getElementById('time-alert').textContent = 'Inativo';
        document.getElementById('time-alert').className = 'alert-status normal';
        
        closeTimerModal();
        return;
    }
    
    // Calcular o tempo estimado de chegada
    const now = new Date();
    estimatedEndTime = new Date(now.getTime() + (hours * 60 * 60 * 1000) + (minutes * 60 * 1000) + (seconds * 1000));
    
    // Iniciar o contador se ainda não estiver ativo
    if (timerInterval === null) {
        timerInterval = setInterval(updateRemainingTime, 1000);
    }
    
    // Atualizar imediatamente
    updateRemainingTime();
    
    // Enviar configuração para o ESP32
    sendCommand('set_timer', {
        hours: hours,
        minutes: minutes,
        seconds: seconds,
        alert_minutes: alertMinutesThreshold,
        active: true
    });
    
    closeTimerModal();
}

// Inicializar elementos da UI
function updateUIStatus() {
    // Adicionar elemento para data e hora atual se não existir
    if (!document.getElementById('current-datetime')) {
        const header = document.querySelector('.header');
        const datetimeDiv = document.createElement('div');
        datetimeDiv.className = 'current-datetime';
        datetimeDiv.innerHTML = `<span id="current-datetime"></span>`;
        header.appendChild(datetimeDiv);
    }
}