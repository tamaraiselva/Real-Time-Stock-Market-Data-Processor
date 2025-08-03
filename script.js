// Stock Market Dashboard JavaScript
class StockDashboard {
    constructor() {
        this.isAutoRefresh = true;
        this.refreshInterval = 10000; // 10 seconds
        this.intervalId = null;
        this.stocks = [];
        
        this.init();
    }
    
    init() {
        this.setupEventListeners();
        this.startAutoRefresh();
        this.loadStockData();
    }
    
    setupEventListeners() {
        // Auto refresh toggle
        const autoRefreshToggle = document.getElementById('auto-refresh');
        autoRefreshToggle.addEventListener('change', (e) => {
            this.isAutoRefresh = e.target.checked;
            if (this.isAutoRefresh) {
                this.startAutoRefresh();
            } else {
                this.stopAutoRefresh();
            }
        });
        
        // Refresh interval selector
        const intervalSelect = document.getElementById('refresh-interval');
        intervalSelect.addEventListener('change', (e) => {
            this.refreshInterval = parseInt(e.target.value) * 1000;
            if (this.isAutoRefresh) {
                this.stopAutoRefresh();
                this.startAutoRefresh();
            }
        });
        
        // Manual refresh button
        const manualRefreshBtn = document.getElementById('manual-refresh');
        manualRefreshBtn.addEventListener('click', () => {
            this.loadStockData();
        });
    }
    
    startAutoRefresh() {
        this.stopAutoRefresh();
        this.intervalId = setInterval(() => {
            this.loadStockData();
        }, this.refreshInterval);
        
        this.updateStatus('Live', true);
    }
    
    stopAutoRefresh() {
        if (this.intervalId) {
            clearInterval(this.intervalId);
            this.intervalId = null;
        }
        
        this.updateStatus('Paused', false);
    }
    
    updateStatus(status, isLive) {
        const statusElement = document.getElementById('status');
        const statusDot = statusElement.querySelector('.status-dot');
        const statusText = statusElement.querySelector('span:last-child');
        
        statusText.textContent = status;
        statusElement.className = `status-indicator ${isLive ? 'live' : 'paused'}`;
        
        if (isLive) {
            statusElement.style.background = '#27ae60';
        } else {
            statusElement.style.background = '#f39c12';
        }
    }
    
    showLoading() {
        const loadingOverlay = document.getElementById('loading-overlay');
        loadingOverlay.classList.add('active');
    }
    
    hideLoading() {
        const loadingOverlay = document.getElementById('loading-overlay');
        loadingOverlay.classList.remove('active');
    }
    
    async loadStockData() {
        try {
            this.showLoading();
            
            const response = await fetch('/api/stocks');
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            
            const data = await response.json();
            
            if (data.status === 'success' && data.stocks) {
                this.stocks = data.stocks;
                this.updateDashboard();
                this.updateLastUpdateTime(data.timestamp);
            } else {
                throw new Error('Invalid response format');
            }
            
        } catch (error) {
            console.error('Error loading stock data:', error);
            this.showError('Failed to load stock data. Please try again.');
        } finally {
            this.hideLoading();
        }
    }
    
    updateDashboard() {
        this.updateStockCards();
        this.updateStockTable();
        this.updateMarketSummary();
    }
    
    updateStockCards() {
        const stocksGrid = document.getElementById('stocks-grid');
        stocksGrid.innerHTML = '';
        
        this.stocks.forEach(stock => {
            const card = this.createStockCard(stock);
            stocksGrid.appendChild(card);
        });
    }
    
    createStockCard(stock) {
        const card = document.createElement('div');
        const isPositive = stock.change >= 0;
        
        card.className = `stock-card ${isPositive ? 'positive' : 'negative'}`;
        card.innerHTML = `
            <div class="stock-header">
                <div class="stock-symbol">${stock.symbol}</div>
                <div class="stock-arrow">${isPositive ? '📈' : '📉'}</div>
            </div>
            <div class="stock-price">$${stock.price.toFixed(2)}</div>
            <div class="stock-change">
                ${stock.change >= 0 ? '+' : ''}${stock.change.toFixed(2)} 
                (${stock.changePercent >= 0 ? '+' : ''}${stock.changePercent.toFixed(2)}%)
            </div>
            <div class="stock-details">
                <div class="detail-item">
                    <span class="detail-label">Open:</span>
                    <span class="detail-value">$${stock.open.toFixed(2)}</span>
                </div>
                <div class="detail-item">
                    <span class="detail-label">High:</span>
                    <span class="detail-value">$${stock.high.toFixed(2)}</span>
                </div>
                <div class="detail-item">
                    <span class="detail-label">Low:</span>
                    <span class="detail-value">$${stock.low.toFixed(2)}</span>
                </div>
                <div class="detail-item">
                    <span class="detail-label">Volume:</span>
                    <span class="detail-value">${this.formatVolume(stock.volume)}</span>
                </div>
                <div class="detail-item">
                    <span class="detail-label">SMA 20:</span>
                    <span class="detail-value">$${stock.sma20.toFixed(2)}</span>
                </div>
                <div class="detail-item">
                    <span class="detail-label">RSI:</span>
                    <span class="detail-value">${stock.rsi.toFixed(1)}</span>
                </div>
            </div>
        `;
        
        return card;
    }
    
    updateStockTable() {
        const tableBody = document.getElementById('stocks-table-body');
        tableBody.innerHTML = '';
        
        this.stocks.forEach(stock => {
            const row = document.createElement('tr');
            const isPositive = stock.change >= 0;
            
            row.innerHTML = `
                <td><strong>${stock.symbol}</strong></td>
                <td>$${stock.price.toFixed(2)}</td>
                <td class="${isPositive ? 'positive-change' : 'negative-change'}">
                    ${stock.change >= 0 ? '+' : ''}${stock.change.toFixed(2)}
                </td>
                <td class="${isPositive ? 'positive-change' : 'negative-change'}">
                    ${stock.changePercent >= 0 ? '+' : ''}${stock.changePercent.toFixed(2)}%
                </td>
                <td>${this.formatVolume(stock.volume)}</td>
                <td>$${stock.high.toFixed(2)}</td>
                <td>$${stock.low.toFixed(2)}</td>
                <td>$${stock.sma20.toFixed(2)}</td>
                <td>${stock.rsi.toFixed(1)}</td>
            `;
            
            tableBody.appendChild(row);
        });
    }
    
    updateMarketSummary() {
        // Update active stocks count
        document.getElementById('active-stocks').textContent = this.stocks.length;
        
        // Calculate average volume
        const avgVolume = this.stocks.reduce((sum, stock) => sum + stock.volume, 0) / this.stocks.length;
        document.getElementById('avg-volume').textContent = this.formatVolume(avgVolume);
        
        // Find top performer
        const topPerformer = this.stocks.reduce((max, stock) => 
            stock.changePercent > max.changePercent ? stock : max
        );
        
        const topPerformerElement = document.getElementById('top-performer');
        topPerformerElement.innerHTML = `
            <div style="font-weight: bold; color: #2c3e50;">${topPerformer.symbol}</div>
            <div style="color: #27ae60; font-size: 0.9em;">+${topPerformer.changePercent.toFixed(2)}%</div>
        `;
    }
    
    updateLastUpdateTime(timestamp) {
        const lastUpdateElement = document.getElementById('last-update-time');
        const date = new Date(timestamp);
        lastUpdateElement.textContent = date.toLocaleTimeString();
    }
    
    formatVolume(volume) {
        if (volume >= 1e9) {
            return (volume / 1e9).toFixed(2) + 'B';
        } else if (volume >= 1e6) {
            return (volume / 1e6).toFixed(2) + 'M';
        } else if (volume >= 1e3) {
            return (volume / 1e3).toFixed(2) + 'K';
        } else {
            return volume.toLocaleString();
        }
    }
    
    showError(message) {
        // Create error notification
        const errorDiv = document.createElement('div');
        errorDiv.className = 'error-notification';
        errorDiv.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            background: #e74c3c;
            color: white;
            padding: 15px 20px;
            border-radius: 5px;
            z-index: 1001;
            max-width: 300px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        `;
        
        errorDiv.innerHTML = `
            <div style="font-weight: bold; margin-bottom: 5px;">Error</div>
            <div>${message}</div>
        `;
        
        document.body.appendChild(errorDiv);
        
        // Remove after 5 seconds
        setTimeout(() => {
            if (errorDiv.parentNode) {
                errorDiv.parentNode.removeChild(errorDiv);
            }
        }, 5000);
    }
}

// Initialize dashboard when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new StockDashboard();
});

// Add some additional interactive features
document.addEventListener('DOMContentLoaded', () => {
    // Add hover effects to stock cards
    const stocksGrid = document.getElementById('stocks-grid');
    
    stocksGrid.addEventListener('mouseenter', (e) => {
        if (e.target.classList.contains('stock-card')) {
            e.target.style.transform = 'translateY(-8px) scale(1.02)';
        }
    }, true);
    
    stocksGrid.addEventListener('mouseleave', (e) => {
        if (e.target.classList.contains('stock-card')) {
            e.target.style.transform = 'translateY(0) scale(1)';
        }
    }, true);
    
    // Add click to refresh functionality
    document.addEventListener('keydown', (e) => {
        if (e.key === 'F5' || (e.ctrlKey && e.key === 'r')) {
            e.preventDefault();
            window.location.reload();
        }
    });
    
    // Add responsive table scrolling
    const tableContainer = document.querySelector('.table-container');
    if (tableContainer) {
        let isScrolling = false;
        
        tableContainer.addEventListener('scroll', () => {
            if (!isScrolling) {
                tableContainer.style.boxShadow = '0 4px 16px rgba(31, 38, 135, 0.4)';
                isScrolling = true;
                
                setTimeout(() => {
                    tableContainer.style.boxShadow = '0 4px 16px rgba(31, 38, 135, 0.2)';
                    isScrolling = false;
                }, 1000);
            }
        });
    }
});