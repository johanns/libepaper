# Crypto Dashboard - Modular Design

A modern, modular cryptocurrency dashboard application for e-Paper displays, built with C++23 and following best practices.

## 🏗️ Architecture

This is a **refactored version** of the original `crypto_dashboard.cpp` example, designed with:

- ✅ **Modern C++23** patterns and features
- ✅ **Composition over inheritance** design
- ✅ **Separation of concerns** with clear module boundaries
- ✅ **RAII** resource management
- ✅ **`std::expected`** for error handling
- ✅ **Const correctness** throughout
- ✅ **Value semantics** where appropriate

### Directory Structure

```
crypto_dashboard/
├── include/              # Public headers
│   ├── types.hpp        # Data structures (CryptoPrice, WalletBalance, etc.)
│   ├── http_client.hpp  # HTTP client (cURL wrapper)
│   ├── crypto_api.hpp   # API clients (CoinGecko, Etherscan, etc.)
│   ├── wallet_config.hpp # Configuration loader
│   └── dashboard_renderer.hpp  # Display rendering
├── src/                 # Implementation files
│   ├── main.cpp         # Application entry point
│   ├── types.cpp
│   ├── http_client.cpp
│   ├── crypto_api.cpp
│   ├── wallet_config.cpp
│   └── dashboard_renderer.cpp
├── CMakeLists.txt       # Build configuration
├── wallets.json.example # Example configuration
└── README.md            # This file
```

## 🎯 Design Principles

### 1. Composition Over Inheritance

Instead of deep inheritance hierarchies, we use composition:

```cpp
// CryptoDataFetcher composes multiple API clients
class CryptoDataFetcher {
private:
  const HTTPClient& client_;          // Composed HTTP client
  CoinGeckoAPI coingecko_api_;        // Composed API client
  BitcoinBlockchainAPI bitcoin_api_;  // Composed API client
  EtherscanAPI etherscan_api_;        // Composed API client
};
```

### 2. Modern Error Handling

Using `std::expected` for type-safe error propagation:

```cpp
[[nodiscard]] auto fetch_price(const std::string& symbol) const
    -> std::expected<CryptoPrice, std::string>;
```

### 3. RAII and Resource Management

```cpp
class HTTPClient {
  // Pimpl idiom hides cURL implementation details
  struct Impl;
  std::unique_ptr<Impl> impl_;

  // Movable but not copyable
  HTTPClient(HTTPClient&&) noexcept;
  HTTPClient& operator=(HTTPClient&&) noexcept;
};
```

### 4. Value Semantics

Data structures are simple value types:

```cpp
struct CryptoPrice {
  std::string symbol;
  double price = 0.0;
  bool valid = false;

  [[nodiscard]] constexpr auto is_valid() const noexcept -> bool;
};
```

### 5. Clear Separation of Concerns

Each module has a single, well-defined responsibility:

| Module | Responsibility |
|--------|---------------|
| `types.hpp` | Data structures and domain types |
| `http_client.hpp` | HTTP communication (cURL wrapper) |
| `crypto_api.hpp` | External API interactions |
| `wallet_config.hpp` | Configuration file loading |
| `dashboard_renderer.hpp` | Display rendering logic |
| `main.cpp` | Application lifecycle and orchestration |

## 🚀 Building

### Prerequisites

```bash
sudo apt-get install \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libbcm2835-dev
```

### Build Commands

```bash
# From the e-Paper root directory
cd /home/jg/code/e-Paper

# Configure
cmake -B build -S .

# Build just the crypto dashboard
cmake --build build --target crypto_dashboard -j$(nproc)

# Install
sudo cmake --install build
```

The executable will be at: `build/examples/crypto_dashboard/crypto_dashboard`

## 📖 Usage

### Basic Usage

```bash
# Run without Etherscan API key (BTC only)
sudo ./crypto_dashboard

# Run with Etherscan API key (BTC + ETH)
sudo ./crypto_dashboard --etherscan-api-key=YOUR_KEY_HERE

# Custom configuration file
sudo ./crypto_dashboard --config=my_wallets.json

# Custom update interval
sudo ./crypto_dashboard --interval=60

# Show help
./crypto_dashboard --help
```

### Configuration File

Create `wallets.json` in the same directory:

```json
{
  "bitcoin_addresses": [
    "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
    "3J98t1WpEZ73CNmYviecrnyiWrnqRhWNLy"
  ],
  "ethereum_addresses": [
    "0xde0B295669a9FD93d5F28D9Ec85E40f4cb697BAe",
    "0x00000000219ab540356cBB839Cbe05303d7705Fa"
  ]
}
```

See `wallets.json.example` for a template.

## 🔑 API Keys

### Etherscan API Key

Required for Ethereum balance fetching:

1. Visit: https://etherscan.io/apis
2. Create a free account
3. Generate an API key
4. Pass it via `--etherscan-api-key=YOUR_KEY`

**Free tier:** 100,000 requests/day, 5 requests/second

## 🧪 Testing

```bash
# Test build
cmake --build build --target crypto_dashboard

# Test with dry run (checks config without display)
./build/examples/crypto_dashboard/crypto_dashboard --help

# Full test
sudo ./build/examples/crypto_dashboard/crypto_dashboard
```

## 📚 Code Examples

### Adding a New API Client

```cpp
// In crypto_api.hpp
class MyCustomAPI {
public:
  explicit MyCustomAPI(const HTTPClient& client) : client_(client) {}

  [[nodiscard]] auto fetch_data() const
      -> std::expected<MyData, std::string>;

private:
  const HTTPClient& client_;
};

// In CryptoDataFetcher, compose it
class CryptoDataFetcher {
private:
  MyCustomAPI my_custom_api_;  // Add as member
};
```

### Custom Rendering

```cpp
// Extend DashboardRenderer
class MyCustomRenderer : public DashboardRenderer {
public:
  void render_custom_section() {
    draw_.draw_string(10, 10, "Custom!", Font::font16());
  }
};
```

## 🔧 Module Details

### HTTPClient

- Wraps libcurl with RAII
- Movable but not copyable
- Uses Pimpl idiom to hide implementation
- Returns `std::expected` for error handling

### CryptoAPI

- **CoinGeckoAPI**: Fetches prices and history
- **BitcoinBlockchainAPI**: Fetches BTC balances
- **EtherscanAPI**: Fetches ETH balances
- **CryptoDataFetcher**: High-level aggregator

All use composition instead of inheritance.

### WalletConfigLoader

- Static methods for loading JSON config
- Uses nlohmann/json library
- Returns `std::expected` for error handling
- Can create example configuration files

### DashboardRenderer

- Handles all e-Paper display rendering
- Modular drawing methods
- Clean separation from data fetching

## 🎨 Benefits of This Design

### Original Monolithic Design

```cpp
// 722 lines in one file
// - HTTP code mixed with display code
// - Hard to test individual components
// - Difficult to understand flow
// - No clear module boundaries
```

### New Modular Design

```cpp
// Clear separation:
// - 6 focused header files (~50 lines each)
// - 6 implementation files (~100-200 lines each)
// - Easy to test, understand, and extend
// - Clear dependencies and ownership
```

### Advantages

1. **Testability**: Each module can be tested in isolation
2. **Reusability**: HTTP client and API clients can be reused
3. **Maintainability**: Changes are localized to specific modules
4. **Readability**: Each file has a clear, focused purpose
5. **Extensibility**: Easy to add new features or APIs
6. **Type Safety**: `std::expected` catches errors at compile time

## 🔄 Migration from Old Version

If you're using the old `crypto_dashboard.cpp`:

```bash
# Old way
cd examples
./run_crypto_dashboard.sh --etherscan-api-key=KEY

# New way (after building)
cd build/examples/crypto_dashboard
sudo ./crypto_dashboard --etherscan-api-key=KEY
```

The functionality is identical, but the code is much cleaner!

## 📝 TODO / Future Improvements

- [ ] Add unit tests for each module
- [ ] Support for more cryptocurrencies
- [ ] Configurable display layouts
- [ ] Historical data persistence
- [ ] Web interface for configuration
- [ ] Docker support
- [ ] Continuous Integration

## 🤝 Contributing

This is an example project demonstrating modern C++ design. Feel free to:

- Use it as a template for your own projects
- Extend it with new features
- Improve the design further
- Share your improvements!

## 📄 License

Same as the parent e-Paper project.

## 🙏 Acknowledgments

- Original monolithic implementation
- CoinGecko API for crypto data
- Etherscan API for Ethereum data
- Blockchain.info for Bitcoin data
- nlohmann/json library
- libcurl library

