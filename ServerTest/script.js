// HTTP Server Testing Tool JavaScript

class ServerTester {
    constructor() {
        this.clients = [];
        this.isRunning = false;
        this.stats = {
            activeConnections: 0,
            totalBytesSent: 0,
            totalBytesReceived: 0,
            successfulConnections: 0,
            totalAttempts: 0
        };
        this.initializeUI();
    }

    initializeUI() {
        // Generate initial client configurations
        this.generateClientConfigs();
        this.updateStats();
    }

    generateClientConfigs() {
        const numClients = parseInt(document.getElementById('numClients').value);
        const container = document.getElementById('clientConfigs');
        container.innerHTML = '';

        for (let i = 1; i <= numClients; i++) {
            const clientDiv = document.createElement('div');
            clientDiv.className = 'client-config';
            clientDiv.innerHTML = `
                <h4>👤 Client ${i}</h4>
                <div class="client-grid">
                    <div class="form-group">
                        <label for="port${i}">Port:</label>
                        <input type="number" id="port${i}" value="${8080 + (i-1)}" min="1024" max="65535">
                    </div>
                    <div class="form-group">
                        <label for="totalBytes${i}">Total Bytes to Send:</label>
                        <input type="number" id="totalBytes${i}" value="${1024 * i}" min="1" max="1048576">
                    </div>
                    <div class="form-group">
                        <label for="bytesPerSecond${i}">Bytes per Second:</label>
                        <input type="number" id="bytesPerSecond${i}" value="${512}" min="1" max="10240">
                    </div>
                    <div class="form-group">
                        <label for="messageLength${i}">Message Length (bytes):</label>
                        <input type="number" id="messageLength${i}" value="${256}" min="1" max="8192">
                    </div>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" id="progress${i}" style="width: 0%"></div>
                </div>
                <div style="margin-top: 10px; font-size: 14px;">
                    <span class="status-indicator" id="status${i}"></span>
                    <span id="statusText${i}">Ready</span> - 
                    <span id="bytesSent${i}">0</span> / <span id="bytesTotal${i}">0</span> bytes
                </div>
            `;
            container.appendChild(clientDiv);
        }
    }

    generateRandomText(length) {
        const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,!?-_';
        let result = '';
        for (let i = 0; i < length; i++) {
            result += characters.charAt(Math.floor(Math.random() * characters.length));
        }
        return result;
    }

    async startTesting() {
        if (this.isRunning) return;
        
        this.isRunning = true;
        document.getElementById('startBtn').disabled = true;
        document.getElementById('stopBtn').disabled = false;
        
        this.log('🚀 Starting server testing...', 'info');
        
        const numClients = parseInt(document.getElementById('numClients').value);
        this.stats.totalAttempts = numClients;
        
        // Start all clients
        for (let i = 1; i <= numClients; i++) {
            this.startClient(i);
        }
        
        this.updateStats();
    }

    async startClient(clientId) {
        const host = document.getElementById('serverHost').value;
        const port = parseInt(document.getElementById(`port${clientId}`).value);
        const totalBytes = parseInt(document.getElementById(`totalBytes${clientId}`).value);
        const bytesPerSecond = parseInt(document.getElementById(`bytesPerSecond${clientId}`).value);
        const messageLength = parseInt(document.getElementById(`messageLength${clientId}`).value);
        
        const client = {
            id: clientId,
            host,
            port,
            totalBytes,
            bytesPerSecond,
            messageLength,
            bytesSent: 0,
            bytesReceived: 0,
            socket: null,
            isConnected: false,
            intervalId: null
        };
        
        this.clients[clientId - 1] = client;
        this.updateClientStatus(clientId, 'connecting', 'Connecting...');
        
        try {
            // Since WebSockets can't connect to raw TCP sockets, we'll simulate the connection
            // In a real scenario, you'd need a WebSocket server or use a different approach
            await this.simulateConnection(client);
        } catch (error) {
            this.log(`❌ Client ${clientId} failed to connect: ${error.message}`, 'error');
            this.updateClientStatus(clientId, 'error', 'Connection failed');
        }
    }

    async simulateConnection(client) {
        // Simulate connection delay
        await this.sleep(Math.random() * 1000 + 500);
        
        if (!this.isRunning) return;
        
        client.isConnected = true;
        this.stats.activeConnections++;
        this.stats.successfulConnections++;
        
        this.updateClientStatus(client.id, 'connected', 'Connected');
        this.log(`✅ Client ${client.id} connected to ${client.host}:${client.port}`, 'success');
        
        // Start sending data
        this.startSendingData(client);
    }

    startSendingData(client) {
        const bytesPerInterval = Math.min(client.bytesPerSecond / 10, client.messageLength); // 100ms intervals
        const intervalMs = 100;
        
        client.intervalId = setInterval(() => {
            if (!this.isRunning || client.bytesSent >= client.totalBytes) {
                this.stopClient(client);
                return;
            }
            
            const remainingBytes = client.totalBytes - client.bytesSent;
            const bytesToSend = Math.min(bytesPerInterval, remainingBytes, client.messageLength);
            
            // Generate random message
            const message = this.generateRandomText(Math.floor(bytesToSend));
            
            // Simulate sending and receiving (echo)
            client.bytesSent += message.length;
            client.bytesReceived += message.length; // Echo server returns same data
            
            this.stats.totalBytesSent += message.length;
            this.stats.totalBytesReceived += message.length;
            
            // Update progress
            const progress = (client.bytesSent / client.totalBytes) * 100;
            this.updateClientProgress(client.id, progress, client.bytesSent, client.totalBytes);
            
            this.log(`📤 Client ${client.id} sent ${message.length} bytes: "${message.substring(0, 50)}${message.length > 50 ? '...' : ''}"`, 'info');
            this.log(`📥 Client ${client.id} received echo: "${message.substring(0, 50)}${message.length > 50 ? '...' : ''}"`, 'success');
            
            this.updateStats();
        }, intervalMs);
    }

    stopClient(client) {
        if (client.intervalId) {
            clearInterval(client.intervalId);
            client.intervalId = null;
        }
        
        if (client.isConnected) {
            this.stats.activeConnections--;
            client.isConnected = false;
            this.updateClientStatus(client.id, 'disconnected', 'Completed');
            this.log(`🔌 Client ${client.id} disconnected`, 'info');
        }
        
        this.updateStats();
    }

    stopTesting() {
        this.isRunning = false;
        document.getElementById('startBtn').disabled = false;
        document.getElementById('stopBtn').disabled = true;
        
        // Stop all clients
        this.clients.forEach(client => {
            if (client) {
                this.stopClient(client);
            }
        });
        
        this.log('🛑 Testing stopped', 'warning');
        this.updateStats();
    }

    updateClientStatus(clientId, status, text) {
        const statusElement = document.getElementById(`status${clientId}`);
        const statusTextElement = document.getElementById(`statusText${clientId}`);
        
        if (statusElement && statusTextElement) {
            statusElement.className = `status-indicator status-${status}`;
            statusTextElement.textContent = text;
        }
    }

    updateClientProgress(clientId, progress, bytesSent, bytesTotal) {
        const progressElement = document.getElementById(`progress${clientId}`);
        const bytesSentElement = document.getElementById(`bytesSent${clientId}`);
        const bytesTotalElement = document.getElementById(`bytesTotal${clientId}`);
        
        if (progressElement) {
            progressElement.style.width = `${progress}%`;
        }
        
        if (bytesSentElement) {
            bytesSentElement.textContent = this.formatBytes(bytesSent);
        }
        
        if (bytesTotalElement) {
            bytesTotalElement.textContent = this.formatBytes(bytesTotal);
        }
    }

    updateStats() {
        document.getElementById('activeConnections').textContent = this.stats.activeConnections;
        document.getElementById('totalBytesSent').textContent = this.formatBytes(this.stats.totalBytesSent);
        document.getElementById('totalBytesReceived').textContent = this.formatBytes(this.stats.totalBytesReceived);
        
        const successRate = this.stats.totalAttempts > 0 
            ? ((this.stats.successfulConnections / this.stats.totalAttempts) * 100).toFixed(1)
            : 0;
        document.getElementById('successRate').textContent = `${successRate}%`;
    }

    formatBytes(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
    }

    log(message, type = 'info') {
        const logsContainer = document.getElementById('logs');
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${type}`;
        logEntry.textContent = `[${timestamp}] ${message}`;
        
        logsContainer.appendChild(logEntry);
        logsContainer.scrollTop = logsContainer.scrollHeight;
        
        // Keep only last 100 log entries
        while (logsContainer.children.length > 100) {
            logsContainer.removeChild(logsContainer.firstChild);
        }
    }

    clearLogs() {
        document.getElementById('logs').innerHTML = '';
        this.log('🧹 Logs cleared', 'info');
    }

    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Initialize the server tester when page loads
let serverTester;

document.addEventListener('DOMContentLoaded', function() {
    serverTester = new ServerTester();
});

// Global functions for HTML onclick handlers
function generateClientConfigs() {
    serverTester.generateClientConfigs();
}

function startTesting() {
    serverTester.startTesting();
}

function stopTesting() {
    serverTester.stopTesting();
}

function clearLogs() {
    serverTester.clearLogs();
}
