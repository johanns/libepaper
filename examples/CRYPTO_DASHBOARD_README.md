# Crypto Dashboard Demo

A modern cryptocurrency price and wallet balance dashboard for e-Paper displays.

## Features

- **Real-time Price Display**: Shows current Bitcoin and Ethereum prices from CoinGecko API
- **24-Hour Change Indicators**: Visual arrows and percentage changes
- **Price History Charts**:
  - Line chart for Bitcoin showing 24-hour price history
  - Sparkline for Ethereum showing 24-hour price trend
- **Wallet Balance Tracking**: Displays balances for configured wallet addresses
- **Portfolio Value**: Shows total value in USD
- **Landscape Layout**: Optimized for 2.7" e-Paper display in landscape mode (264x176)

## Requirements

- libcurl (for HTTP requests)
- nlohmann-json (for JSON parsing)
- Internet connection
- Raspberry Pi with e-Paper display

## Installation

### Install Dependencies

```bash
# Install required libraries
sudo apt-get update
sudo apt-get install -y libcurl4-openssl-dev nlohmann-json3-dev

# Build the project
cd /home/jg/code/e-Paper
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-14
cmake --build . -j$(nproc)
```

## Configuration

### Wallet Addresses

1. Copy the example configuration file:
```bash
cd /home/jg/code/e-Paper/examples
cp wallets.json.example wallets.json
```

2. Edit `wallets.json` with your wallet addresses:
```json
{
  "bitcoin": [
    "your_bitcoin_address_1",
    "your_bitcoin_address_2"
  ],
  "ethereum": [
    "your_ethereum_address_1",
    "your_ethereum_address_2"
  ]
}
```

**Note**: For Ethereum, you may need to add an Etherscan API key. Get a free API key at https://etherscan.io/apis

To use the API key, modify the Ethereum balance fetch URL in `crypto_dashboard.cpp` to include `&apikey=YOUR_API_KEY`.

## Usage

### Run the Dashboard

```bash
# From the build directory
cd /home/jg/code/e-Paper/build/examples
sudo ./crypto_dashboard
```

The dashboard will:
1. Load wallet addresses from `wallets.json` (or use example addresses if not found)
2. Fetch current cryptocurrency prices from CoinGecko
3. Fetch price history for charts
4. Fetch wallet balances from blockchain APIs
5. Render the dashboard on the e-Paper display
6. Update every 5 minutes (performs 3 updates then exits)

### Continuous Updates

To run the dashboard continuously, you can use a loop:

```bash
while true; do
  sudo ./crypto_dashboard
  sleep 300  # Wait 5 minutes between updates
done
```

Or create a systemd service for automatic startup.

## API Details

### CoinGecko API
- **Endpoint**: `https://api.coingecko.com/api/v3/simple/price`
- **Rate Limit**: 10-50 calls/minute (free tier)
- **No API Key Required**

### Bitcoin Balance API
- **Endpoint**: `https://blockchain.info/q/addressbalance/{address}`
- **Rate Limit**: ~1 request/10 seconds per address
- **No API Key Required**

### Ethereum Balance API
- **Endpoint**: `https://api.etherscan.io/api`
- **Rate Limit**: 5 calls/second (with free API key)
- **API Key**: Recommended (free registration)

## Layout

```
┌─────────────────────────────────────────┐
│  CRYPTO DASHBOARD                       │
├─────────────────────────────────────────┤
│  BTC: $XX,XXX  [↑] +X.XX%               │
│  ┌──────────────────────┐               │
│  │  [Line Chart]        │               │
│  └──────────────────────┘               │
├─────────────────────────────────────────┤
│  ETH: $X,XXX   [↓] -X.XX%               │
│  ───────────────── [Sparkline]          │
├─────────────────────────────────────────┤
│  WALLETS                                │
│  BTC: X.XXXX BTC          $XX,XXX       │
│  ETH: X.XXXX ETH          $X,XXX        │
│  Total:                   $XX,XXX       │
└─────────────────────────────────────────┘
```

## Troubleshooting

### No Internet Connection
- Check your Raspberry Pi's network connection
- Verify you can reach external APIs: `curl https://api.coingecko.com/api/v3/ping`

### API Rate Limiting
- The demo implements a 5-minute delay between updates
- Reduce update frequency if you encounter rate limiting

### Wallet Balance Shows Zero
- Verify wallet addresses are correct
- Check if addresses have any balance using a blockchain explorer
- For Ethereum, consider adding an Etherscan API key

### Display Not Updating
- Ensure you're running with sudo (required for GPIO access)
- Check display connections
- Verify the display is properly initialized

## Customization

### Update Frequency
Change the update loop in `main()`:
```cpp
std::this_thread::sleep_for(std::chrono::minutes(5));  // Change to desired interval
```

### Add More Cryptocurrencies
Modify the CoinGecko API URL to include additional coins:
```cpp
std::string url =
    "https://api.coingecko.com/api/v3/simple/price?"
    "ids=bitcoin,ethereum,cardano,solana&vs_currency=usd&include_24hr_change=true";
```

### Modify Layout
Edit the `render_dashboard()` function to adjust positioning and styling.

## Example Output

```
Crypto Dashboard Demo
====================

Initializing device...
Display size (landscape): 264x176 pixels

Loading wallet configuration...
Loaded 1 BTC addresses and 1 ETH addresses

Fetching data (update 1/3)...
  Fetching crypto prices...
  BTC: $43521.50 (2.35%)
  ETH: $2287.32 (-1.12%)
  Fetching price history...
  BTC history: 288 points
  ETH history: 288 points
  Fetching wallet balances...
  BTC balance: 0.0015 BTC
  ETH balance: 0.5432 ETH
  Rendering dashboard...
Dashboard updated successfully!

Waiting 5 minutes before next update...
```

## License

This demo is part of the e-Paper library project. See the main LICENSE file for details.

## Credits

- CoinGecko API for cryptocurrency price data
- Blockchain.info for Bitcoin balance queries
- Etherscan for Ethereum balance queries

