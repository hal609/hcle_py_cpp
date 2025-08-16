#ifndef __CYNES_PPU__
#define __CYNES_PPU__

#include <cstdint>
#include <memory>

#include "utils.hpp"

// enum struct SpriteEvaluationStep
// {
//     LOAD_SECONDARY_OAM,
//     INCREMENT_POINTER,
//     IDLE
// };

namespace cynes
{
    // Forward declaration.
    class NES;

    /// Picture Processing Unit (see https://www.nesdev.org/wiki/PPU).
    class PPU
    {
    public:
        /// Initialize the PPU.
        PPU(NES &nes);

        /// Default destructor.
        ~PPU() = default;

    public:
        /// Set the PPU in its power-up state.
        void power();

        /// Set the PPU in its reset state.
        void reset();

        /// Tick the PPU.
        void tick();

        /// Write to the PPU memory.
        /// @note This function has other side effects than simply writing to the memory, it
        /// should not be used as a memory set function.
        /// @param address Memory address within the PPU memory address space.
        /// @param value Value to write.
        void write(uint8_t address, uint8_t value);

        /// Read from the APU memory.
        /// @note This function has other side effects than simply reading from memory, it
        /// should not be used as a memory watch function.
        /// @param address Memory address within the PPU memory address space.
        /// @return The value stored at the given address.
        uint8_t read(uint8_t address);

        /// Get a pointer to the internal frame buffer.
        const uint8_t *get_frame_buffer() const;

        /// Check whether or not the frame is ready.
        /// @note Calling this function will reset the flag.
        /// @return True if the frame is ready, false otherwise.
        bool is_frame_ready();

        void setOutputModeGrayscale();

    private:
        NES &_nes;
        // PPUState _state;

    private:
        std::unique_ptr<uint8_t[]> _frame_buffer;

        // === GRAYSCALE OUTPUT MODIFICATIONS ===
        using RenderPixelFunc = void (PPU::*)(size_t, uint8_t);
        RenderPixelFunc _render_pixel = &PPU::render_pixel_rgb;

        void render_pixel_rgb(size_t pixel_offset, uint8_t color_index);
        void render_pixel_gray(size_t pixel_offset, uint8_t color_index);

        uint8_t _palette_cache[32];
        void update_palette_cache();

    private:
        const uint8_t DECAY_PERIOD = 30;

    private:
        void increment_scroll_x();
        void increment_scroll_y();

        void reset_scroll_x();
        void reset_scroll_y();

    private:
        void load_background_shifters();
        void update_background_shifters();

    private:
        void reset_foreground_data();
        void clear_foreground_data();
        void fetch_foreground_data();
        void load_foreground_shifter();
        void update_foreground_shifter();

        uint8_t blend_colors();

    private:
        enum class Register : uint8_t
        {
            PPU_CTRL = 0x00,
            PPU_MASK = 0x01,
            PPU_STATUS = 0x02,
            OAM_ADDR = 0x03,
            OAM_DATA = 0x04,
            PPU_SCROLL = 0x05,
            PPU_ADDR = 0x06,
            PPU_DATA = 0x07
        };
    };
}

#endif
