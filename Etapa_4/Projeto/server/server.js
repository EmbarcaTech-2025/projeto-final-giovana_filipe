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
    umidade: 0, // <-- Adicione esta linha
    luminosidade: 0,
    aceleracao_x: 0,
    aceleracao_y: 0,
    aceleracao_z: 0,
    aceleracao_total: 0,
    caixa_aberta: false,
    tempo_entrega_ms: 0,
    tempo_restante_ms: 0,
    alerta_tempo_min: 10,
    alerta_temp_ativo: false,
    alerta_acel_ativo: false,
    alarme_tempo_ativo: false,
    alerta_tempo_ativo: false,
    data_hora: '',
    led_status: 'all_off',
    registros_totais: 0,
    tempo_decorrido_ms: 0, // <-- Adicionado para o cronômetro
    timestamp: new Date()
};

// Armazenar o histórico de temperatura
let historicoTemperatura = [];
// Armazenar o histórico completo de todos os dados
let historicoCompleto = [];

// Armazenar comando pendente para o Pico
let comandoPendente = null;

app.post('/receber', (req, res) => {
    const { dado } = req.body;
    console.log('Dado recebido:', dado);
    res.send('Dados recebidos com sucesso!');
});

app.post('/sensores', (req, res) => {
    console.log('[POST /sensores] Body recebido:', req.body);
    ultimosDados = {
        temperatura: req.body.temperatura ?? 0,
        pressao: req.body.pressao ?? 0,
        umidade: req.body.umidade ?? 0, // <-- Adicione esta linha
        luminosidade: req.body.luminosidade ?? 0,
        aceleracao_x: req.body.aceleracao_x ?? 0,
        aceleracao_y: req.body.aceleracao_y ?? 0,
        aceleracao_z: req.body.aceleracao_z ?? 0,
        aceleracao_total: req.body.aceleracao_total ?? 0,
        caixa_aberta: req.body.caixa_aberta ?? false,
        tempo_entrega_ms: req.body.tempo_entrega_ms ?? 0,
        tempo_restante_ms: req.body.tempo_restante_ms ?? 0,
        alerta_tempo_min: req.body.alerta_tempo_min ?? 10,
        alerta_temp_ativo: req.body.alerta_temp_ativo ?? false,
        alerta_acel_ativo: req.body.alerta_acel_ativo ?? false,
        alarme_tempo_ativo: req.body.alarme_tempo_ativo ?? false,
        alerta_tempo_ativo: req.body.alerta_tempo_ativo ?? false,
        data_hora: req.body.data_hora ?? '',
        led_status: req.body.led_status ?? 'all_off',
        registros_totais: req.body.registros_totais ?? 0,
        tempo_decorrido_ms: req.body.tempo_decorrido_ms ?? 0, // <-- Adicionado para o cronômetro
        timestamp: new Date()
    };
    // Adiciona temperatura ao histórico
    historicoTemperatura.push({
        temperatura: ultimosDados.temperatura,
        timestamp: new Date()
    });
    // Limita o histórico a 500 pontos
    if (historicoTemperatura.length > 500) historicoTemperatura.shift();
    // Adiciona todos os dados ao histórico completo
    historicoCompleto.push({ ...ultimosDados });
    // Limita o histórico completo a 2000 pontos
    if (historicoCompleto.length > 2000) historicoCompleto.shift();
    console.log('[POST /sensores] ultimosDados atualizado:', ultimosDados);
    
    // Verificar se há comando pendente
    let response = { sucesso: true, mensagem: 'Dados dos sensores recebidos com sucesso!' };
    if (comandoPendente) {
        response.comando = comandoPendente;
        comandoPendente = null; // Limpar após enviar
    }
    res.json(response);
});

app.get('/sensores', (req, res) => {
    console.log('[GET /sensores] ultimosDados enviado:', ultimosDados);
    res.json(ultimosDados);
});

// Endpoint para obter o histórico de temperatura
app.get('/historico-temperatura', (req, res) => {
    res.json(historicoTemperatura);
});

// Endpoint para download do histórico em CSV
app.get('/historico-csv', (req, res) => {
    if (historicoCompleto.length === 0) {
        return res.status(204).send('Sem dados para exportar');
    }
    // Cabeçalho CSV
    const header = Object.keys(historicoCompleto[0]).join(',');
    // Linhas CSV
    const rows = historicoCompleto.map(obj =>
        Object.values(obj).map(v =>
            typeof v === 'string' ? '"' + v.replace(/"/g, '""') + '"' : v
        ).join(',')
    );
    const csv = [header, ...rows].join('\n');
    res.setHeader('Content-Type', 'text/csv');
    res.setHeader('Content-Disposition', 'attachment; filename="historico_transporte.csv"');
    res.send(csv);
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

// Endpoint para o Pico buscar comandos pendentes
app.get('/comando', (req, res) => {
    if (comandoPendente) {
        res.json({ comando: comandoPendente.comando, parametros: comandoPendente.parametros });
        comandoPendente = null; // Limpar após enviar
    } else {
        res.json({ comando: null });
    }
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
                comandoPendente = { comando, parametros: params };
                result = { status: 'success', message: 'Timer configurado com sucesso' };
            }
            break;
        case 'lock_box':
            console.log('Travando caixa');
            comandoPendente = { comando, parametros: params };
            result = { status: 'success', message: 'Caixa travada com sucesso' };
            break;
        case 'unlock_box':
            console.log('Destravando caixa');
            comandoPendente = { comando, parametros: params };
            result = { status: 'success', message: 'Caixa destravada com sucesso' };
            break;
        case 'start_transport':
            console.log('Iniciando transporte');
            comandoPendente = { comando, parametros: params };
            result = { status: 'success', message: 'Transporte iniciado com sucesso' };
            break;
        case 'start_recording':
            console.log('Iniciando gravação');
            comandoPendente = { comando, parametros: params };
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