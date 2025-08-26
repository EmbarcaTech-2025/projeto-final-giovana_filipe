# Servidor HTTP simples para servir os arquivos do dashboard
# Este script é uma alternativa ao servidor Node.js

import http.server
import socketserver
import os
import json
from urllib.parse import parse_qs, urlparse, unquote

# Configuração do servidor
PORT = 3000
HANDLER = http.server.SimpleHTTPRequestHandler

# Dados dos sensores (simulados)
ultimos_dados = {
    "temperatura": 25.5,
    "pressao": 1013.2,
    "luminosidade": 75,
    "led_status": "off",
    "timestamp": "2023-08-22T15:30:00Z"
}

# Classe personalizada para lidar com as requisições
class DashboardHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        # Rota para obter dados dos sensores
        if self.path == '/sensores':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(ultimos_dados).encode())
            return
        
        # Servir arquivos estáticos
        return http.server.SimpleHTTPRequestHandler.do_GET(self)
    
    def do_POST(self):
        # Rota para receber dados dos sensores
        if self.path == '/sensores':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            dados = json.loads(post_data.decode('utf-8'))
            
            # Atualizar dados
            global ultimos_dados
            ultimos_dados.update(dados)
            
            # Responder
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({"status": "success"}).encode())
            return
        
        # Rota para receber comandos
        elif self.path == '/comando':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            comando_data = json.loads(post_data.decode('utf-8'))
            
            comando = comando_data.get('comando')
            params = comando_data.get('parametros', {})
            
            print(f"Comando recebido: {comando}")
            print(f"Parâmetros: {params}")
            
            # Processar comando (simulado)
            resultado = {"sucesso": True, "mensagem": f"Comando {comando} processado com sucesso"}
            
            # Responder
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(resultado).encode())
            return
    
    def do_OPTIONS(self):
        # Suporte a CORS
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

# Mudar para o diretório web
web_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'web')
os.chdir(web_dir)

# Iniciar o servidor
with socketserver.TCPServer(("0.0.0.0", PORT), DashboardHandler) as httpd:
    print(f"Servidor rodando na porta {PORT}")
    print(f"Acesse o dashboard em: http://10.63.72.222:{PORT}/dashboard.html")
    httpd.serve_forever()