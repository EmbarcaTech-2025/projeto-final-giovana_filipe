import express, { Request, Response } from 'express';
import cors from 'cors';
import path from 'path';
const app = express();
const PORT = 3000;

app.use(express.json());
app.use(cors());

// Servir arquivos estáticos do dashboard
app.use(express.static(path.join(__dirname, '../web')));

// Rota para a página principal
app.get('/', (req: Request, res: Response) => {
    res.sendFile(path.join(__dirname, '../web/dashboard.html'));
});

// Armazenar os dados mais recentes dos sensores
let ultimosDados = {
    temperatura: 0,
    pressao: 0,
    umidade: 0, 
    luminosidade: 0,
    aceleracao: {
        x: 0,
        y: 0,
        z: 0
    },
    aceleracao_total: 0, // <-- Se quiser mostrar no dashboard
    caixaAberta: false,
    tempo_decorrido_ms: 0, // NOVO campo para o cronômetro
    timestamp: new Date()
};

// Endpoint original para receber dados simples
app.post('/receber', (req: Request, res: Response) => {
    const { dado } = req.body;
    console.log('Dado recebido:', dado);
    res.send('Dados recebidos com sucesso!');
});

// Novo endpoint para receber dados dos sensores
app.post('/sensores', (req: Request, res: Response) => {
    const { temperatura, pressao, umidade, luminosidade, aceleracao_x, aceleracao_y, aceleracao_z, aceleracao_total, caixa_aberta, tempo_decorrido_ms } = req.body;

    ultimosDados = {
        temperatura,
        pressao,
        umidade,
        luminosidade,
        aceleracao: {
            x: aceleracao_x,
            y: aceleracao_y,
            z: aceleracao_z
        },
        aceleracao_total,
        caixaAberta: caixa_aberta,
        tempo_decorrido_ms: tempo_decorrido_ms ?? 0,
        timestamp: new Date()
    };

    console.log('Dados dos sensores recebidos:', ultimosDados);
    res.send('Dados dos sensores recebidos com sucesso!');
});

// Endpoint para obter os dados mais recentes
app.get('/sensores', (req: Request, res: Response) => {
    res.json(ultimosDados);
});

// Endpoint para receber comandos do dashboard
app.post('/comando', (req: Request, res: Response) => {
    const { comando, parametros } = req.body;
    console.log('Comando recebido:', comando, parametros);
    
    // Processar o comando usando a função processarComando
    const resultado = processarComando(comando, parametros);
    
    res.json({
        sucesso: resultado.status === 'success',
        mensagem: resultado.message,
        timestamp: new Date()
    });
});

// Função para processar comandos
function processarComando(comando, params) {
    console.log(`Processando comando: ${comando}`, params);
    let result;
    
    switch (comando) {
        case 'set_timer':
            // Validar parâmetros
            if (typeof params.hours !== 'number' || 
                typeof params.minutes !== 'number' || 
                typeof params.seconds !== 'number' || 
                typeof params.alert_minutes !== 'number' || 
                typeof params.active !== 'boolean') {
                console.error('Parâmetros inválidos para set_timer');
                result = { status: 'error', message: 'Parâmetros inválidos' };
            } else {
                // Processar comando set_timer
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

app.listen(PORT, '0.0.0.0', () => {
    console.log(`Servidor rodando na porta ${PORT}`);
    console.log(`Acesse o dashboard em: http://10.63.72.222:${PORT}/`);
});