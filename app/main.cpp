#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-exceptions/control.hpp>
#include <libhal-lpc40/clock.hpp>
#include <libhal-lpc40/constants.hpp>
#include <libhal-lpc40/output_pin.hpp>
#include <libhal-util/steady_clock.hpp>

#include <array>

#include <libhal-armcortex/dwt_counter.hpp>
#include <libhal-lpc40/clock.hpp>
#include <libhal-lpc40/constants.hpp>
#include <libhal-lpc40/i2c.hpp>
#include <libhal-lpc40/uart.hpp>
#include <libhal-util/i2c.hpp>
#include <libhal-util/serial.hpp>
#include <libhal-util/steady_clock.hpp>

#include <libhal-lpc40/spi.hpp>
#include <libhal-util/spi.hpp>

// Application function must be implemented by one of the compilation units
// (.cpp) files.
// void application()
// {
//   hal::cortex_m::dwt_counter steady_clock(
//     hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

//   std::array<hal::byte, 1> uart_buffer{};
//   hal::lpc40::uart uart0(0, uart_buffer);
//   hal::print(uart0, "Application starting!\n");
//   hal::lpc40::i2c i2c2(2);

//   while (true) {
//     using namespace std::literals;

//     constexpr hal::byte first_i2c_address = 0x08;
//     constexpr hal::byte last_i2c_address = 0x78;

//     hal::print(uart0, "Devices Found: ");
//     hal::print(uart0, "\nSending data from uart0!");

//     for (hal::byte address = first_i2c_address; address < last_i2c_address;
//          address++) {
//       // This can only fail if the device is not present
//       if (hal::probe(i2c2, address)) {
//         hal::print<12>(uart0, "0x%02X ", address);
//       }
//     }

//     print(uart0, "\n");
//     hal::delay(steady_clock, 1s);
//   }
// }

[[noreturn]] void terminate_handler() noexcept
{
  hal::cortex_m::dwt_counter steady_clock(
    hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

  hal::lpc40::output_pin led(1, 10);

  while (true) {
    using namespace std::chrono_literals;
    led.level(false);
    hal::delay(steady_clock, 100ms);
    led.level(true);
    hal::delay(steady_clock, 100ms);
    led.level(false);
    hal::delay(steady_clock, 100ms);
    led.level(true);
    hal::delay(steady_clock, 1000ms);
  }
}
void application()
{
  using namespace hal::literals;

  hal::cortex_m::dwt_counter steady_clock(
    hal::lpc40::get_frequency(hal::lpc40::peripheral::cpu));

  std::array<hal::byte, 32> uart_buffer{};
  hal::lpc40::uart uart0(0, uart_buffer);

  hal::lpc40::spi spi2(2);
  hal::lpc40::output_pin chip_select(1, 10);
  hal::lpc40::output_pin chip_select_mirror(1, 14);
  chip_select.level(true);

  hal::print(uart0, "Starting SPI Application...\n");

  while (true) {
    using namespace std::literals;
    std::array<hal::byte, 4> payload{ 0xDE, 0xAD, 0xBE, 0xEF };
    std::array<hal::byte, 8> buffer{};

    hal::print(uart0, "Write operation\n");
    hal::write(spi2, payload);
    hal::delay(steady_clock, 1s);

    hal::print(uart0, "Read operation: [ ");
    hal::read(spi2, buffer);

    for (const auto& byte : buffer) {
      hal::print<32>(uart0, "0x%02X ", byte);
    }

    hal::print(uart0, "]\n");
    hal::delay(steady_clock, 1s);

    hal::print(uart0, "Full-duplex transfer\n");
    spi2.transfer(payload, buffer);
    hal::delay(steady_clock, 1s);

    hal::print(uart0, "Half-duplex transfer\n");
    hal::write_then_read(spi2, payload, buffer);
    hal::delay(steady_clock, 1s);

    {
      std::array read_manufacturer_id{ hal::byte{ 0x9F } };
      std::array<hal::byte, 4> id_data{};

      chip_select.level(false);
      hal::delay(steady_clock, 250ns);
      hal::write_then_read(spi2, read_manufacturer_id, id_data, 0xA5);
      chip_select.level(true);

      hal::print(uart0, "SPI Flash Memory ID info: ");
      hal::print(uart0, "[ ");
      for (const auto& byte : id_data) {
        hal::print<32>(uart0, "0x%02X ", byte);
      }
      hal::print(uart0, "]\n");
    }

    hal::delay(steady_clock, 1s);
  }
}

int main()
{
  using namespace hal::literals;
  // Change the input frequency to match the frequency of the crystal attached
  // to the external OSC pins.
  hal::lpc40::maximum(12.0_MHz);

  hal::set_terminate(terminate_handler);

  // Run the application
  application();

  return 0;
}