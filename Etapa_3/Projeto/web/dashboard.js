// Interatividade para o dashboard do BioCooler
// Comunicação apenas via polling HTTP (fetch), sem WebSocket

// Variáveis globais
let timerInterval = null; // Intervalo para o contador de tempo
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

    // Verificar status inicial dos elementos
    updateUIStatus();

    // Adicionar event listeners para os botões
    document.getElementById('startRecordingBtn').addEventListener('click', function() {
        sendCommand('start_recording');
    });

    document.getElementById('stopRecordingBtn').addEventListener('click', function() {
        sendCommand('stop_recording');
    });

    document.getElementById('downloadCsvBtn').addEventListener('click', function() {
        sendCommand('download_csv');
    });

    document.getElementById('unlockBoxBtn').addEventListener('click', function() {
        showUnlockModal();
    });

    document.getElementById('lockBoxBtn').addEventListener('click', function() {
        showLockModal();
    });

    document.getElementById('configTimerBtn').addEventListener('click', function() {
        showTimerModal();
    });

    // Event listeners para os modais
    document.getElementById('closeUnlockModal').addEventListener('click', closeUnlockModal);
    document.getElementById('closeLockModal').addEventListener('click', closeLockModal);
    document.getElementById('closeTimerModal').addEventListener('click', closeTimerModal);

    // Event listeners para os botões de ação nos modais
    document.getElementById('confirmUnlockBtn').addEventListener('click', unlockBox);
    document.getElementById('confirmLockBtn').addEventListener('click', lockBox);
    document.getElementById('confirmTimerBtn').addEventListener('click', setTimer);

    // Event listeners para o teclado numérico
    const keypadButtons = document.querySelectorAll('.keypad-button');
    keypadButtons.forEach(button => {
        button.addEventListener('click', function() {
            const digit = this.textContent;
            if (digit === 'C') {
                clearPassword();
            } else {
                addDigit(digit);
            }
        });
    });

    // Fechar modais quando clicar fora deles
    window.addEventListener('click', function(event) {
        if (event.target.classList.contains('modal')) {
            event.target.style.display = 'none';
            if (event.target.id === 'unlockModal') {
                clearPassword();
            }
        }
    });
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
        // Atualizar LED para verde quando conectado e sem alertas
        if (!document.getElementById('temp-alert').classList.contains('danger') && 
            !document.getElementById('accel-alert').classList.contains('danger') && 
            !document.getElementById('time-alert').classList.contains('danger')) {
            updateLedStatus('normal');
        }
    } else {
        statusElement.className = 'connection-status disconnected';
        textElement.textContent = 'Desconectado';
        // Atualizar LED para azul quando desconectado (aviso)
        updateLedStatus('warning');
    }
}

// Atualizar dados do dashboard com base nos dados recebidos
function updateDashboardData(data) {
    // Atualizar temperatura
    if (data.temperatura !== undefined) {
        document.getElementById('temperature').textContent = data.temperatura.toFixed(1);
        
        // Verificar alerta de temperatura
        if (data.alerta_temp_ativo || data.temperatura > TEMP_THRESHOLD) {
            document.getElementById('temp-alert').textContent = `ALERTA: Temperatura acima de ${TEMP_THRESHOLD}°C!`;
            document.getElementById('temp-alert').className = 'alert-status danger';
            
            // Atualizar LED para vermelho (perigo)
            updateLedStatus('danger');
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
        
        // Se a luminosidade for baixa, a caixa está fechada
        if (data.luminosidade < 10) {
            // Mostrar botão de destravar
            document.getElementById('unlockBoxBtn').style.display = 'inline-block';
            document.getElementById('lockBoxBtn').style.display = 'none';
        } else {
            // Mostrar botão de travar
            document.getElementById('unlockBoxBtn').style.display = 'none';
            document.getElementById('lockBoxBtn').style.display = 'inline-block';
        }
    }
    
    // Atualizar aceleração
    if (data.aceleracao_x !== undefined && data.aceleracao_y !== undefined && data.aceleracao_z !== undefined) {
        // Calcular a magnitude da aceleração
        const accelX = data.aceleracao_x / 16384.0; // Converter para g (1g = 16384 no MPU6050)
        const accelY = data.aceleracao_y / 16384.0;
        const accelZ = data.aceleracao_z / 16384.0;
        
        const magnitude = Math.sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ);
        document.getElementById('acceleration').textContent = magnitude.toFixed(2);
        
        // Verificar alerta de movimento brusco
        if (data.alerta_acel_ativo || magnitude > ACCEL_THRESHOLD) {
            document.getElementById('accel-alert').textContent = 'ALERTA: Movimento brusco detectado!';
            document.getElementById('accel-alert').className = 'alert-status danger';
            
            // Atualizar LED para vermelho (perigo)
            updateLedStatus('danger');
        } else {
            document.getElementById('accel-alert').textContent = 'Normal';
            document.getElementById('accel-alert').className = 'alert-status normal';
        }
    }
    
    // Atualizar tempo de entrega e tempo restante
    if (data.tempo_entrega_ms !== undefined && data.tempo_restante_ms !== undefined) {
        // Converter de milissegundos para horas e minutos
        const horasTotal = Math.floor(data.tempo_entrega_ms / (3600 * 1000));
        const minutosTotal = Math.floor((data.tempo_entrega_ms % (3600 * 1000)) / (60 * 1000));
        
        const horasRestantes = Math.floor(data.tempo_restante_ms / (3600 * 1000));
        const minutosRestantes = Math.floor((data.tempo_restante_ms % (3600 * 1000)) / (60 * 1000));
        const segundosRestantes = Math.floor((data.tempo_restante_ms % (60 * 1000)) / 1000);
        
        // Formatar o tempo restante
        const timeString = `${horasRestantes.toString().padStart(2, '0')}:${minutosRestantes.toString().padStart(2, '0')}:${segundosRestantes.toString().padStart(2, '0')}`;
        
        // Atualizar elementos na interface
        document.getElementById('estimated-time').textContent = timeString;
        
        if (data.alarme_tempo_ativo) {
            // Verificar se está próximo do tempo limite para alerta
            const tempoAlertaMs = data.alerta_tempo_min * 60 * 1000;
            if (data.tempo_restante_ms <= tempoAlertaMs && data.tempo_restante_ms > 0) {
                document.getElementById('time-alert').textContent = 
                    `ALERTA: Menos de ${data.alerta_tempo_min} minutos para entrega!`;
                document.getElementById('time-alert').className = 'alert-status warning';
                updateLedStatus('warning');
            } else if (data.tempo_restante_ms <= 0) {
                document.getElementById('time-alert').textContent = 
                    `ALERTA: Tempo de entrega esgotado!`;
                document.getElementById('time-alert').className = 'alert-status danger';
                updateLedStatus('danger');
            } else {
                document.getElementById('time-alert').textContent = 'Normal';
                document.getElementById('time-alert').className = 'alert-status normal';
            }
        } else {
            document.getElementById('time-alert').textContent = 'Inativo';
            document.getElementById('time-alert').className = 'alert-status normal';
        }
    }
    
    // Atualizar status da caixa
    if (data.caixa_aberta !== undefined) {
        const boxStatus = data.caixa_aberta ? 'ABERTA' : 'FECHADA';
        document.getElementById('box-status').textContent = boxStatus;
        document.getElementById('box-status').className = data.caixa_aberta ? 'status open' : 'status closed';
        
        // Atualizar visibilidade do botão de travar/destravar
        if (!data.caixa_aberta) {
            // Caixa fechada - mostrar botão de destravar
            document.getElementById('unlockBoxBtn').style.display = 'inline-block';
            document.getElementById('lockBoxBtn').style.display = 'none';
            
            // Atualizar LED para verde (normal/seguro) se não houver outros alertas
            if (!document.getElementById('temp-alert').classList.contains('danger') && 
                !document.getElementById('accel-alert').classList.contains('danger') && 
                !document.getElementById('time-alert').classList.contains('danger')) {
                updateLedStatus('normal');
            }
        } else {
            // Caixa aberta - mostrar botão de travar
            document.getElementById('unlockBoxBtn').style.display = 'none';
            document.getElementById('lockBoxBtn').style.display = 'inline-block';
            
            // Atualizar LED para azul (aviso - caixa aberta)
            if (!document.getElementById('temp-alert').classList.contains('danger') && 
                !document.getElementById('time-alert').classList.contains('danger')) {
                updateLedStatus('warning');
            }
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
        
        // Atualizar visibilidade dos botões de gravação
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
