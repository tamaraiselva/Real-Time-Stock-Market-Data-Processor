#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <regex>

// Simple HTTP server and JSON handling
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

struct StockData {
    std::string symbol;
    double price;
    double change;
    double changePercent;
    long long volume;
    double high;
    double low;
    double open;
    std::string timestamp;
    
    // Technical indicators
    double sma20;
    double sma50;
    double rsi;
    double volatility;
};

class StockMarketProcessor {
private:
    std::map<std::string, std::vector<double>> priceHistory;
    std::map<std::string, StockData> currentData;
    
public:
    // Simulate real-time stock data (in production, use actual API)
    StockData generateStockData(const std::string& symbol) {
        StockData data;
        data.symbol = symbol;
        
        // Simulate realistic stock prices with random walk
        static std::map<std::string, double> lastPrices = {
            {"AAPL", 175.0}, {"GOOGL", 140.0}, {"MSFT", 420.0}, 
            {"AMZN", 155.0}, {"TSLA", 220.0}, {"META", 480.0}
        };
        
        if (lastPrices.find(symbol) == lastPrices.end()) {
            lastPrices[symbol] = 100.0; // Default price
        }
        
        // Random price movement (-2% to +2%)
        double changePercent = (rand() % 400 - 200) / 10000.0;
        double newPrice = lastPrices[symbol] * (1 + changePercent);
        
        data.price = newPrice;
        data.change = newPrice - lastPrices[symbol];
        data.changePercent = changePercent * 100;
        data.volume = 1000000 + (rand() % 2000000);
        data.high = newPrice * (1 + (rand() % 30) / 1000.0);
        data.low = newPrice * (1 - (rand() % 30) / 1000.0);
        data.open = lastPrices[symbol];
        
        // Update price history
        priceHistory[symbol].push_back(newPrice);
        if (priceHistory[symbol].size() > 100) {
            priceHistory[symbol].erase(priceHistory[symbol].begin());
        }
        
        // Calculate technical indicators
        data.sma20 = calculateSMA(symbol, 20);
        data.sma50 = calculateSMA(symbol, 50);
        data.rsi = calculateRSI(symbol);
        data.volatility = calculateVolatility(symbol);
        
        // Update timestamp
        auto now = std::time(nullptr);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        data.timestamp = oss.str();
        
        lastPrices[symbol] = newPrice;
        currentData[symbol] = data;
        
        return data;
    }
    
    double calculateSMA(const std::string& symbol, int period) {
        if (priceHistory[symbol].size() < static_cast<size_t>(period)) return 0.0;
        
        double sum = 0.0;
        size_t count = std::min(static_cast<size_t>(period), priceHistory[symbol].size());
        
        for (size_t i = priceHistory[symbol].size() - count; i < priceHistory[symbol].size(); i++) {
            sum += priceHistory[symbol][i];
        }
        
        return sum / count;
    }
    
    double calculateRSI(const std::string& symbol, int period = 14) {
        if (priceHistory[symbol].size() < static_cast<size_t>(period + 1)) return 50.0;
        
        std::vector<double> gains, losses;
        const auto& prices = priceHistory[symbol];
        
        for (size_t i = prices.size() - period - 1; i < prices.size() - 1; i++) {
            double change = prices[i + 1] - prices[i];
            gains.push_back(change > 0 ? change : 0);
            losses.push_back(change < 0 ? -change : 0);
        }
        
        double avgGain = 0, avgLoss = 0;
        for (double gain : gains) avgGain += gain;
        for (double loss : losses) avgLoss += loss;
        
        avgGain /= period;
        avgLoss /= period;
        
        if (avgLoss == 0) return 100.0;
        
        double rs = avgGain / avgLoss;
        return 100.0 - (100.0 / (1.0 + rs));
    }
    
    double calculateVolatility(const std::string& symbol, int period = 20) {
        if (priceHistory[symbol].size() < static_cast<size_t>(period)) return 0.0;
        
        const auto& prices = priceHistory[symbol];
        std::vector<double> returns;
        
        size_t start = prices.size() >= static_cast<size_t>(period) ? prices.size() - period : 0;
        for (size_t i = start; i < prices.size() - 1; i++) {
            returns.push_back((prices[i + 1] - prices[i]) / prices[i]);
        }
        
        double mean = 0;
        for (double ret : returns) mean += ret;
        mean /= returns.size();
        
        double variance = 0;
        for (double ret : returns) {
            variance += (ret - mean) * (ret - mean);
        }
        variance /= returns.size();
        
        return std::sqrt(variance) * std::sqrt(252); // Annualized
    }
    
    std::string generateJSON(const std::vector<std::string>& symbols) {
        std::ostringstream json;
        json << "{\n  \"stocks\": [\n";
        
        for (size_t i = 0; i < symbols.size(); i++) {
            StockData data = generateStockData(symbols[i]);
            
            json << "    {\n";
            json << "      \"symbol\": \"" << data.symbol << "\",\n";
            json << "      \"price\": " << std::fixed << std::setprecision(2) << data.price << ",\n";
            json << "      \"change\": " << data.change << ",\n";
            json << "      \"changePercent\": " << data.changePercent << ",\n";
            json << "      \"volume\": " << data.volume << ",\n";
            json << "      \"high\": " << data.high << ",\n";
            json << "      \"low\": " << data.low << ",\n";
            json << "      \"open\": " << data.open << ",\n";
            json << "      \"sma20\": " << data.sma20 << ",\n";
            json << "      \"sma50\": " << data.sma50 << ",\n";
            json << "      \"rsi\": " << data.rsi << ",\n";
            json << "      \"volatility\": " << data.volatility << ",\n";
            json << "      \"timestamp\": \"" << data.timestamp << "\"\n";
            json << "    }";
            
            if (i < symbols.size() - 1) json << ",";
            json << "\n";
        }
        
        json << "  ],\n";
        json << "  \"timestamp\": \"" << getCurrentTimestamp() << "\",\n";
        json << "  \"status\": \"success\"\n";
        json << "}";
        
        return json.str();
    }
    
    std::string getCurrentTimestamp() {
        auto now = std::time(nullptr);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

class HTTPServer {
private:
    SOCKET server_fd;
    int port;
    StockMarketProcessor processor;
    WSADATA wsaData;
    
public:
    HTTPServer(int p) : port(p), server_fd(INVALID_SOCKET) {
        // Initialize Winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return;
        }
    }
    
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "File not found: " + filename;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::string getContentType(const std::string& filename) {
        if (filename.find(".html") != std::string::npos) return "text/html";
        if (filename.find(".css") != std::string::npos) return "text/css";
        if (filename.find(".js") != std::string::npos) return "application/javascript";
        if (filename.find(".json") != std::string::npos) return "application/json";
        return "text/plain";
    }
    
    std::string handleRequest(const std::string& request) {
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;
        
        std::string response;
        std::string content;
        std::string contentType = "text/html";
        
        if (path == "/" || path == "/index.html") {
            content = readFile("index.html");
            contentType = "text/html";
        }
        else if (path == "/style.css") {
            content = readFile("style.css");
            contentType = "text/css";
        }
        else if (path == "/script.js") {
            content = readFile("script.js");
            contentType = "application/javascript";
        }
        else if (path == "/api/stocks") {
            // Get symbols from query parameter
            std::vector<std::string> symbols = {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA", "META"};
            content = processor.generateJSON(symbols);
            contentType = "application/json";
        }
        else {
            content = "<html><body><h1>404 Not Found</h1></body></html>";
            response = "HTTP/1.1 404 Not Found\r\n";
        }
        
        if (response.empty()) {
            response = "HTTP/1.1 200 OK\r\n";
        }
        
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Connection: close\r\n\r\n";
        response += content;
        
        return response;
    }
    
    void start() {
        struct sockaddr_in address;
        char opt = 1;
        int addrlen = sizeof(address);
        
        // Create socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            std::cerr << "Socket failed with error: " << WSAGetLastError() << std::endl;
            return;
        }
        
        // Set socket options
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR) {
            std::cerr << "Setsockopt failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            return;
        }
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        // Bind socket
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            return;
        }
        
        // Listen for connections
        if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_fd);
            return;
        }
        
        std::cout << "Server running on http://localhost:" << port << std::endl;
        
        while (true) {
            SOCKET new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
                std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
                continue;
            }
            
            // Read request
            char buffer[1024] = {0};
            int bytes_received = recv(new_socket, buffer, 1024, 0);
            
            if (bytes_received > 0) {
                // Handle request
                std::string response = handleRequest(std::string(buffer));
                
                // Send response
                send(new_socket, response.c_str(), response.length(), 0);
            }
            
            closesocket(new_socket);
        }
    }
    
    ~HTTPServer() {
        if (server_fd != INVALID_SOCKET) {
            closesocket(server_fd);
        }
        WSACleanup();
    }
};

int main() {
    std::srand(std::time(nullptr));
    
    std::cout << "Starting Stock Market Data Processor..." << std::endl;
    std::cout << "C++ Backend with HTML/CSS Frontend" << std::endl;
    
    HTTPServer server(5000);
    server.start();
    
    return 0;
}