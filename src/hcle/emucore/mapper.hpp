#ifndef __CYNES_MAPPER__
#define __CYNES_MAPPER__

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>

#include "utils.hpp"

namespace cynes
{
    // Forward declaration.
    class NES;

    enum class MirroringMode : uint8_t
    {
        NONE,
        ONE_SCREEN_LOW,
        ONE_SCREEN_HIGH,
        HORIZONTAL,
        VERTICAL
    };

    /// Simple wrapper storing memory parsed from a ROM file.
    struct ParsedMemory
    {
    public:
        bool read_only_chr = true;
        uint16_t size_prg = 0x00;
        uint16_t size_chr = 0x00;

        std::unique_ptr<uint8_t[]> trainer;
        std::unique_ptr<uint8_t[]> memory_prg;
        std::unique_ptr<uint8_t[]> memory_chr;
    };

    /// Generic NES Mapper (see https://www.nesdev.org/wiki/Mapper).
    class Mapper
    {
    public:
        /// Initialize the mapper.
        /// @param nes Emulator.
        /// @param metadata ROM metadata.
        /// @param mode Mapper mirroring mode.
        /// @param size_cpu_ram Size of the CPU RAM.
        /// @param size_ppu_ram Size of the PPU RAM.
        Mapper(
            NES &nes,
            const ParsedMemory &metadata,
            MirroringMode mode,
            uint8_t size_cpu_ram = 0x8,
            uint8_t size_ppu_ram = 0x2);

        /// Default destructor.
        virtual ~Mapper() = default;

        /// Load and deserialize a ROM into a mapper.
        /// @param nes Emulator.
        /// @param path_rom Path to the NES ROM file.
        /// @return A pointer to the instantiated mapper.
        static std::unique_ptr<Mapper> load_mapper(
            NES &nes,
            const std::filesystem::path &path_rom);

        inline bool chr_is_read_only() const { return _read_only_chr; }

    public:
        /// Tick the mapper.
        virtual void tick();

        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);

        /// Write to a PPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_ppu(uint16_t address, uint8_t value);

        /// Read from the CPU memory mapped banks.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the console memory address space.
        /// @return The value stored at the given address.
        virtual uint8_t read_cpu(uint16_t address);

        /// Read from the PPU memory mapped banks.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the console memory address space.
        /// @return The value stored at the given address.
        virtual uint8_t read_ppu(uint16_t address);

    protected:
        NES &_nes;

    protected:
        const uint16_t _banks_prg;
        const uint16_t _banks_chr;
        const uint8_t _banks_cpu_ram;
        const uint8_t _banks_ppu_ram;

    private:
        const size_t _size_prg;
        const size_t _size_chr;
        const size_t _size_cpu_ram;
        const size_t _size_ppu_ram;
        const bool _read_only_chr;

        uint8_t *start_addr;
        size_t data_size;

    protected:
        void map_bank_prg(uint8_t page, uint16_t address);
        void map_bank_prg(uint8_t page, uint8_t size, uint16_t address);

        void map_bank_cpu_ram(uint8_t page, uint16_t address, bool read_only);
        void map_bank_cpu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only);

        void map_bank_chr(uint8_t page, uint16_t address);
        void map_bank_chr(uint8_t page, uint8_t size, uint16_t address);

        void map_bank_ppu_ram(uint8_t page, uint16_t address, bool read_only);
        void map_bank_ppu_ram(uint8_t page, uint8_t size, uint16_t address, bool read_only);

        void unmap_bank_cpu(uint8_t page);
        void unmap_bank_cpu(uint8_t page, uint8_t size);

        void set_mirroring_mode(MirroringMode mode);

        void mirror_cpu_banks(uint8_t page, uint8_t size, uint8_t mirror);
        void mirror_ppu_banks(uint8_t page, uint8_t size, uint8_t mirror);

    public:
        template <DumpOperation operation, typename T>
        constexpr void dump(T &buffer)
        {

            cynes::dump<operation>(buffer, start_addr, data_size);

            // Note to self: I'm commenting this out on the assumption that if !_size_ppu_ram then
            // _size_ppu_ram == 0 and so adding it to the start address does not increase the size
            // of the dump and therefore naturally prevents that data from being copied.

            // if (_size_cpu_ram && _size_ppu_ram)
            // {
            //     cynes::dump<operation>(buffer, start_addr, _size_cpu_ram + _size_ppu_ram);
            // }
            // else if (!_size_ppu_ram)
            // {
            //     cynes::dump<operation>(buffer, start_addr, _size_cpu_ram);
            // }
            // else
            // {
            //     cynes::dump<operation>(buffer, start_addr + _size_cpu_ram, _size_ppu_ram);
            // }
        }
    };

    /// NROM mapper (see https://www.nesdev.org/wiki/NROM).
    class NROM : public Mapper
    {
    public:
        NROM(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~NROM() = default;
    };

    /// MMC1 mapper (see https://www.nesdev.org/wiki/MMC1).
    class MMC1 : public Mapper
    {
    public:
        MMC1(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~MMC1() = default;

    public:
        /// Tick the mapper.
        virtual void tick();

        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);

    private:
        void write_registers(uint8_t register_target, uint8_t value);
        void update_banks();
    };

    /// UxROM mapper (see https://www.nesdev.org/wiki/UxROM).
    class UxROM : public Mapper
    {
    public:
        UxROM(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~UxROM() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);
    };

    /// CNROM mapper (see https://www.nesdev.org/wiki/CNROM).
    class CNROM : public Mapper
    {
    public:
        CNROM(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~CNROM() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);
    };

    /// UNROM 512 mapper (see https://www.nesdev.org/wiki/UNROM_512).
    class UNROM512 : public Mapper
    {
    public:
        UNROM512(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~UNROM512() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value) override;
    };

    /// MMC3 mapper (see https://www.nesdev.org/wiki/MMC3).
    class MMC3 : public Mapper
    {
    public:
        MMC3(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~MMC3() = default;

    public:
        /// Tick the mapper.
        virtual void tick();

        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);

        /// Write to a PPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_ppu(uint16_t address, uint8_t value);

        /// Read from the PPU memory mapped banks.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the console memory address space.
        /// @return The value stored at the given address.
        virtual uint8_t read_ppu(uint16_t address);

    private:
        void update_state(bool state);
    };

    /// AxROM mapper (see https://www.nesdev.org/wiki/AxROM).
    class AxROM : public Mapper
    {
    public:
        AxROM(NES &nes, const ParsedMemory &metadata);
        ~AxROM() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);
    };

    /// Generic MMC mapper (see https://www.nesdev.org/wiki/MMC2).
    template <uint8_t BANK_SIZE>
    class MMC : public Mapper
    {
    public:
        MMC(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~MMC() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        void write_cpu(uint16_t address, uint8_t value) override;

        /// Read from the PPU memory mapped banks.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the console memory address space.
        /// @return The value stored at the given address.
        virtual uint8_t read_ppu(uint16_t address);

    private:
        void update_banks();
    };

    using MMC2 = MMC<0x08>;
    using MMC4 = MMC<0x10>;

    /// GxROM mapper (see https://www.nesdev.org/wiki/GxROM).
    class GxROM : public Mapper
    {
    public:
        GxROM(NES &nes, const ParsedMemory &metadata, MirroringMode mode);
        ~GxROM() = default;

    public:
        /// Write to a CPU mapped memory bank.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the console memory address space.
        /// @param value Value to write.
        virtual void write_cpu(uint16_t address, uint8_t value);
    };
}

#endif
