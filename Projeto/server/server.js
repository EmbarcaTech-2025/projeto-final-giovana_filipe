const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 3000;

app.use(express.json());
app.use(cors());

// Servir arquivos estáticos do dashboard
app.use(express.static(path.join(__dirname, '../web')));

// Redirecionar '/' para '/dashboard.html'
app.get('/', (req, res) => {
    res.redirect('/dashboard.html');
});

// Armazenar os dados mais recentes dos sensores

let ultimosDados = {
    temperatura: 0,
    pressao: 0,
    luminosidade: 0,
    aceleracao_x: 0,
    aceleracao_y: 0,
    aceleracao_z: 0,
    caixa_aberta: false,
    tempo_entrega_ms: 0,
    tempo_restante_ms: 0,
    alerta_tempo_min: 10,
    alerta_temp_ativo: false,
    alerta_acel_ativo: false,
    alarme_tempo_ativo: false,
    alerta_tempo_ativo: false,
    data_hora: '',
    timestamp: new Date()
};

app.post('/receber', (req, res) => {
    const { dado } = req.body;
    console.log('Dado recebido:', dado);
    res.send('Dados recebidos com sucesso!');
});

app.post('/sensores', (req, res) => {
    console.log('[POST /sensores] Body recebido:', req.body);
    // Copiar todos os campos relevantes do body para ultimosDados
    ultimosDados = {
        temperatura: req.body.temperatura ?? 0,
        pressao: req.body.pressao ?? 0,
        luminosidade: req.body.luminosidade ?? 0,
        aceleracao_x: req.body.aceleracao_x ?? 0,
        aceleracao_y: req.body.aceleracao_y ?? 0,
        aceleracao_z: req.body.aceleracao_z ?? 0,
        caixa_aberta: req.body.caixa_aberta ?? false,
        tempo_entrega_ms: req.body.tempo_entrega_ms ?? 0,
        tempo_restante_ms: req.body.tempo_restante_ms ?? 0,
        alerta_tempo_min: req.body.alerta_tempo_min ?? 10,
        alerta_temp_ativo: req.body.alerta_temp_ativo ?? false,
        alerta_acel_ativo: req.body.alerta_acel_ativo ?? false,
        alarme_tempo_ativo: req.body.alarme_tempo_ativo ?? false,
        alerta_tempo_ativo: req.body.alerta_tempo_ativo ?? false,
        data_hora: req.body.data_hora ?? '',
        timestamp: new Date()
    };
    console.log('[POST /sensores] ultimosDados atualizado:', ultimosDados);
    res.send('Dados dos sensores recebidos com sucesso!');
});

app.get('/sensores', (req, res) => {
    console.log('[GET /sensores] ultimosDados enviado:', ultimosDados);
    res.json(ultimosDados);
});

app.post('/comando', (req, res) => {
    const { comando, parametros } = req.body;
    console.log('Comando recebido:', comando, parametros);
    const resultado = processarComando(comando, parametros);
    res.json({
        sucesso: resultado.status === 'success',
        mensagem: resultado.message,
        timestamp: new Date()
    });
});

function processarComando(comando, params) {
    console.log(`Processando comando: ${comando}`, params);
    let result;
    switch (comando) {
        case 'set_timer':
            if (typeof params.hours !== 'number' || 
                typeof params.minutes !== 'number' || 
                typeof params.seconds !== 'number' || 
                typeof params.alert_minutes !== 'number' || 
                typeof params.active !== 'boolean') {
                console.error('Parâmetros inválidos para set_timer');
                result = { status: 'error', message: 'Parâmetros inválidos' };
            } else {
                console.log('Configurando timer:', params);
                result = { status: 'success', message: 'Timer configurado com sucesso' };
            }
            break;
        case 'lock_box':
            console.log('Travando caixa');
            result = { status: 'success', message: 'Caixa travada com sucesso' };
            break;
        case 'unlock_box':
            console.log('Destravando caixa');
            result = { status: 'success', message: 'Caixa destravada com sucesso' };
            break;
        case 'start_recording':
            console.log('Iniciando gravação');
            result = { status: 'success', message: 'Gravação iniciada com sucesso' };
            break;
        default:
            console.error(`Comando desconhecido: ${comando}`);
            result = { status: 'error', message: 'Comando desconhecido' };
    }
    return result;
}

app.listen(PORT, '::', () => {
    console.log(`Servidor rodando na porta ${PORT} (IPv4)`);
    console.log(`Acesse o dashboard em: http://localhost:${PORT}/`);
});
